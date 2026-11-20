#!/usr/bin/env python3
# Genera los manuales PDF (EN / ES) de ResonatorIntoTexture.
# Uso: python3 build_manual.py <directorio_capturas> <directorio_salida>
import sys, os
from reportlab.lib.pagesizes import A4
from reportlab.lib import colors
from reportlab.lib.units import mm
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.enums import TA_CENTER, TA_LEFT, TA_JUSTIFY
from reportlab.platypus import (BaseDocTemplate, PageTemplate, Frame, Paragraph, Spacer, Image,
                                Table, TableStyle, PageBreak, KeepTogether, NextPageTemplate)
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus.tableofcontents import TableOfContents

FD = "/usr/share/fonts/truetype/dejavu/"
pdfmetrics.registerFont(TTFont("Sans", FD + "DejaVuSans.ttf"))
pdfmetrics.registerFont(TTFont("Sans-Bold", FD + "DejaVuSans-Bold.ttf"))
pdfmetrics.registerFont(TTFont("Sans-Oblique", FD + "DejaVuSans-Oblique.ttf"))
pdfmetrics.registerFont(TTFont("Sans-BoldOblique", FD + "DejaVuSans-BoldOblique.ttf"))
pdfmetrics.registerFont(TTFont("Serif", FD + "DejaVuSerif.ttf"))
pdfmetrics.registerFont(TTFont("Serif-Italic", FD + "DejaVuSerif-Italic.ttf"))
pdfmetrics.registerFont(TTFont("Mono", FD + "DejaVuSansMono.ttf"))
from reportlab.lib.fonts import addMapping
addMapping("Sans", 0, 0, "Sans"); addMapping("Sans", 1, 0, "Sans-Bold")
addMapping("Sans", 0, 1, "Sans-Oblique"); addMapping("Sans", 1, 1, "Sans-BoldOblique")

INK = colors.HexColor("#1c1c1e")
SOFT = colors.HexColor("#4a4a4e")
ACCENT = colors.HexColor("#d9661a")
PLATE = colors.HexColor("#2a2a2d")
ALU = colors.HexColor("#e6e7e9")

S = {
  "body": ParagraphStyle("body", fontName="Sans", fontSize=9.6, leading=14.2, textColor=INK, alignment=TA_JUSTIFY, spaceAfter=5),
  "lead": ParagraphStyle("lead", fontName="Serif-Italic", fontSize=11, leading=16.5, textColor=SOFT, alignment=TA_JUSTIFY, spaceAfter=8),
  "h1": ParagraphStyle("h1", fontName="Sans-Bold", fontSize=19, leading=24, textColor=INK, spaceBefore=4, spaceAfter=10),
  "h2": ParagraphStyle("h2", fontName="Sans-Bold", fontSize=13, leading=17, textColor=ACCENT, spaceBefore=10, spaceAfter=5),
  "h3": ParagraphStyle("h3", fontName="Sans-Bold", fontSize=10.5, leading=14, textColor=INK, spaceBefore=7, spaceAfter=3),
  "bullet": ParagraphStyle("bullet", fontName="Sans", fontSize=9.6, leading=13.6, textColor=INK, leftIndent=12, bulletIndent=2, spaceAfter=2.5),
  "cell": ParagraphStyle("cell", fontName="Sans", fontSize=8.4, leading=11, textColor=INK),
  "cellh": ParagraphStyle("cellh", fontName="Sans-Bold", fontSize=8.4, leading=11, textColor=colors.white),
  "caption": ParagraphStyle("caption", fontName="Sans-Oblique", fontSize=8.2, leading=11, textColor=SOFT, alignment=TA_CENTER, spaceAfter=8),
  "quote": ParagraphStyle("quote", fontName="Serif-Italic", fontSize=10.5, leading=15.5, textColor=INK, leftIndent=18, rightIndent=18, spaceBefore=4, spaceAfter=8),
  "small": ParagraphStyle("small", fontName="Sans", fontSize=8, leading=11, textColor=SOFT),
  "code": ParagraphStyle("code", fontName="Mono", fontSize=8.2, leading=11.5, textColor=INK, backColor=ALU, borderPadding=5, leftIndent=6, rightIndent=6, spaceBefore=3, spaceAfter=8),
  "toc1": ParagraphStyle("toc1", fontName="Sans-Bold", fontSize=10, leading=15, leftIndent=0),
  "toc2": ParagraphStyle("toc2", fontName="Sans", fontSize=9, leading=13, leftIndent=14, textColor=SOFT),
}


class Doc(BaseDocTemplate):
    def __init__(self, path, title, footer, **kw):
        super().__init__(path, pagesize=A4, leftMargin=20 * mm, rightMargin=20 * mm,
                         topMargin=20 * mm, bottomMargin=20 * mm, title=title,
                         author="Eloy Platas", subject=title, **kw)
        self.footer = footer
        fr = Frame(self.leftMargin, self.bottomMargin, self.width, self.height, id="f")
        self.addPageTemplates([PageTemplate("cover", [fr], onPage=self._cover),
                               PageTemplate("normal", [fr], onPage=self._page)])

    def _cover(self, c, d):
        c.saveState()
        c.setFillColor(ALU); c.rect(0, 0, A4[0], A4[1], fill=1, stroke=0)
        c.setFillColor(PLATE); c.rect(0, 0, A4[0], 30 * mm, fill=1, stroke=0)
        c.restoreState()

    def _page(self, c, d):
        c.saveState()
        c.setStrokeColor(colors.HexColor("#c9cacd")); c.setLineWidth(0.6)
        c.line(20 * mm, 14 * mm, A4[0] - 20 * mm, 14 * mm)
        c.setFont("Sans", 7.5); c.setFillColor(SOFT)
        c.drawString(20 * mm, 9.5 * mm, self.footer)
        c.drawRightString(A4[0] - 20 * mm, 9.5 * mm, str(d.page))
        c.restoreState()

    def afterFlowable(self, f):
        if isinstance(f, Paragraph) and f.style.name in ("h1", "h2"):
            lvl = 0 if f.style.name == "h1" else 1
            txt = f.getPlainText()
            key = "k%d" % id(f)
            self.canv.bookmarkPage(key)
            self.canv.addOutlineEntry(txt, key, level=lvl, closed=lvl > 0)
            self.notify("TOCEntry", (lvl, txt, self.page, key))


def table(rows, widths, head=True):
    data = []
    wide = len(widths) >= 8
    cs = ParagraphStyle("cs", parent=S["cell"], fontSize=6.9, leading=9) if wide else S["cell"]
    ch = ParagraphStyle("chs", parent=S["cellh"], fontSize=6.6, leading=9) if wide else S["cellh"]
    for i, r in enumerate(rows):
        st = ch if (head and i == 0) else cs
        data.append([Paragraph(str(x), st) for x in r])
    t = Table(data, colWidths=widths, repeatRows=1 if head else 0)
    style = [("VALIGN", (0, 0), (-1, -1), "TOP"),
             ("GRID", (0, 0), (-1, -1), 0.4, colors.HexColor("#c9cacd")),
             ("TOPPADDING", (0, 0), (-1, -1), 3), ("BOTTOMPADDING", (0, 0), (-1, -1), 3),
             ("LEFTPADDING", (0, 0), (-1, -1), 3 if len(widths) >= 8 else 6),
             ("RIGHTPADDING", (0, 0), (-1, -1), 2 if len(widths) >= 8 else 6),
             ("ROWBACKGROUNDS", (0, 1), (-1, -1), [colors.white, colors.HexColor("#f3f3f4")])]
    if head:
        style.append(("BACKGROUND", (0, 0), (-1, 0), PLATE))
    t.setStyle(TableStyle(style))
    return t


def build(content, shots, out):
    L = content
    doc = Doc(out, L["title"], L["footer"])
    W = doc.width
    story = []
    # ---------------------------------------------------------------- portada
    story.append(Spacer(1, 28 * mm))
    story.append(Paragraph("ResonatorIntoTexture", ParagraphStyle("t", fontName="Sans-Bold", fontSize=34, leading=40, textColor=INK, alignment=TA_CENTER)))
    story.append(Paragraph(L["cover_sub"], ParagraphStyle("st", fontName="Serif-Italic", fontSize=14, leading=20, textColor=SOFT, alignment=TA_CENTER, spaceBefore=6)))
    story.append(Spacer(1, 12 * mm))
    story.append(Image(os.path.join(shots, "ui_gen.png"), width=W, height=W * 740 / 1428))
    story.append(Spacer(1, 10 * mm))
    story.append(Paragraph(L["cover_line"], ParagraphStyle("cl", fontName="Sans", fontSize=10, leading=15, textColor=INK, alignment=TA_CENTER)))
    story.append(Spacer(1, 4 * mm))
    story.append(Paragraph(L["cover_ver"], ParagraphStyle("cv", fontName="Sans-Bold", fontSize=9, leading=13, textColor=ACCENT, alignment=TA_CENTER)))
    story.append(NextPageTemplate("normal"))
    story.append(PageBreak())
    # ------------------------------------------------------------------ índice
    story.append(Paragraph(L["toc"], S["h1"]))
    toc = TableOfContents(); toc.levelStyles = [S["toc1"], S["toc2"]]
    story.append(toc)
    story.append(PageBreak())
    # --------------------------------------------------------------- secciones
    for blk in L["blocks"]:
        kind = blk[0]
        if kind == "h1":
            story.append(Paragraph(blk[1], S["h1"]))
        elif kind == "h1p":
            story.append(PageBreak()); story.append(Paragraph(blk[1], S["h1"]))
        elif kind == "h2":
            story.append(Paragraph(blk[1], S["h2"]))
        elif kind == "h3":
            story.append(Paragraph(blk[1], S["h3"]))
        elif kind == "p":
            story.append(Paragraph(blk[1], S["body"]))
        elif kind == "lead":
            story.append(Paragraph(blk[1], S["lead"]))
        elif kind == "quote":
            story.append(Paragraph(blk[1], S["quote"]))
        elif kind == "code":
            story.append(Paragraph(blk[1].replace("\n", "<br/>"), S["code"]))
        elif kind == "ul":
            for it in blk[1]:
                story.append(Paragraph(it, S["bullet"], bulletText="•"))
            story.append(Spacer(1, 4))
        elif kind == "table":
            fr = blk[2]
            story.append(table(blk[1], [W * f for f in fr]))
            story.append(Spacer(1, 8))
        elif kind == "img":
            path = os.path.join(shots, blk[1]); w = W * blk[2]
            from PIL import Image as PI
            iw, ih = PI.open(path).size
            h = w * ih / iw
            maxh = 105 * mm
            if h > maxh:
                w, h = w * maxh / h, maxh
            story.append(KeepTogether([Image(path, width=w, height=h), Paragraph(blk[3], S["caption"])]))
        elif kind == "small":
            story.append(Paragraph(blk[1], S["small"]))
        elif kind == "space":
            story.append(Spacer(1, blk[1]))
        elif kind == "pb":
            story.append(PageBreak())
    doc.multiBuild(story)


if __name__ == "__main__":
    shots, outdir = sys.argv[1], sys.argv[2]
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from manual_en import CONTENT as EN
    from manual_es import CONTENT as ES
    build(EN, shots, os.path.join(outdir, "ResonatorIntoTexture-Manual-EN.pdf"))
    build(ES, shots, os.path.join(outdir, "ResonatorIntoTexture-Manual-ES.pdf"))
    print("ok")
