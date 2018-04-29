#include "Image.h"

#include <QtGlobal>

Image::Image() {
    this->width = 0;
    this->height = 0;
    this->edgeEffect = EdgeEffect::Repeat;
}

Image Image::operator-(const Image &right) const {
    Q_ASSERT(this->width == right.width && this->height == right.height);
    Image result(this->width, this->height, this->edgeEffect);
    for (int i = 0; i < this->width; i++) {
        for (int j = 0; j < this->height; j++) {
            result.setPixel(i, j, this->getPixel(i, j) - right.getPixel(i, j));
        }
    }
    return result;
}

Image::Image(const int width, const int height, const EdgeEffect edgeEffect) {
    this->width = width;
    this->height = height;
    this->edgeEffect = edgeEffect;
    this->pixels.resize(width * height, 0);
}

bool Image::sizeEq(const Image &img1,const Image &img2) {
    return img1.width == img2.width && img1.height == img1.height;
}

void Image::setPixel(int x, int y, double pixel) {
    pixels[x + y * width] = pixel;
}

double Image::getPixel(int x, int y) const {
    if (x < width && x > -1 && y < height && y > -1) {
        return pixels[x + y * width];
    }

    switch (edgeEffect) {
    case EdgeEffect::Black:
        return 0;
    case EdgeEffect::Repeat:
        return getPixelRepeat(x, y);
    case EdgeEffect::Mirror:
        return getPixelMirror(x, y);
    case EdgeEffect::Wrapping:
        return getPixelWrapping(x, y);
    }
    return 0;
}

double Image::getPixelRepeat(int x, int y) const {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= width) x = width - 1;
    if (y >= height)  y = height - 1;

    return pixels[x + y * width];
}

double Image::getPixelMirror(int x, int y) const {
    if (x < 0) x = -x;
    if (y < 0) y = -y;
    if (x >= width) x = 2 * width - x - 1;
    if (y >= height) y = 2 * height - y - 1;

    return pixels[x + y * width];
}

double Image::getPixelWrapping(int x, int y) const {
    if (x < 0) x = width + x;
    if (y < 0) y = height + y;
    if (x >= width) x = 1 + (x - width);
    if (y >= height) y = 1 + (y - height);

    return pixels[x + y * width];
}

Image Image::getDeNormolize() const {
    Image outImage(*this); //copy

    double min = this->getMin();
    double max = this->getMax();

    //если границы в пределах допустимого
    if (min >= 0 && max <= 1) {
        for (auto &pixel : outImage.pixels) {
            pixel *= 255;
        }
        return outImage;
    }

    double delta = max - min;
    if (delta == 0) delta = max;

    for (auto &pixel : outImage.pixels) {
        pixel -= min;
        pixel /= delta;
        pixel *= 255;
    }
    return outImage;
}

void Image::normolizePixels() {
    for (auto &pixel : this->pixels) {
        pixel /= 255;
    }
}
