#ifndef CJK_RENDERER_H
#define CJK_RENDERER_H

#include "Inkplate.h"
#include "CJKFont.h"

// Baseline offset: how many pixels above the bottom of the CJK cell
// the GFX font baseline sits. Adjust to align CJK with Latin text.
#define CJK_BASELINE_OFFSET 20

class CJKRenderer {
public:
    // Draw a mixed ASCII/CJK UTF-8 string. Returns ending x position.
    static int drawString(Inkplate &display, int x, int y,
                          const char *utf8str, uint16_t color = BLACK);

    // Measure total pixel width of a mixed ASCII/CJK UTF-8 string.
    static int measureString(Inkplate &display, const char *utf8str);

private:
    // Decode one UTF-8 character from the string. Advances *ptr.
    // Returns the Unicode codepoint, or 0 on error/end.
    static uint32_t decodeUTF8(const char **ptr);

    // Binary search for a codepoint in the sorted glyph table.
    // Returns pointer to the glyph entry, or NULL if not found.
    static const CJKGlyphEntry* findGlyph(uint16_t codepoint);
};

#endif // CJK_RENDERER_H
