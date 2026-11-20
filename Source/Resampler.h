// Remuestreador polifásico en streaming (sinc enventanado, Kaiser).
// Permite que cada módulo corra a su frecuencia de muestreo nativa
// (Rings/Plaits/Marbles a 48 kHz, Clouds a 32 kHz) sea cual sea la del host.
#pragma once

#include <vector>
#include <cmath>
#include <cstring>
#include <algorithm>

namespace ric {

// FIFO simple multicanal de tamaño fijo (sin reservas en el hilo de audio).
class Fifo {
 public:
  void init(int channels, int capacity) {
    channels_ = channels;
    capacity_ = capacity;
    data_.assign(static_cast<size_t>(channels * capacity), 0.0f);
    read_ = write_ = count_ = 0;
  }
  int size() const { return count_; }
  int space() const { return capacity_ - count_; }
  void clear() { read_ = write_ = count_ = 0; }

  void push(const float* const* in, int n) {
    n = std::min(n, space());
    for (int i = 0; i < n; ++i) {
      for (int c = 0; c < channels_; ++c)
        data_[static_cast<size_t>(c * capacity_ + write_)] = in[c][i];
      write_ = (write_ + 1) % capacity_;
    }
    count_ += n;
  }
  void pushFrame(const float* frame) {
    if (space() <= 0) return;
    for (int c = 0; c < channels_; ++c)
      data_[static_cast<size_t>(c * capacity_ + write_)] = frame[c];
    write_ = (write_ + 1) % capacity_;
    ++count_;
  }
  void pushSilence(int n) {
    std::vector<float> z(static_cast<size_t>(channels_), 0.0f);
    for (int i = 0; i < n; ++i) pushFrame(z.data());
  }
  // Extrae n muestras; si no hay suficientes rellena con ceros.
  int pop(float* const* out, int n) {
    int avail = std::min(n, count_);
    for (int i = 0; i < avail; ++i) {
      for (int c = 0; c < channels_; ++c)
        out[c][i] = data_[static_cast<size_t>(c * capacity_ + read_)];
      read_ = (read_ + 1) % capacity_;
    }
    for (int i = avail; i < n; ++i)
      for (int c = 0; c < channels_; ++c) out[c][i] = 0.0f;
    count_ -= avail;
    return avail;
  }

 private:
  int channels_ = 1, capacity_ = 1;
  std::vector<float> data_;
  int read_ = 0, write_ = 0, count_ = 0;
};

class StreamResampler {
 public:
  static constexpr int kHalf = 16;            // la mitad de los taps
  static constexpr int kTaps = kHalf * 2;
  static constexpr int kPhases = 256;
  static constexpr int kHistory = 16384;

  void init(double inRate, double outRate, int channels) {
    channels_ = channels;
    step_ = inRate / outRate;
    const double cutoff = std::min(1.0, outRate / inRate) * 0.94;
    table_.assign(static_cast<size_t>((kPhases + 1) * kTaps), 0.0f);
    const double beta = 8.0;
    auto bessel0 = [](double x) {
      double sum = 1.0, term = 1.0;
      for (int k = 1; k < 30; ++k) {
        term *= (x / (2.0 * k)) * (x / (2.0 * k));
        sum += term;
      }
      return sum;
    };
    const double i0b = bessel0(beta);
    for (int p = 0; p <= kPhases; ++p) {
      const double frac = static_cast<double>(p) / kPhases;
      double sum = 0.0;
      for (int k = 0; k < kTaps; ++k) {
        const double d = (k - (kHalf - 1)) - frac;  // distancia al punto
        const double x = d * cutoff;
        const double sinc = std::abs(x) < 1e-9 ? 1.0 : std::sin(3.14159265358979323846 * x) / (3.14159265358979323846 * x);
        const double r = d / kHalf;
        const double w = std::abs(r) >= 1.0 ? 0.0 : bessel0(beta * std::sqrt(1.0 - r * r)) / i0b;
        const double h = cutoff * sinc * w;
        table_[static_cast<size_t>(p * kTaps + k)] = static_cast<float>(h);
        sum += h;
      }
      for (int k = 0; k < kTaps; ++k)
        table_[static_cast<size_t>(p * kTaps + k)] /= static_cast<float>(sum);
    }
    hist_.assign(static_cast<size_t>(channels * kHistory), 0.0f);
    reset();
  }

  void reset() {
    std::fill(hist_.begin(), hist_.end(), 0.0f);
    len_ = kHalf;  // ceros iniciales: retardo fijo
    pos_ = static_cast<double>(kHalf - 1);
  }

  // Retardo de grupo aproximado, en muestras de entrada.
  int latencyIn() const { return kHalf; }

  // Introduce n muestras y deja las salidas disponibles en `out`.
  void process(const float* const* in, int n, Fifo& out) {
    int offset = 0;
    while (offset < n) {
      int chunk = std::min(n - offset, kHistory - len_);
      for (int c = 0; c < channels_; ++c)
        std::memcpy(&hist_[static_cast<size_t>(c * kHistory + len_)], in[c] + offset,
                    sizeof(float) * static_cast<size_t>(chunk));
      len_ += chunk;
      offset += chunk;
      render(out);
    }
  }

 private:
  void render(Fifo& out) {
    float frame[8];
    while (true) {
      const int base = static_cast<int>(pos_);
      if (base + kHalf >= len_) break;
      const double frac = pos_ - base;
      const double fp = frac * kPhases;
      const int p = static_cast<int>(fp);
      const float t = static_cast<float>(fp - p);
      const float* h0 = &table_[static_cast<size_t>(p * kTaps)];
      const float* h1 = h0 + kTaps;
      const int start = base - (kHalf - 1);
      for (int c = 0; c < channels_; ++c) {
        const float* x = &hist_[static_cast<size_t>(c * kHistory + start)];
        float a = 0.0f, b = 0.0f;
        for (int k = 0; k < kTaps; ++k) {
          a += x[k] * h0[k];
          b += x[k] * h1[k];
        }
        frame[c] = a + (b - a) * t;
      }
      out.pushFrame(frame);
      pos_ += step_;
    }
    // Descarta lo ya consumido, conservando el historial necesario.
    const int drop = static_cast<int>(pos_) - (kHalf - 1);
    if (drop > 0) {
      for (int c = 0; c < channels_; ++c) {
        float* h = &hist_[static_cast<size_t>(c * kHistory)];
        std::memmove(h, h + drop, sizeof(float) * static_cast<size_t>(len_ - drop));
      }
      len_ -= drop;
      pos_ -= drop;
    }
  }

  int channels_ = 1;
  double step_ = 1.0;
  std::vector<float> table_;
  std::vector<float> hist_;
  int len_ = 0;
  double pos_ = 0.0;
};

}  // namespace ric
