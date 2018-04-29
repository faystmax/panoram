#ifndef KERNEL_H
#define KERNEL_H

#include <vector>

using namespace std;

class Kernel {
public:
    Kernel(Kernel&&) = default;
    Kernel& operator=(const Kernel& kernel) = default;
    Kernel& operator=(Kernel&&) = default;
    Kernel(const int width, const int height,const vector<double> kernel);
    ~Kernel() = default;

    void rotate();
    int getHeight() const {return height;}
    int getWidth() const {return width;}
    double get(int x, int y) const {return kernel[x + y * width];}

private:
    int width;
    int height;
    vector<double> kernel;
};


class KernelCreator {
public:
    static Kernel getSobelX();
    static Kernel getSobelY();
    static Kernel getGauss(const double sigma);
    static Kernel getGaussDoubleDim(const double sigma);
    static double getGaussValue(const int i, const int j, const double sigma, const int radius);
    static Kernel getGauss(const double sigma, const int radius);
    static Kernel getGaussDoubleDim(const int width, const int height, const double sigma);
};

#endif // KERNEL_H
