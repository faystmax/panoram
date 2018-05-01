#ifndef INTERESTPOINTS_H
#define INTERESTPOINTS_H

#include "Image.h"
#include "Pyramid.h"

struct Point {
    int x;
    int y;
    int z;
    double s; // S(x,y) - значение оператора
    double sigmaScale;
    double sigmaEffect;

    Point(int x = 0, int y = 0, double s = 0) {
        this->x = x;
        this->y = y;
        this->s = s;
    }
    Point(int x, int y, int z, double s, double sigmaScale = 0, double sigmaEffect = 0) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->s = s;
        this->sigmaScale = sigmaScale;
        this->sigmaEffect = sigmaEffect;
    }
    Point(Point&&) = default;
    Point(const Point&) = default ;
    Point& operator=(Point&&) = default;
    Point& operator=(const Point&) = default;
};

class InterestPoints{
public:
    static vector<Point> harris(const Image &image, const double threshold, const int radius, const int pointsCount);
    static vector<Point> blob(Pyramid &pyramid, const double threshold = 0.05, const int radius = 2, const int pointsCount = 200);

    static void restorePoints(Pyramid &pyramid, vector<Point> &points);
private:
    static vector<Point> anmsFilter(vector<Point> points, const int pointsCount);
    static double lambda(const Image &image_dx, const Image &image_dy, const int x, const int y, const int radius);
    static vector <Point> thresholdFilter(const Image &image_S, const double threshold);
    static vector <Point> localMaximum(const vector <Point> points, const Image &image_S);
    static bool isExtremum(Pyramid &pyramid, const int x, const int y, const int z);
    static bool correctPosition(Point &p, Pyramid &pyramid);
};

#endif // INTERESTPOINTS_H
