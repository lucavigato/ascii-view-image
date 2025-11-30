import re
from PIL import Image, ImageDraw, ImageFont
import argparse

def ansi_to_image(input_file, output_file, font_path=None, font_size=10):
    with open(input_file, "r", encoding="utf-8", errors="ignore") as f:
        text = f.read()

    pattern = re.compile(r"\x1b\[38;2;(\d+);(\d+);(\d+)m(.)")

    lines = text.splitlines()
    rows = len(lines)
    cols = max(len(re.findall(pattern, line)) for line in lines)

    img = Image.new("RGB", (cols, rows), "black")
    draw = ImageDraw.Draw(img)

    for y, line in enumerate(lines):
        x = 0
        for match in re.finditer(pattern, line):
            r, g, b, char = match.groups()
            color = (int(r), int(g), int(b))
            draw.point((x, y), fill=color)
            x += 1

    scale = font_size
    img = img.resize((cols * scale, rows * scale), Image.NEAREST)

    img.save(output_file)
    print(f"Salvata immagine in: {output_file}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="FileConvert", description="Converts ascii art into png (with pixels)", epilog="")

    parser.add_argument("-i", "--input")
    parser.add_argument("-o", "--output")

    args = parser.parse_args()
    ansi_to_image(args.input, args.output)
