import re
from PIL import Image, ImageDraw, ImageFont
import argparse

def ansi_ascii_to_image(input_file, output_file, font_path="/usr/local/texlive/2025/texmf-dist/fonts/truetype/public/dejavu/DejaVuSansMono.ttf", font_size=12):
    ansi_pattern = re.compile(r'\x1b\[38;2;(\d+);(\d+);(\d+)m(.)')

    with open(input_file, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()

    max_cols = 0
    parsed_lines = []
    for line in lines:
        matches = ansi_pattern.findall(line)
        parsed_lines.append(matches)
        max_cols = max(max_cols, len(matches))

    rows = len(parsed_lines)

    font = ImageFont.truetype(font_path, font_size)
    left, top, right, bottom = font.getbbox("A")
    char_w = right - left
    char_h = int((bottom - top) * 1.8)

    img = Image.new("RGB", (max_cols * char_w, rows * char_h), (0, 0, 0))
    draw = ImageDraw.Draw(img)

    for y, matches in enumerate(parsed_lines):
        for x, (r, g, b, char) in enumerate(matches):
            draw.text((x * char_w, y * char_h), char, font=font, fill=(int(r), int(g), int(b)))

    img.save(output_file)
    print(f"Immagine salvata in: {output_file}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="File Converter", description="Converti ascii art in immagini di ascii art", epilog="")

    parser.add_argument("-i", "--input")
    parser.add_argument("-o", "--output")

    args = parser.parse_args()
    ansi_ascii_to_image(args.input, args.output)

