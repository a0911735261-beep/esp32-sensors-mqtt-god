from PIL import Image, ImageDraw, ImageFont
from pathlib import Path

W, H = 296, 128
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
RED = (220, 0, 0)

ROOT = Path(__file__).resolve().parent
OUT = ROOT / "sensor_preview.png"
FONT_PATH = "C:/Windows/Fonts/msjh.ttc"

im = Image.new("RGB", (W, H), WHITE)
d = ImageDraw.Draw(im)


def font(size):
    return ImageFont.truetype(FONT_PATH, size)


def centered(text, box, f, fill=BLACK):
    x0, y0, x1, y1 = box
    bb = d.textbbox((0, 0), text, font=f)
    tw, th = bb[2] - bb[0], bb[3] - bb[1]
    d.text(
        (x0 + (x1 - x0 - tw) // 2, y0 + (y1 - y0 - th) // 2 - bb[1]),
        text,
        font=f,
        fill=fill,
    )


def line(points, fill=BLACK, width=1):
    d.line(points, fill=fill, width=width, joint="curve")


def draw_thermometer(cx, cy):
    # Red outer bulb and tube, black inner scale.
    d.ellipse((cx - 10, cy + 8, cx + 10, cy + 28), fill=RED, outline=BLACK, width=2)
    d.rounded_rectangle((cx - 6, cy - 25, cx + 6, cy + 17), radius=6, fill=WHITE, outline=BLACK, width=2)
    d.rectangle((cx - 2, cy - 11, cx + 2, cy + 17), fill=RED)
    for dy in (-17, -8, 1):
        line([(cx + 8, cy + dy), (cx + 13, cy + dy)], BLACK, 1)


def draw_drop(cx, cy):
    # Simple high-contrast water droplet.
    d.polygon([(cx, cy - 27), (cx - 15, cy - 3), (cx - 14, cy + 9),
               (cx - 7, cy + 20), (cx, cy + 23), (cx + 9, cy + 18),
               (cx + 15, cy + 7), (cx + 13, cy - 4)], fill=RED, outline=BLACK)
    d.ellipse((cx - 6, cy - 2, cx + 2, cy + 10), fill=WHITE)
    line([(cx - 4, cy + 14), (cx + 5, cy + 8)], BLACK, 1)


def draw_sun(cx, cy):
    # Black center with red rays, readable at native 296x128 resolution.
    d.ellipse((cx - 12, cy - 12, cx + 12, cy + 12), fill=RED, outline=BLACK, width=2)
    d.ellipse((cx - 5, cy - 5, cx + 5, cy + 5), fill=WHITE)
    for dx, dy in ((0, -25), (0, 25), (-25, 0), (25, 0),
                   (-18, -18), (18, -18), (-18, 18), (18, 18)):
        line([(cx + dx * 0.62, cy + dy * 0.62),
              (cx + dx, cy + dy)], RED, 2)


# Header keeps the title compact and leaves most of the display for readings.
d.rectangle((0, 0, W, 22), fill=RED)
centered("環境即時監測", (4, 1, 292, 21), font(16), BLACK)

# Three equal cards, separated by clear black lines.
cards = [(5, 27, 99, 123), (101, 27, 195, 123), (197, 27, 291, 123)]
for x0, y0, x1, y1 in cards:
    d.rounded_rectangle((x0, y0, x1, y1), radius=5, outline=BLACK, width=2)

# Card labels.
centered("溫度", (8, 29, 96, 45), font(13), RED)
centered("濕度", (104, 29, 192, 45), font(13), RED)
centered("亮度", (200, 29, 288, 45), font(13), RED)

# Icons are centered in the upper half of each card.
draw_thermometer(52, 58)
draw_drop(148, 59)
draw_sun(244, 58)

# Example live values: these are placeholders for the eventual sensor readings.
centered("25.6", (8, 78, 96, 101), font(20), BLACK)
centered("62", (104, 78, 192, 101), font(20), BLACK)
centered("780", (200, 78, 288, 101), font(20), BLACK)

# Units are explicit and visually separate from the numbers.
centered("°C", (8, 98, 96, 111), font(11), BLACK)
centered("%RH", (104, 98, 192, 111), font(10), BLACK)
centered("lux", (200, 98, 288, 111), font(10), BLACK)

# Red status strip makes each reading feel like a live monitor.
d.rectangle((8, 111, 96, 119), fill=RED)
d.rectangle((104, 111, 192, 119), fill=RED)
d.rectangle((200, 111, 288, 119), fill=RED)
for x, label in ((8, "正常"), (104, "正常"), (200, "明亮")):
    centered(label, (x, 110, x + 88, 120), font(8), BLACK)

# Quantize to the exact three colors supported by the panel.
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

im.save(OUT)
print(f"wrote {OUT}")
