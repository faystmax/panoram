#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <algorithm>

using namespace std;

class Image {

public:
    enum EdgeEffect { Black, Repeat, Mirror,  Wrapping };

    Image();
    Image(Image &&) = default;
    Image(const Image &) = default ;
    Image &operator=(Image &&) = default;
    Image &operator=(const Image &) = default;
    Image operator-(const Image &right) const;
    Image(const int width, const int height, const EdgeEffect edgeEffect = Repeat);
    ~Image() = default;

    double getPixel(const int x,const int y) const;
    void setPixel(const int x, const int y, const double pixel);
    static bool sizeEq(const Image &img1, const Image &img2);

    Image getDeNormolize() const;
    void normolizePixels();

    int getHeight() const {return height;}
    int getWidth() const {return width;}

    inline double getMax() const {return *std::max_element(pixels.begin(), pixels.end());}
    inline double getMin() const {return *std::min_element(pixels.begin(), pixels.end());}

    void setEdgeEffect(const EdgeEffect edgeEffect) {this->edgeEffect = edgeEffect;}
    EdgeEffect getEdgeEffect() const {return this->edgeEffect;}

private:
    int height;
    int width;
    EdgeEffect edgeEffect;
    vector<double> pixels;

    double getPixelRepeat(int x,int y) const;
    double getPixelMirror(int x, int y) const;
    double getPixelWrapping(int x, int y) const;
};

#endif // IMAGE_H
