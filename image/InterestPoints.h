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
    vector<Point> harris(const Image &image, const double threshold, const int radius, const int pointsCount);
    vector<Point> blob(Pyramid &pyramid, const double threshold, const int radius, const int pointsCount);

    void restorePoints(Pyramid &pyramid, vector<Point> &points);
private:
    vector<Point> anmsFilter(vector<Point> points, const int pointsCount);
    double lambda(const Image &image_dx, const Image &image_dy, const int x, const int y, const int radius);
    vector <Point> thresholdFilter(const Image &image_S, const double threshold);
    vector <Point> localMaximum(const vector <Point> points, const Image &image_S);
    bool isExtremum(Pyramid &pyramid, const int x, const int y, const int z);
    bool correctPosition(Point &p, Pyramid &pyramid);
};

#endif // INTERESTPOINTS_H
