#!/usr/bin/env python3
"""
Generate CJK bitmap font header for Inkplate e-ink display.

Renders Chinese characters from a TTF font into 1-bit bitmaps packed in
MSB-first row-major format (compatible with Adafruit GFX drawBitmap).
Outputs a C header file with PROGMEM data for use on ESP32.

Usage:
    python3 generate_cjk_font.py [--font FONT_PATH] [--size PIXELS] [--preview]
"""

import argparse
import os
import struct
import sys
from pathlib import Path

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("Error: Pillow is required. Install with: pip3 install Pillow", file=sys.stderr)
    sys.exit(1)


# Default character set for the calendar project
# Organized by category for clarity
DEFAULT_CHARS = (
    # Dates: year/month/day/week days
    "年月日星期一二三四五六天"
    # Month names (七月=July, 八月=Aug, 九月=Sep, 十月=Oct, etc.)
    "七八九十"
    # Weather descriptions
    "晴阴多云雨雪雾霾风大小中暴雷转"
    # 24 solar terms (节气) - all unique chars
    "立春水惊蛰分清明谷夏至满芒种暑秋处白露寒霜降冬"
    # Solar term names need these additional chars
    "节气"
    # Holiday names
    "元宵端午中秋国庆重阳除夕腊八旦"
    # Festival
    "节"
    # Additional for display
    "温度湿速"
    # Weather page labels
    "量紫外向早晨傍晚夜北东南西周"
)


def get_unique_chars(char_string):
    """Return sorted unique characters from input string."""
    seen = set()
    result = []
    for ch in char_string:
        if ch not in seen and ord(ch) > 127:
            seen.add(ch)
            result.append(ch)
    # Sort by Unicode codepoint for binary search
    result.sort(key=lambda c: ord(c))
    return result


def find_system_font():
    """Try to find a CJK-capable font on the system."""
    candidates = [
        # macOS
        "/System/Library/Fonts/PingFang.ttc",
        "/System/Library/Fonts/STHeiti Medium.ttc",
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
        "/Library/Fonts/Arial Unicode.ttf",
        # Linux
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
        # Windows
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simsun.ttc",
    ]
    for path in candidates:
        if os.path.exists(path):
            return path
    return None


def render_glyph(font, char, size):
    """Render a single character to a 1-bit bitmap.

    Returns (bitmap_bytes, width, height) where bitmap_bytes is packed
    MSB-first row-major (same format as Adafruit GFX drawBitmap).
    """
    # Create image with some padding for measurement
    img = Image.new("L", (size * 2, size * 2), 0)
    draw = ImageDraw.Draw(img)

    # Draw character
    draw.text((0, 0), char, font=font, fill=255)

    # Find bounding box of actual content
    bbox = img.getbbox()
    if bbox is None:
        # Empty glyph - return blank bitmap of standard size
        byte_width = (size + 7) // 8
        return bytes(byte_width * size), size, size

    # Crop to the glyph cell size (fixed width/height for all CJK chars)
    # Center the glyph in a size x size cell
    glyph_img = Image.new("L", (size, size), 0)
    glyph_draw = ImageDraw.Draw(glyph_img)

    # Get the font metrics for centering
    left, top, right, bottom = font.getbbox(char)
    glyph_w = right - left
    glyph_h = bottom - top

    # Center horizontally and vertically in the cell
    offset_x = (size - glyph_w) // 2 - left
    offset_y = (size - glyph_h) // 2 - top

    glyph_draw.text((offset_x, offset_y), char, font=font, fill=255)

    # Threshold to 1-bit
    pixels = glyph_img.load()
    byte_width = (size + 7) // 8
    bitmap = bytearray(byte_width * size)

    for y in range(size):
        for x in range(size):
            if pixels[x, y] > 128:
                byte_idx = y * byte_width + x // 8
                bit_idx = 7 - (x % 8)
                bitmap[byte_idx] |= (1 << bit_idx)

    return bytes(bitmap), size, size


def generate_header(chars, font_path, size, output_path):
    """Generate the C header file with bitmap data and glyph table."""
    font = ImageFont.truetype(font_path, size)

    glyphs = []  # (codepoint, bitmap_bytes)

    for char in chars:
        codepoint = ord(char)
        bitmap_data, w, h = render_glyph(font, char, size)
        glyphs.append((codepoint, bitmap_data, char))
        print(f"  Rendered U+{codepoint:04X} '{char}'  ({len(bitmap_data)} bytes)")

    # Sort by codepoint (should already be sorted, but ensure)
    glyphs.sort(key=lambda g: g[0])

    byte_width = (size + 7) // 8
    glyph_bytes = byte_width * size

    with open(output_path, "w", encoding="utf-8") as f:
        f.write("// Auto-generated CJK bitmap font header\n")
        f.write(f"// Font: {os.path.basename(font_path)}, Size: {size}px\n")
        f.write(f"// Characters: {len(glyphs)}\n")
        f.write(f"// Glyph size: {size}x{size} pixels, {glyph_bytes} bytes each\n")
        f.write("// Format: MSB-first row-major (Adafruit GFX drawBitmap compatible)\n")
        f.write("//\n")
        f.write("// DO NOT EDIT - generated by tools/generate_cjk_font.py\n\n")
        f.write("#ifndef CJK_FONT_H\n")
        f.write("#define CJK_FONT_H\n\n")
        f.write("#include <Arduino.h>\n\n")

        f.write(f"#define CJK_GLYPH_W {size}\n")
        f.write(f"#define CJK_GLYPH_H {size}\n")
        f.write(f"#define CJK_GLYPH_BYTES {glyph_bytes}\n")
        f.write(f"#define CJK_GLYPH_COUNT {len(glyphs)}\n\n")

        # Glyph entry struct
        f.write("typedef struct {\n")
        f.write("    uint16_t codepoint;    // Unicode codepoint\n")
        f.write("    uint16_t bitmapOffset; // Byte offset into cjk_bitmap_data[]\n")
        f.write("} CJKGlyphEntry;\n\n")

        # Concatenated bitmap data
        f.write("const uint8_t cjk_bitmap_data[] PROGMEM = {\n")
        for i, (cp, bmp, char) in enumerate(glyphs):
            f.write(f"    // U+{cp:04X} '{char}'\n    ")
            hex_vals = ", ".join(f"0x{b:02x}" for b in bmp)
            f.write(hex_vals)
            if i < len(glyphs) - 1:
                f.write(",")
            f.write("\n")
        f.write("};\n\n")

        # Sorted glyph table for binary search
        f.write("const CJKGlyphEntry cjk_glyphs[] PROGMEM = {\n")
        offset = 0
        for i, (cp, bmp, char) in enumerate(glyphs):
            comma = "," if i < len(glyphs) - 1 else ""
            f.write(f"    {{0x{cp:04X}, {offset:5d}}}{comma}  // '{char}'\n")
            offset += len(bmp)
        f.write("};\n\n")

        f.write("#endif // CJK_FONT_H\n")

    total_bitmap = sum(len(g[1]) for g in glyphs)
    table_size = len(glyphs) * 4  # 2 bytes codepoint + 2 bytes offset
    print(f"\nGenerated {output_path}")
    print(f"  {len(glyphs)} glyphs, {size}x{size}px")
    print(f"  Bitmap data: {total_bitmap:,} bytes")
    print(f"  Glyph table: {table_size:,} bytes")
    print(f"  Total PROGMEM: {total_bitmap + table_size:,} bytes")


def generate_preview(chars, font_path, size, output_path):
    """Generate a PNG contact sheet for visual verification."""
    font = ImageFont.truetype(font_path, size)

    cols = 16
    rows = (len(chars) + cols - 1) // cols
    cell = size + 4
    margin = 2

    img_w = cols * cell + margin * 2
    img_h = rows * cell + margin * 2
    img = Image.new("L", (img_w, img_h), 255)
    draw = ImageDraw.Draw(img)

    for idx, char in enumerate(chars):
        col = idx % cols
        row = idx // cols
        x = margin + col * cell
        y = margin + row * cell

        # Draw cell border
        draw.rectangle([x, y, x + cell - 1, y + cell - 1], outline=200)

        # Draw character centered in cell
        left, top, right, bottom = font.getbbox(char)
        glyph_w = right - left
        glyph_h = bottom - top
        offset_x = x + (cell - glyph_w) // 2 - left
        offset_y = y + (cell - glyph_h) // 2 - top
        draw.text((offset_x, offset_y), char, font=font, fill=0)

    img.save(output_path)
    print(f"Preview saved to {output_path} ({img_w}x{img_h})")


def main():
    parser = argparse.ArgumentParser(description="Generate CJK bitmap font header for Inkplate")
    parser.add_argument("--font", type=str, default=None,
                        help="Path to TTF/TTC font file (default: auto-detect system CJK font)")
    parser.add_argument("--size", type=int, default=24,
                        help="Pixel size for glyphs (default: 24)")
    parser.add_argument("--chars", type=str, default=None,
                        help="Custom character string to render (default: built-in set)")
    parser.add_argument("--output", type=str, default=None,
                        help="Output header path (default: ../CJKFont.h)")
    parser.add_argument("--preview", action="store_true",
                        help="Generate a PNG contact sheet for visual verification")

    args = parser.parse_args()

    # Determine font
    font_path = args.font
    if font_path is None:
        font_path = find_system_font()
        if font_path is None:
            print("Error: No CJK font found. Specify with --font.", file=sys.stderr)
            sys.exit(1)
    print(f"Using font: {font_path}")

    if not os.path.exists(font_path):
        print(f"Error: Font file not found: {font_path}", file=sys.stderr)
        sys.exit(1)

    # Determine characters
    char_string = args.chars if args.chars else DEFAULT_CHARS
    chars = get_unique_chars(char_string)
    print(f"Unique CJK characters: {len(chars)}")
    print(f"  {''.join(chars)}")

    # Determine output path
    script_dir = Path(__file__).parent
    project_dir = script_dir.parent
    output_path = args.output or str(project_dir / "CJKFont.h")

    # Generate
    print(f"\nRendering at {args.size}x{args.size} pixels...")
    generate_header(chars, font_path, args.size, output_path)

    if args.preview:
        preview_path = str(Path(output_path).with_suffix(".png"))
        generate_preview(chars, font_path, args.size, preview_path)


if __name__ == "__main__":
    main()
