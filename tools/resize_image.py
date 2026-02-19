#!/usr/bin/env python3
"""
Resize an image by a given scale factor.

Used to prepare slogan.png (or other images) for the Inkplate SD card,
since the Inkplate drawImage API does not support scaling at runtime.

Usage:
    python3 resize_image.py <input_image> [--scale FACTOR] [--output OUTPUT_PATH]

Examples:
    python3 resize_image.py slogan.png --scale 2
    python3 resize_image.py slogan.png --scale 2 --output slogan_2x.png
"""

import argparse
import os
import sys

try:
    from PIL import Image
except ImportError:
    print("Error: Pillow is required. Install with: pip3 install Pillow", file=sys.stderr)
    sys.exit(1)


def main():
    parser = argparse.ArgumentParser(description="Resize an image by a scale factor")
    parser.add_argument("input", help="Path to input image")
    parser.add_argument("--scale", type=float, default=2.0,
                        help="Scale factor (default: 2.0)")
    parser.add_argument("--output", type=str, default=None,
                        help="Output path (default: overwrite input)")

    args = parser.parse_args()

    if not os.path.exists(args.input):
        print(f"Error: File not found: {args.input}", file=sys.stderr)
        sys.exit(1)

    img = Image.open(args.input)
    orig_w, orig_h = img.size
    new_w = int(orig_w * args.scale)
    new_h = int(orig_h * args.scale)

    resized = img.resize((new_w, new_h), Image.NEAREST)

    output_path = args.output or args.input
    resized.save(output_path)
    print(f"Resized {orig_w}x{orig_h} -> {new_w}x{new_h}: {output_path}")


if __name__ == "__main__":
    main()
