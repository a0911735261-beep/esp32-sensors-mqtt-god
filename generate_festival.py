from PIL import Image, ImageDraw, ImageFont
from pathlib import Path

W, H = 296, 128
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
RED = (220, 0, 0)

ROOT = Path(__file__).resolve().parent
PROJECT = ROOT / "esp32_2in9b_v4_test"
PREVIEW = ROOT / "festival_preview.png"
HEADER = PROJECT / "festival_data.h"
FONT_PATH = "C:/Windows/Fonts/msjh.ttc"

im = Image.new("RGB", (W, H), WHITE)
d = ImageDraw.Draw(im)


def font(size, index=0):
    return ImageFont.truetype(FONT_PATH, size, index=index)


def text_center(text, box, f, fill=BLACK):
    x0, y0, x1, y1 = box
    bb = d.textbbox((0, 0), text, font=f)
    tw, th = bb[2] - bb[0], bb[3] - bb[1]
    d.text((x0 + (x1 - x0 - tw) // 2, y0 + (y1 - y0 - th) // 2 - bb[1]), text, font=f, fill=fill)


def ellipse(box, fill, outline=None, width=1):
    x0, y0, x1, y1 = box
    d.ellipse((min(x0, x1), min(y0, y1), max(x0, x1), max(y0, y1)),
              fill=fill, outline=outline, width=width)


def line(points, fill=BLACK, width=1):
    d.line(points, fill=fill, width=width, joint="curve")


def poly(points, fill, outline=None, width=1):
    d.polygon(points, fill=fill)
    if outline:
        line(points + [points[0]], outline, width)


# White background with a clean black title.
text_center("中秋節快樂", (5, 0, 291, 18), font(17), RED)
line([(8, 19), (288, 19)], RED, 2)

# Large moon and clouds.
ellipse((162, 22, 271, 112), RED)
ellipse((171, 28, 262, 106), WHITE, BLACK, 1)
for b in [(188, 43, 201, 50), (231, 41, 244, 48), (207, 80, 222, 87), (245, 65, 254, 72)]:
    ellipse(b, RED)
for b in [(151, 90, 188, 106), (174, 95, 216, 111), (235, 91, 275, 107), (255, 84, 290, 102)]:
    ellipse(b, WHITE, BLACK, 1)
line([(150, 105), (289, 105)], RED, 2)

# Large Chang'e on the right, drawn with black hair and strong black
# shadow shapes so her white dress remains readable on the e-paper.
ellipse((207, 24, 230, 47), BLACK)
ellipse((211, 35, 228, 57), WHITE, BLACK, 1)
poly([(211, 39), (216, 34), (228, 38), (226, 44), (221, 41), (218, 46)], BLACK)
poly([(214, 30), (218, 23), (222, 30), (225, 24), (229, 32), (224, 35), (217, 34)], RED, BLACK)
ellipse((216, 44, 218, 46), BLACK)
ellipse((223, 44, 225, 46), BLACK)
line([(219, 50), (222, 51), (224, 49)], BLACK, 1)
line([(219, 56), (219, 62)], BLACK, 2)
poly([(213, 59), (226, 59), (237, 70), (232, 88), (221, 98), (204, 84), (205, 69)], WHITE, BLACK, 2)
poly([(214, 62), (205, 66), (195, 78), (199, 82), (215, 74)], WHITE, BLACK, 1)
poly([(226, 62), (237, 65), (249, 78), (245, 82), (229, 74)], WHITE, BLACK, 1)
poly([(215, 77), (230, 78), (246, 111), (212, 111), (203, 96)], WHITE, BLACK, 2)
poly([(211, 88), (225, 88), (217, 111), (201, 111)], WHITE, BLACK, 1)
poly([(225, 87), (236, 84), (258, 105), (245, 108)], WHITE, BLACK, 1)
# Black shadow folds and sleeve accents.
poly([(214, 61), (220, 64), (218, 78), (211, 84), (207, 75)], BLACK)
poly([(227, 64), (234, 69), (229, 77), (224, 73)], BLACK)
poly([(218, 79), (226, 80), (231, 92), (224, 104), (218, 96)], BLACK)
poly([(232, 88), (241, 99), (245, 106), (236, 104)], BLACK)
line([(205, 84), (217, 91), (229, 91)], BLACK, 2)
line([(212, 103), (222, 98), (236, 108)], BLACK, 2)
line([(207, 67), (225, 75), (239, 67)], BLACK, 1)
line([(208, 72), (224, 81), (243, 73)], WHITE, 1)
line([(207, 69), (188, 74), (174, 91), (158, 93)], RED, 3)
line([(232, 68), (252, 59), (270, 68), (257, 86), (243, 91)], RED, 2)
line([(231, 76), (249, 88), (265, 94), (278, 90)], RED, 2)

# Two large, high-contrast rabbits facing and eating a shared mooncake.
def rabbit(cx, cy, scale=1, facing=1):
    def sx(v):
        return cx + facing * v * scale
    # Solid black ears and body make the rabbits visible on the small panel.
    poly([(sx(-18), cy - 18 * scale), (sx(-25), cy - 57 * scale),
          (sx(-12), cy - 62 * scale), (sx(-3), cy - 27 * scale)], BLACK)
    poly([(sx(-4), cy - 19 * scale), (sx(0), cy - 61 * scale),
          (sx(14), cy - 60 * scale), (sx(11), cy - 15 * scale)], BLACK)
    ellipse((sx(-24), cy - 31 * scale, sx(17), cy + 7 * scale), BLACK)
    ellipse((sx(-32), cy - 5 * scale, sx(26), cy + 38 * scale), BLACK)
    # White face inset and clear facial features.
    ellipse((sx(-16), cy - 24 * scale, sx(11), cy + 2 * scale), WHITE)
    ellipse((sx(2), cy - 15 * scale, sx(8), cy - 9 * scale), BLACK)
    ellipse((sx(12), cy - 8 * scale, sx(19), cy - 2 * scale), RED)
    line([(sx(16), cy - 1 * scale), (sx(10), cy + 5 * scale)], BLACK, max(1, int(1.5 * scale)))
    # White belly and feet provide a readable rabbit silhouette.
    ellipse((sx(-18), cy + 0 * scale, sx(16), cy + 30 * scale), WHITE)
    ellipse((sx(-29), cy + 25 * scale, sx(-5), cy + 40 * scale), WHITE, BLACK, 1)
    ellipse((sx(6), cy + 25 * scale, sx(28), cy + 40 * scale), WHITE, BLACK, 1)
    # Arm reaches directly toward the mooncake.
    line([(sx(8), cy + 8 * scale), (sx(29), cy + 1 * scale)], BLACK, max(2, int(3 * scale)))


rabbit(45, 72, 0.72, 1)
rabbit(101, 84, 0.72, -1)

# Shared mooncake with a clear traditional pattern.
ellipse((60, 90, 112, 111), RED, BLACK, 2)
ellipse((64, 92, 108, 105), WHITE, BLACK, 1)
line([(69, 98), (75, 94), (81, 98), (87, 94), (93, 98), (99, 94), (105, 98)], BLACK, 2)
text_center("月", (77, 94, 96, 105), font(8), BLACK)
ellipse((110, 102, 133, 112), RED, BLACK, 1)
line([(113, 106), (118, 103), (123, 106), (128, 103)], BLACK, 1)

# Bottom greeting.
line([(5, 116), (145, 116)], BLACK, 1)
text_center("花好月圓", (151, 112, 224, 127), font(12), RED)
text_center("闔家平安", (224, 112, 294, 127), font(12), RED)

# Quantize strictly to the three e-paper colors.
pix = im.load()
for y in range(H):
    for x in range(W):
        r, g, b = pix[x, y]
        if r > 180 and g < 80 and b < 80:
            pix[x, y] = RED
        elif r < 100 and g < 100 and b < 100:
            pix[x, y] = BLACK
        else:
            pix[x, y] = WHITE

im.save(PREVIEW)

# Convert landscape panel pixels to the Waveshare V4 rotated memory layout.
black = bytearray([0xFF] * (W * H // 8))
# The V4 driver sends ~red_byte to the panel:
# 1-bit cleared in this buffer becomes red on the physical display.
red = bytearray([0xFF] * (W * H // 8))
for panel_y in range(H):
    for panel_x in range(W):
        driver_x = panel_y
        driver_y = W - 1 - panel_x
        index = driver_x // 8 + driver_y * (H // 8)
        mask = 0x80 >> (driver_x % 8)
        if pix[panel_x, panel_y] == BLACK:
            black[index] &= (~mask) & 0xFF
        elif pix[panel_x, panel_y] == RED:
            red[index] &= (~mask) & 0xFF


def bytes_as_cpp(name, data):
    lines = []
    for i in range(0, len(data), 16):
        lines.append("  " + ", ".join(f"0x{v:02X}" for v in data[i:i + 16]))
    return f"const uint8_t {name}[{len(data)}] PROGMEM = {{\n" + ",\n".join(lines) + "\n};\n"


HEADER.write_text(
    "#pragma once\n#include <Arduino.h>\n\n"
    + bytes_as_cpp("FESTIVAL_BLACK", black)
    + "\n"
    + bytes_as_cpp("FESTIVAL_RED", red),
    encoding="utf-8",
)
print(f"wrote {PREVIEW}")
print(f"wrote {HEADER}")
