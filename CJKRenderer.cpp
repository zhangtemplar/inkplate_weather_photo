#include "CJKRenderer.h"

uint32_t CJKRenderer::decodeUTF8(const char **ptr) {
    const uint8_t *p = (const uint8_t *)*ptr;
    uint32_t cp = 0;

    if (*p == 0) {
        return 0;
    }

    if (*p < 0x80) {
        // 1-byte ASCII
        cp = *p;
        *ptr += 1;
    } else if ((*p & 0xE0) == 0xC0) {
        // 2-byte sequence
        cp = (*p & 0x1F) << 6;
        if ((p[1] & 0xC0) != 0x80) { *ptr += 1; return 0xFFFD; }
        cp |= (p[1] & 0x3F);
        *ptr += 2;
    } else if ((*p & 0xF0) == 0xE0) {
        // 3-byte sequence (CJK lives here: U+4E00..U+9FFF)
        cp = (*p & 0x0F) << 12;
        if ((p[1] & 0xC0) != 0x80) { *ptr += 1; return 0xFFFD; }
        cp |= (p[1] & 0x3F) << 6;
        if ((p[2] & 0xC0) != 0x80) { *ptr += 2; return 0xFFFD; }
        cp |= (p[2] & 0x3F);
        *ptr += 3;
    } else if ((*p & 0xF8) == 0xF0) {
        // 4-byte sequence
        cp = (*p & 0x07) << 18;
        if ((p[1] & 0xC0) != 0x80) { *ptr += 1; return 0xFFFD; }
        cp |= (p[1] & 0x3F) << 12;
        if ((p[2] & 0xC0) != 0x80) { *ptr += 2; return 0xFFFD; }
        cp |= (p[2] & 0x3F) << 6;
        if ((p[3] & 0xC0) != 0x80) { *ptr += 3; return 0xFFFD; }
        cp |= (p[3] & 0x3F);
        *ptr += 4;
    } else {
        // Invalid leading byte
        *ptr += 1;
        return 0xFFFD;
    }

    return cp;
}

const CJKGlyphEntry* CJKRenderer::findGlyph(uint16_t codepoint) {
    int lo = 0;
    int hi = CJK_GLYPH_COUNT - 1;

    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        uint16_t midCp = pgm_read_word(&cjk_glyphs[mid].codepoint);

        if (midCp == codepoint) {
            return &cjk_glyphs[mid];
        } else if (midCp < codepoint) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    return NULL;
}

int CJKRenderer::drawString(Inkplate &display, int x, int y,
                             const char *utf8str, uint16_t color) {
    const char *p = utf8str;
    int curX = x;

    while (*p) {
        uint32_t cp = decodeUTF8(&p);
        if (cp == 0) break;

        if (cp < 0x80) {
            // ASCII: use the current GFX font via setCursor/write
            display.setCursor(curX, y);
            display.setTextColor(color);
            display.write((uint8_t)cp);
            curX = display.getCursorX();
        } else {
            // CJK or other multi-byte: look up in our bitmap table
            const CJKGlyphEntry *glyph = findGlyph((uint16_t)cp);
            if (glyph) {
                uint16_t offset = pgm_read_word(&glyph->bitmapOffset);
                // drawBitmap draws from top-left corner.
                // y is the GFX baseline; shift up to align CJK glyph.
                display.drawBitmap(curX, y - CJK_BASELINE_OFFSET,
                                   cjk_bitmap_data + offset,
                                   CJK_GLYPH_W, CJK_GLYPH_H, color);
                curX += CJK_GLYPH_W;
            }
            // If glyph not found, silently skip
        }
    }

    return curX;
}

int CJKRenderer::measureString(Inkplate &display, const char *utf8str) {
    const char *p = utf8str;
    int width = 0;

    while (*p) {
        uint32_t cp = decodeUTF8(&p);
        if (cp == 0) break;

        if (cp < 0x80) {
            int16_t x1, y1;
            uint16_t w, h;
            char buf[2] = {(char)cp, 0};
            display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
            width += w + 1;
        } else {
            const CJKGlyphEntry *glyph = findGlyph((uint16_t)cp);
            if (glyph) {
                width += CJK_GLYPH_W;
            }
        }
    }

    return width;
}
