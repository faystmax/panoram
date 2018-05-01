#include "InterestPoints.h"
#include "ImageConverter.h"
#include "Pyramid.h"
#include "Image.h"

vector<Point> InterestPoints::harris(const Image &image, const double threshold, const int radius, const int pointsCount) {

    Image image_dx = ImageConverter::convolution(image, KernelCreator::getSobelX());
    Image image_dy = ImageConverter::convolution(image, KernelCreator::getSobelY());

    Image image_S(image.getWidth(),image.getHeight());  // Веса
    for (int x = 0; x < image.getWidth(); x++) {
        for (int y = 0; y < image.getHeight(); y++) {
            image_S.setPixel(x, y, lambda(image_dx, image_dy, x, y, radius));
        }
    }

    vector<Point> points = thresholdFilter(image_S, threshold);
    vector<Point> localMaximumPoints = localMaximum(points, image_S);
    return anmsFilter(localMaximumPoints, pointsCount);
}

vector<Point> InterestPoints::blob(Pyramid &pyramid, const double threshold, const int radius, const int pointsCount) {
    vector<Point> points;
    Kernel kernel_x = KernelCreator::getSobelX();
    Kernel kernel_y = KernelCreator::getSobelY();
    for (int z = 1; z < pyramid.getDogsSize()-1; z++) {
        Image &imageDOG = pyramid.getDog(z).image;
        Image &imageTrue = pyramid.getDog(z).trueImage;
        Image image_dx = ImageConverter::convolution(imageTrue, kernel_x);
        Image image_dy = ImageConverter::convolution(imageTrue, kernel_y);

        for (int i = 1; i < imageDOG.getWidth() - 1; i++) {
            for (int j = 1; j < imageDOG.getHeight() - 1; j++) {

                Point point(i, j, z, 0, pyramid.getDog(z).sigmaScale, pyramid.getDog(z).sigmaEffect);
                if (isExtremum(pyramid, i, j, z) && correctPosition(point,pyramid)) {

                    // check harris
                    double val = pyramid.getDog(z).sigmaScale / pyramid.getDog(0).sigmaScale;
                    double lambdaMin = lambda(image_dx, image_dy, i, j, round(radius * val));
                    if (lambdaMin < threshold) {
                        continue;    // skip - haris to low
                    }

                    // Сохраняем
                    point.s = lambdaMin;
                    points.push_back(point);
                }
            }
        }
    }

    // Сортируем и оборезаем если нужно
    std::sort(points.begin(), points.end(), [](auto &p1, auto &p2) {
        return p1.s > p2.s;
    });
    if (points.size() > pointsCount) {
        points.resize(pointsCount);
    }

    return points;
}

void InterestPoints::restorePoints(Pyramid &pyramid, vector<Point> &points) {
    for (auto &point: points) {
        //приводим к оригинальному масштабу
        double step_W = double(pyramid.getDog(0).image.getWidth()) / pyramid.getDog(point.z).image.getWidth();
        double step_H = double(pyramid.getDog(0).image.getHeight()) / pyramid.getDog(point.z).image.getHeight();
        point.x = point.x * step_W;
        point.y = point.y * step_H;
    }
}

bool InterestPoints::isExtremum(Pyramid &pyramid, const int x, const int y, const int z) {

    if (pyramid.getDog(z-1).octave == pyramid.getDog(z+1).octave) {
        bool min = true, max = true;
        double center = pyramid.getDog(z).image.getPixel(x, y);

        // ищем в 3D
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                for (int k = -1; k <= 1; k++) {
                    if (i == 0 && j == 0 && k == 0) {
                        continue;   //skip center
                    }
                    double value = pyramid.getDog(z + k).image.getPixel(x + i, y + j);
                    if (value >= center) {
                        max = false;
                    }
                    if (value <= center) {
                        min = false;
                    }
                }
            }
        }

        return max || min;
    }
    return false;
}

bool InterestPoints::correctPosition(Point &p, Pyramid &pyramid) {

    bool flagSelect = true;                         // Флаг - подходит ли точка
    Point originalPoint = p;                        // Исходная точка

    // Производные
    double proizv_x, proizv_y, proizv_z;
    double proizv_xx, proizv_yy, proizv_zz;
    double proizv_xy;


    // Корректируем позицию
    for (int i = 0; i < 20; i++) { // максимум 10 итераций
        proizv_x = 0.5 * (pyramid.getDog(p.z).image.getPixel(p.x + 1, p.y) - pyramid.getDog(p.z).image.getPixel(p.x - 1, p.y));
        proizv_y = 0.5 * (pyramid.getDog(p.z).image.getPixel(p.x, p.y + 1) - pyramid.getDog(p.z).image.getPixel(p.x, p.y - 1));
        proizv_z = 0.5 * (pyramid.getDog(p.z + 1).image.getPixel(p.x, p.y) - pyramid.getDog(p.z - 1).image.getPixel(p.x, p.y));

        proizv_xx = pyramid.getDog(p.z).image.getPixel(p.x - 1,p.y) - 2 * pyramid.getDog(p.z).image.getPixel(p.x, p.y) + pyramid.getDog(p.z).image.getPixel(p.x + 1,p.y);
        proizv_yy = pyramid.getDog(p.z).image.getPixel(p.x,p.y - 1) - 2 * pyramid.getDog(p.z).image.getPixel(p.x,p.y) + pyramid.getDog(p.z).image.getPixel(p.x + 1,p.y+1);
        proizv_zz = pyramid.getDog(p.z-1).image.getPixel(p.x,p.y) - 2 * pyramid.getDog(p.z).image.getPixel(p.x,p.y) + pyramid.getDog(p.z + 1).image.getPixel(p.x,p.y);

        double x_shift = -proizv_x / proizv_xx;
        double y_shift = -proizv_y / proizv_yy;
        double z_shift = -proizv_z / proizv_zz;

        if (abs(x_shift) <= 0.5 && abs(y_shift) <= 0.5 && abs(z_shift)<= 0.5) {
            break;
        }

        // Меняем позицию x,y,sigma
        if (abs(x_shift)>0.5) {
            p.x += x_shift > 0 ? 1 : -1;
        }
        if (abs(y_shift)>0.5) {
            p.y += y_shift > 0 ? 1 : -1;
        }
        if (abs(z_shift)>0.5) {
            p.z += z_shift > 0 ? 1 : -1;
        }
        // Если вышли за октаву - отбрасываем
        if (pyramid.getDog(p.z).octave != pyramid.getDog(originalPoint.z).octave || p.z<1 || p.z>pyramid.getDogsSize()) {
            flagSelect = false;
            break;
        }
    }

    // Отбрасываем
    if (!flagSelect) {
        return false;
    }

    // Считаем вектор сдвига
    double x_dif = originalPoint.x - p.x;
    double y_dif = originalPoint.y - p.y;
    double z_dif = pyramid.getDog(originalPoint.z).sigmaScale - pyramid.getDog(p.z).sigmaScale;

    // Проверяем контрастность в точке
    double contrast = pyramid.getDog(p.z).image.getPixel(p.x, p.y) +
                      0.5 * proizv_x * x_dif +
                      0.5 * proizv_y * y_dif +
                      0.5 * proizv_z * z_dif;

    // Пропускаем если не проходит
    if (abs(contrast) < 0.01) { // если поставить 0.03, то очень мало точек выходит
        return false;
    }

    //считаем proizv_xy
    double dx1 = 0.5 * (pyramid.getDog(p.z).image.getPixel(p.x + 1, p.y + 1) - pyramid.getDog(p.z).image.getPixel(p.x - 1, p.y + 1));
    double dx2 = 0.5 * (pyramid.getDog(p.z).image.getPixel(p.x + 1, p.y - 1) - pyramid.getDog(p.z).image.getPixel(p.x - 1, p.y - 1));
    proizv_xy = 0.5 * (dx2 - dx1);

    // Отбрасываем краевые точки
    double trH = proizv_xx + proizv_yy;
    double detH = proizv_xx * proizv_yy - proizv_xy * proizv_xy;
    if (trH * trH / detH > 12.1) { // ((10 + 1) * (10 + 1)) / 10
        return false;
    }

    // Точка подходит
    return true;
}


// Adaptive Non-Maximum Suppression
vector<Point> InterestPoints::anmsFilter(vector<Point> points, const int pointsCount) {

    vector<bool> flagUsedPoints(points.size(), true);
    auto radius = 3;
    int usedPointsCount = points.size();
    while (usedPointsCount > pointsCount) {
        for (unsigned int i = 0; i < points.size(); i++) {

            if (!flagUsedPoints[i]) {
                continue;
            }

            auto &p1 = points[i];
            for (unsigned int j = i + 1; j < points.size(); j++) {
                if (flagUsedPoints[j]) {
                    Point &p2 = points[j];
                    if (p1.s * 0.9 > p2.s && sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)) <= radius) {
                        flagUsedPoints[j] = false;
                        usedPointsCount--;
                        if (usedPointsCount <= pointsCount) {
                            break;
                        }
                    }
                }
            }
        }
        radius++;
    }

    vector <Point> resultPoints;
    for (unsigned int i = 0; i < points.size(); i++) {
        if (flagUsedPoints[i]) {
            resultPoints.push_back(points[i]);
        }
    }
    return resultPoints;
}

double InterestPoints::lambda(const Image &image_dx, const Image &image_dy, const int x, const int y, const int radius) {
    double A = 0, B = 0, C = 0;
    for (auto i = x - radius; i < x + radius; i++) {
        for (auto j = y - radius; j < y + radius; j++) {
            auto curA = image_dx.getPixel(i, j);
            auto curB = image_dy.getPixel(i, j);
            A += curA * curA;
            B += curA * curB;
            C += curB * curB;
        }
    }
    auto descreminant = sqrt((A - C) * (A - C) + 4 * B * B);
    return min(abs((A + C - descreminant) / 2), abs((A + C + descreminant) / 2));
}

vector <Point> InterestPoints::thresholdFilter(const Image &image_S, const double threshold) {

    vector <Point> points;
    //    vector<double> truePixels = image_S.deNormolize();
    for (auto i = 0; i < image_S.getWidth(); i++) {
        for (auto j = 0; j < image_S.getHeight(); j++) {
            if (image_S.getPixel(i,j) >= threshold) {
                points.emplace_back(i, j, image_S.getPixel(i, j));
            }
        }
    }
    std::sort(points.begin(), points.end(), [](auto &p1, auto &p2) {
        return p1.s > p2.s;
    });
    return points;
}

vector<Point> InterestPoints::localMaximum(const vector<Point> points, const Image &image_S) {

    vector <Point> result;
    const int radius = 2;

    for (size_t i = 0; i < points.size(); i ++) {
        auto p1 = points[i];
        bool flagMaximum = true;

        for (auto j = -radius; j <= radius; ++j) {
            for (auto k = -radius; k <= radius; ++k) {
                if (j ==0 && k == 0) {
                    continue;
                }

                if (image_S.getPixel(p1.x + j, p1.y + k) >= p1.s) {
                    flagMaximum = false;
                    break;
                }
            }
        }

        if (flagMaximum == true) {
            result.push_back(p1);
        }
    }
    return result;
}

