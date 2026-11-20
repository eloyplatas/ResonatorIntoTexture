// arena.h - the DATA BENDER borrows the two big RAM blocks of the granular
// processor (block_mem and block_ccm, clouds.cc).
// Only one of them runs at a time: entering the bender stops the granular
// processor, leaving it re-initialises the processor (fresh buffers).
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace arena
{
// bump allocator over one block; Reset() before an engine takes it
struct Bump
{
    uint8_t* base = nullptr;
    size_t   size = 0, used = 0;
    void     Reset() { used = 0; }
    template <typename T>
    T* Take(size_t n)
    {
        used  = (used + 15u) & ~(size_t)15u;
        T* p  = (T*)(base + used);
        used += n * sizeof(T);
        if(used > size)
            __builtin_trap(); // does not fit: a bug (shows the fault LEDs)
        memset(p, 0, n * sizeof(T));
        return p;
    }
};
extern Bump M; // main RAM block
extern Bump C; // CCM block
} // namespace arena
