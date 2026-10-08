#!/usr/bin/env python3
import io
import os
import random
import sys
import urllib.request

from PIL import Image, ImageEnhance, ImageFilter, ImageOps

BASE_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "test_images")

SOURCES = {
    "butterfly": "https://raw.githubusercontent.com/opencv/opencv/master/samples/data/butterfly.jpg",
    "fruits": "https://raw.githubusercontent.com/opencv/opencv/master/samples/data/fruits.jpg",
    "home": "https://raw.githubusercontent.com/opencv/opencv/master/samples/data/home.jpg",
    "apple": "https://raw.githubusercontent.com/opencv/opencv/master/samples/data/apple.jpg",
}


def download(name, url):
    request = urllib.request.Request(url, headers={"User-Agent": "Mozilla/5.0"})
    with urllib.request.urlopen(request, timeout=60) as response:
        data = response.read()
    image = Image.open(io.BytesIO(data)).convert("RGB")
    if image.width > 700 or image.height > 700:
        image.thumbnail((700, 700))
    return image


def save(image, name):
    image.save(os.path.join(BASE_DIR, name + ".png"))
    print(name)


def gaussian_noise(image, strength):
    random.seed(7)
    noise = Image.effect_noise(image.size, strength).convert("RGB")
    return Image.blend(image, noise, 0.3)


def salt_pepper(image, amount):
    random.seed(1234)
    pixels = image.load()
    w, h = image.size
    count = int(w * h * amount)
    for _ in range(count):
        x = random.randrange(w)
        y = random.randrange(h)
        value = 0 if random.random() < 0.5 else 255
        pixels[x, y] = (value, value, value)
    return image


def main():
    if not os.path.isdir(BASE_DIR):
        os.makedirs(BASE_DIR)
    for file_name in os.listdir(BASE_DIR):
        if file_name.endswith(".png"):
            os.remove(os.path.join(BASE_DIR, file_name))

    for name, url in SOURCES.items():
        try:
            image = download(name, url)
        except Exception as error:
            print("не удалось скачать %s: %s" % (name, error), file=sys.stderr)
            continue
        save(image, name)
        save(ImageEnhance.Contrast(image).enhance(0.25), "low_contrast_" + name)
        save(gaussian_noise(image, 50), "noisy_gauss_" + name)
        save(salt_pepper(image.copy(), 0.05), "noisy_salt_" + name)
        save(image.filter(ImageFilter.GaussianBlur(3)), "blurred_" + name)
        save(ImageEnhance.Brightness(image).enhance(0.4), "dark_" + name)
        save(ImageEnhance.Brightness(image).enhance(1.6), "bright_" + name)
        save(ImageOps.grayscale(image).convert("RGB"), "gray_" + name)

    gradient = Image.new("RGB", (512, 512))
    for x in range(512):
        for y in range(512):
            gradient.putpixel((x, y), (x // 2, (x + y) // 4, y // 2))
    save(gradient, "gradient")
    save(ImageEnhance.Contrast(gradient).enhance(0.2), "low_contrast_gradient")
    print("готово: %s" % BASE_DIR)


if __name__ == "__main__":
    main()
