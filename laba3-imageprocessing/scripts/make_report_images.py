#!/usr/bin/env python3
import os
import subprocess
import sys

from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BINARY = os.path.join(ROOT, "build", "ImageProcessing")
TEST_DIR = os.path.join(ROOT, "test_images")
OUT_DIR = os.path.join(ROOT, "Conditions", "report_images")
TMP_DIR = os.path.join(ROOT, "build", "report_tmp")

EXPERIMENTS = [
    ("low_contrast_home.png", "contrast-auto", [], "Линейное контрастирование (авто)"),
    ("low_contrast_home.png", "equalize-rgb", [], "Эквализация по каналам RGB"),
    ("low_contrast_home.png", "equalize-hsv", [], "Эквализация яркости HSV"),
    ("low_contrast_home.png", "clahe", ["tiles=8", "clip=2"], "CLAHE (тайлы 8x8, лимит 2)"),
    ("low_contrast_butterfly.png", "contrast-auto", [], "Линейное контрастирование (авто)"),
    ("low_contrast_butterfly.png", "equalize-rgb", [], "Эквализация по каналам RGB"),
    ("low_contrast_butterfly.png", "equalize-hsv", [], "Эквализация яркости HSV"),
    ("low_contrast_butterfly.png", "clahe", ["tiles=8", "clip=2"], "CLAHE (тайлы 8x8, лимит 2)"),
    ("noisy_gauss_home.png", "equalize-hsv", [], "Эквализация яркости HSV на зашумленном"),
    ("noisy_gauss_home.png", "clahe", ["tiles=8", "clip=4"], "CLAHE на зашумленном (лимит 4)"),
    ("dark_fruits.png", "gamma", ["gamma=0.5"], "Гамма-коррекция 0.5"),
    ("dark_fruits.png", "contrast-auto", [], "Линейное контрастирование (авто)"),
]


def font(size):
    for path in (
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "C:/Windows/Fonts/arial.ttf",
    ):
        if os.path.exists(path):
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()


def luminance_histogram(image):
    gray = image.convert("L")
    histogram = [0] * 256
    for value in gray.getdata():
        histogram[value] += 1
    return histogram


def draw_histogram(before, after, title, path):
    width, height = 640, 360
    plot_x, plot_y, plot_w, plot_h = 50, 50, 570, 260
    canvas = Image.new("RGB", (width, height), (255, 255, 255))
    draw = ImageDraw.Draw(canvas)
    draw.text((10, 12), title, fill=(0, 0, 0), font=font(16))
    maximum = max(max(before), max(after)) or 1
    draw.line((plot_x, plot_y, plot_x, plot_y + plot_h), fill=(0, 0, 0))
    draw.line((plot_x, plot_y + plot_h, plot_x + plot_w, plot_y + plot_h), fill=(0, 0, 0))
    for index in range(256):
        x0 = plot_x + int(index * plot_w / 256)
        x1 = plot_x + int((index + 1) * plot_w / 256)
        bar_h = int(before[index] * plot_h / maximum)
        draw.rectangle((x0, plot_y + plot_h - bar_h, max(x1, x0 + 1), plot_y + plot_h),
                       fill=(170, 170, 170))
    points = []
    for index in range(256):
        x = plot_x + int((index + 0.5) * plot_w / 256)
        y = plot_y + plot_h - int(after[index] * plot_h / maximum)
        points.append((x, y))
    draw.line(points, fill=(0, 160, 0), width=2)
    draw.text((plot_x, plot_y + plot_h + 10), "0", fill=(0, 0, 0), font=font(12))
    draw.text((plot_x + plot_w - 20, plot_y + plot_h + 10), "255", fill=(0, 0, 0), font=font(12))
    draw.rectangle((plot_x + 300, 16, plot_x + 320, 28), fill=(170, 170, 170))
    draw.text((plot_x + 325, 14), "до", fill=(0, 0, 0), font=font(12))
    draw.line((plot_x + 360, 22, plot_x + 385, 22), fill=(0, 160, 0), width=2)
    draw.text((plot_x + 390, 14), "после", fill=(0, 0, 0), font=font(12))
    canvas.save(path)


def side_by_side(before_image, after_image, path, title_before, title_after):
    width = 700
    height = 380
    canvas = Image.new("RGB", (width, height), (245, 245, 245))
    draw = ImageDraw.Draw(canvas)
    draw.text((10, 8), title_before + "  ->  " + title_after, fill=(0, 0, 0), font=font(15))
    left = before_image.copy()
    left.thumbnail((340, 340))
    right = after_image.copy()
    right.thumbnail((340, 340))
    canvas.paste(left, (5, 32))
    canvas.paste(right, (355, 32))
    draw.rectangle((4, 31, 5 + left.width, 32 + left.height), outline=(0, 0, 0))
    draw.rectangle((354, 31, 355 + right.width, 32 + right.height), outline=(0, 0, 0))
    canvas.save(path)


def stats_line(image):
    gray = image.convert("L")
    data = list(gray.getdata())
    count = len(data)
    mean = sum(data) / count
    variance = sum(v * v for v in data) / count - mean * mean
    return "min=%d max=%d среднее=%.1f sigma=%.1f" % (
        min(data), max(data), mean, variance ** 0.5)


def main():
    if not os.path.exists(BINARY):
        print("сначала соберите проект: cmake --build build", file=sys.stderr)
        return 1
    if not os.path.isdir(OUT_DIR):
        os.makedirs(OUT_DIR)
    if not os.path.isdir(TMP_DIR):
        os.makedirs(TMP_DIR)

    report_lines = []
    for source_name, op, params, title in EXPERIMENTS:
        source_path = os.path.join(TEST_DIR, source_name)
        if not os.path.exists(source_path):
            print("нет файла " + source_path, file=sys.stderr)
            continue
        output_path = os.path.join(TMP_DIR, "%s_%s.png" % (source_name.replace(".png", ""), op))
        command = [BINARY, "cli", source_path, output_path, op] + params
        completed = subprocess.run(command, capture_output=True, text=True)
        if completed.returncode != 0:
            print("ошибка: " + completed.stderr, file=sys.stderr)
            continue
        before = Image.open(source_path).convert("RGB")
        after = Image.open(output_path).convert("RGB")
        slug = "%s_%s" % (source_name.replace(".png", ""), op)
        histogram_path = os.path.join(OUT_DIR, "hist_%s.png" % slug)
        draw_histogram(luminance_histogram(before), luminance_histogram(after),
                       "Гистограмма яркости: " + title, histogram_path)
        compare_path = os.path.join(OUT_DIR, "img_%s.png" % slug)
        side_by_side(before, after, compare_path, source_name, op)
        report_lines.append("| %s | %s | %s | %s | %s |" % (
            title, source_name,
            stats_line(before), stats_line(after),
            "![](report_images/hist_%s.png)" % slug))
        print(title + ": " + stats_line(after))

    table_path = os.path.join(OUT_DIR, "stats_table.md")
    with open(table_path, "w") as file:
        file.write("| Операция | Изображение | До | После | График |\n")
        file.write("|---|---|---|---|---|\n")
        for line in report_lines:
            file.write(line + "\n")
    print("таблица: " + table_path)
    return 0


if __name__ == "__main__":
    sys.exit(main())
