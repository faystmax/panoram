#define _USE_MATH_DEFINES
#include "Descriptor.h"
#include "Kernel.h"
#include "Pyramid.h"
#include "ImageConverter.h"

#include <math.h>
#include <numeric>
#include <algorithm>
#include <iostream>

Descriptor::Descriptor(const int size, Point interPoint) {
    data.resize(size, 0);
    this->interPoint = interPoint;
}

void Descriptor::normalize() {
    double length = 0;
    for (auto &data : this->data) {
        length += data * data;
    }
    length = sqrt(length);
    for (auto &data : this->data) {
        data /= length;
    }
}

void Descriptor::clampData(const double min, const double max) {
    for (auto &data : this->data) {
        data = std::clamp(data, min, max);
    }
}

void Descriptor::setPointXY(const int x, const int y) {
    this->interPoint.x = x;
    this->interPoint.y = y;
}

double DescriptorCreator::getDistance(const Descriptor &d1, const Descriptor &d2) {
    auto op = [](double a, double b) {
        return pow(a - b, 2);
    };
    auto sum = inner_product(begin(d1.data), end(d1.data), begin(d2.data), 0., plus<>(), op);
    return sqrt(sum);
}

/* Ориентация точки */
vector<double> DescriptorCreator::getPointOrientation(const Image &image_dx, const Image &image_dy, const Point &point,
        const int sigma,const int radius) {

    const int basketCount = 36;

    auto dimension = radius * 2;
    auto sector = 2 * M_PI / basketCount;
    auto halfSector = M_PI / basketCount;


    vector<double> baskets(basketCount, 0);
    for (auto i = 1; i < dimension; i++) {
        for (auto j = 1; j < dimension; j++) {
            // координаты
            auto coord_X = i - radius + point.x;
            auto coord_Y = j - radius + point.y;

            // градиент
            auto gradient_X = image_dx.getPixel(coord_X, coord_Y);
            auto gradient_Y = image_dy.getPixel(coord_X, coord_Y);

            // получаем значение(домноженное на Гаусса) и угол
            auto value = getGradientValue(gradient_X, gradient_Y)  * KernelCreator::getGaussValue(i, j, sigma*2, radius);
            auto phi = getGradientDirection(gradient_X, gradient_Y);

            // получаем индекс корзины в которую входит phi и смежную с ней
            int firstBasketIndex = floor(phi / sector);
            int secondBasketIndex = int(floor((phi - halfSector) / sector) + basketCount) % basketCount;

            // вычисляем центр
            auto mainBasketPhi = firstBasketIndex * sector + halfSector;

            // распределяем L(value)
            auto mainBasketValue = (1 - (abs(phi - mainBasketPhi) / sector)) * value;
            auto sideBasketValue = value - mainBasketValue;

            // записываем значения
            firstBasketIndex = clamp(firstBasketIndex, 0, basketCount-1);
            secondBasketIndex = clamp(secondBasketIndex, 0, basketCount-1);
            baskets[firstBasketIndex] += mainBasketValue;
            baskets[secondBasketIndex] += sideBasketValue;
        }
    }

    // Ищем Пики
    auto peak_1 = getPeak(baskets);
    auto peak_2 = getPeak(baskets, peak_1);

    vector<double> peaks;
    peaks.push_back(parabaloidInterpolation(baskets, peak_1));
    if (peak_2 != -1 && baskets[peak_2] / baskets[peak_1] >= 0.8) { // Если второй пик не ниже 80%
        peaks.push_back(parabaloidInterpolation(baskets, peak_2));
    }
    return peaks;
}

/* Интерполяция параболой */
double DescriptorCreator::parabaloidInterpolation(const vector<double> &baskets, const int maxIndex) {
    // берём левую и правую корзину и интерполируем параболой
    auto left = baskets[(maxIndex - 1 + baskets.size()) % baskets.size()];
    auto right = baskets[(maxIndex + 1) % baskets.size()];
    auto mid = baskets[maxIndex];

    auto sector = 2 * M_PI / baskets.size();
    auto phi = (left - right) / (2 * (left + right - 2 * mid));
    return (phi + maxIndex) * sector + (sector / 2);
}

/* Поиск пика */
double DescriptorCreator::getPeak(const vector<double> &baskets, const int notEqual) {
    int maxBasketIndex = -1;
    for (unsigned int i = 0; i < baskets.size(); i++) {
        if (baskets[i] > baskets[(i - 1 + baskets.size()) % baskets.size()] && baskets[i] > baskets[(i + 1) % baskets.size()] && (int)i != notEqual) {
            if (maxBasketIndex != -1 && baskets[maxBasketIndex] > baskets[i]) {
                continue;
            }
            maxBasketIndex = i;
        }
    }
    return maxBasketIndex;
}

/*  Инвариантость к вращению и масштабу */
vector <Descriptor> DescriptorCreator::getDescriptorsInvRotationScale(Pyramid &pyramid, vector <Point> points,  const int _radius,
                                                                      const int basketCount, const int barCharCount)
{
    auto sigma = 20;
    auto sigma0 = pyramid.getDog(0).sigmaScale;
    auto sector = 2 * M_PI / basketCount;
    auto halfSector = M_PI / basketCount;
    auto barCharCountInLine = (barCharCount / 4);

    vector<Image> images_dx;
    vector<Image> images_dy;

    // Ищем производные заранее
    for (int i = 0; i< pyramid.getDogsSize(); i++) {
        Image &imageTrue = pyramid.getDog(i).trueImage;
        images_dx.push_back(ImageConverter::convolution(imageTrue, KernelCreator::getSobelX()));
        images_dy.push_back(ImageConverter::convolution(imageTrue, KernelCreator::getSobelY()));
    }

    vector <Descriptor> descriptors(points.size());
    for (unsigned int k = 0; k < points.size(); k++) {
        descriptors[k] = Descriptor(barCharCount * basketCount, points[k]);

        double scale = (points[k].sigmaScale/sigma0);
        int radius = _radius * scale;
        int dimension = 2 * radius;
        double barCharStep = double(dimension) / (barCharCount / 4);
        Image &image_dx = images_dx[points[k].z];
        Image &image_dy = images_dy[points[k].z];

        // Ориентация точки
        auto peaks = getPointOrientation(image_dx, image_dy, points[k], sigma, radius);

        for (auto &phiRotate : peaks) {
            for (auto i = 0 ; i < dimension ; i++) {
                for (auto j = 0 ; j < dimension; j++) {
                    // координаты
                    auto coord_X = i - radius + points[k].x;
                    auto coord_Y = j - radius + points[k].y;

                    // градиент
                    auto gradient_X = image_dx.getPixel(coord_X, coord_Y);
                    auto gradient_Y = image_dy.getPixel(coord_X, coord_Y);

                    // получаем значение(домноженное на Гаусса) и угол
                    auto value = getGradientValue(gradient_X, gradient_Y)  * KernelCreator::getGaussValue(i, j, sigma * scale, radius);
                    auto phi = getGradientDirection(gradient_X, gradient_Y) + 2 * M_PI - phiRotate;
                    phi = fmod(phi, 2 * M_PI);  // Shift

                    // получаем индекс корзины в которую входит phi и смежную с ней
                    int firstBasketIndex = floor(phi / sector);
                    int secondBasketIndex = int(floor((phi - halfSector) / sector) + basketCount) % basketCount;

                    // вычисляем центр
                    auto mainBasketPhi = firstBasketIndex * sector + halfSector;

                    // распределяем L(value)
                    auto mainBasketValue = (1 - (abs(phi - mainBasketPhi) / sector)) * value;
                    auto sideBasketValue = value - mainBasketValue;

                    // вычисляем индекс куда записывать значения
                    int i_Rotate = round((i - radius) * cos(phiRotate) +(j- radius) * sin(phiRotate));
                    int j_Rotate = round(-(i - radius) * sin(phiRotate) + (j- radius) * cos(phiRotate));

                    // отбрасываем
                    if (i_Rotate < -radius || j_Rotate < -radius || i_Rotate >= radius || j_Rotate >= radius) {
                        continue;
                    }

                    int tmp_i = (i_Rotate + radius) / barCharStep;
                    int tmp_j = (j_Rotate + radius) / barCharStep;

                    auto indexMain = (tmp_i * barCharCountInLine + tmp_j) * basketCount + firstBasketIndex;
                    auto indexSide = (tmp_i * barCharCountInLine + tmp_j) * basketCount + secondBasketIndex;

                    // записываем значения
                    descriptors[k].data[indexMain] += mainBasketValue;
                    descriptors[k].data[indexSide] += sideBasketValue;
                }
            }
            descriptors[k].normalize();
            descriptors[k].clampData(0, 0.2);
            descriptors[k].normalize();
        }
    }

    for (unsigned int i = 0; i < descriptors.size(); i++) {
        //приводим к оригинальному масштабу
        Point interPoint = descriptors[i].getInterPoint();
        double step_W = double(pyramid.getDog(0).image.getWidth()) / pyramid.getDog(interPoint.z).image.getWidth();
        double step_H = double(pyramid.getDog(0).image.getHeight()) / pyramid.getDog(interPoint.z).image.getHeight();
        descriptors[i].setPointXY(round(interPoint.x * step_W), round(interPoint.y * step_H));
    }
    return descriptors;
}

vector<Descriptor> DescriptorCreator::getDescriptorsInvRotationScaleAfinn(Pyramid &pyramid, vector<Point> points,
                                                                          const int _radius,const int basketCount, const int barCharCount)
{
    auto sigma = 20;
    auto sigma0 = pyramid.getDog(0).sigmaScale;
    auto sector = 2 * M_PI / basketCount;
    auto halfSector = M_PI / basketCount;
    auto barCharCountInLine = (barCharCount / 4);

    vector<Image> images_dx;
    vector<Image> images_dy;

    // Ищем производные заранее
    for (int i = 0; i< pyramid.getDogsSize(); i++) {
        Image &imageTrue = pyramid.getDog(i).trueImage;
        images_dx.push_back(ImageConverter::convolution(imageTrue, KernelCreator::getSobelX()));
        images_dy.push_back(ImageConverter::convolution(imageTrue, KernelCreator::getSobelY()));
    }

    vector <Descriptor> descriptors(points.size());
    for (unsigned int k = 0; k < points.size(); k++) {
        descriptors[k] = Descriptor(barCharCount * basketCount, points[k]);

        double scale = (points[k].sigmaScale/sigma0);
        int radius = _radius * scale;
        int dimension = 2 * radius;
        double barCharStep = double(dimension) / (barCharCount / 4);
        Image &image_dx = images_dx[points[k].z];
        Image &image_dy = images_dy[points[k].z];

        // Ориентация точки
        auto peaks = getPointOrientation(image_dx, image_dy, points[k], sigma, radius);

        for (auto &phiRotate : peaks) {
            for (auto i = 0 ; i < dimension ; i++) {
                for (auto j = 0 ; j < dimension; j++) {
                    // координаты
                    auto coord_X = i - radius + points[k].x;
                    auto coord_Y = j - radius + points[k].y;

                    // градиент
                    auto gradient_X = image_dx.getPixel(coord_X, coord_Y);
                    auto gradient_Y = image_dy.getPixel(coord_X, coord_Y);

                    // получаем значение(домноженное на Гаусса) и угол
                    auto value = getGradientValue(gradient_X, gradient_Y) * KernelCreator::getGaussValue(i, j, sigma * scale, radius);
                    auto phi = getGradientDirection(gradient_X, gradient_Y) + 2 * M_PI - phiRotate;
                    phi = fmod(phi, 2 * M_PI);  // Shift

                    // получаем индекс корзины в которую входит phi и смежную с ней
                    int firstBasketIndex = floor(phi / sector);
                    int secondBasketIndex = int(floor((phi - halfSector) / sector) + basketCount) % basketCount;

                    // вычисляем центр
                    auto mainBasketPhi = firstBasketIndex * sector + halfSector;

                    // распределяем L(value)
                    auto mainBasketValue = (1 - (abs(phi - mainBasketPhi) / sector)) * value;
                    auto sideBasketValue = value - mainBasketValue;

                    // вычисляем индекс куда записывать значения
                    int i_Rotate = round((i - radius) * cos(phiRotate) + (j - radius) * sin(phiRotate));
                    int j_Rotate = round(-(i - radius) * sin(phiRotate) + (j - radius) * cos(phiRotate));

                    // отбрасываем
                    if (i_Rotate < -radius || j_Rotate < -radius || i_Rotate >= radius || j_Rotate >= radius) {
                        continue;
                    }

                    // координаты точки в дискрипторе
                    int true_i = (i_Rotate + radius);
                    int true_j = (j_Rotate + radius);

                    int half = barCharStep / 2;

                    // отнимаем половинку для поиска ближайших 4 гистограмм
                    int disk_i = (true_i - half + dimension) % dimension;
                    int disk_j = (true_j - half + dimension) % dimension;

                    // i j левой верхней гистограммы
                    int gist_i = disk_i / barCharStep;
                    int gist_j = disk_j / barCharStep;

                    // индексы 4-ех гистограмм
                    int gist[4];
                    gist[0] = gist_i * barCharCountInLine + gist_j;
                    gist[1] = gist_i + 1 >= barCharCountInLine ? -1 : (gist_i + 1) * barCharCountInLine + gist_j;
                    gist[2] = gist_j + 1 >= barCharCountInLine ? -1 :  gist_i * barCharCountInLine + (gist_j + 1),
                    gist[3] = (gist_i + 1 >= barCharCountInLine || gist_j + 1 >= barCharCountInLine) ? -1 :
                              (gist_i + 1) * barCharCountInLine + (gist_j + 1);

                    // добиваемся чтоб координаты были между 4 гистограммами
                    double tmp_i = fmod(true_i  + barCharStep/2, barCharStep);
                    double tmp_j = fmod(true_j  + barCharStep/2, barCharStep);

                    // считаем веса  по x и y
                    double wt_X = (barCharStep - tmp_i) / barCharStep;
                    double wt_Y = (barCharStep - tmp_j) / barCharStep;

                    // перемножаем для 4 гистограмм
                    double wt[4] = {wt_X * wt_Y, (1 - wt_X) * wt_Y,
                                    wt_X *(1 - wt_Y), (1 - wt_X) * (1 - wt_Y)};

                    // считаем индексы и записываем значения
                    for (int i = 0; i < 4; i++) {
                        if (gist[i] != -1) {
                            int index = gist[i] * basketCount;
                            descriptors[k].data[index + firstBasketIndex] += wt[i] * mainBasketValue;
                            descriptors[k].data[index + secondBasketIndex] += wt[i] * sideBasketValue;
                        }
                    }
                }
            }
            descriptors[k].normalize();
            descriptors[k].clampData(0, 0.2);
            descriptors[k].normalize();
        }
    }

    for (unsigned int i = 0; i < descriptors.size(); i++) {
        //приводим к оригинальному масштабу
        Point interPoint = descriptors[i].getInterPoint();
        double step_W = double(pyramid.getDog(0).image.getWidth()) / pyramid.getDog(interPoint.z).image.getWidth();
        double step_H = double(pyramid.getDog(0).image.getHeight()) / pyramid.getDog(interPoint.z).image.getHeight();
        descriptors[i].setPointXY(round(interPoint.x * step_W), round(interPoint.y * step_H));
    }
    return descriptors;
}


vector <Vector> DescriptorCreator::findSimilar(const vector <Descriptor> &d1, const vector <Descriptor> &d2, const double treshhold) {
    vector <Vector> similar;
    for (unsigned int i = 0; i < d1.size(); i++) {
        int indexSimilar = -1;
        double prevDistance = numeric_limits<double>::max();       // Предыдущий
        double minDistance = numeric_limits<double>::max();        // Минимальный
        for (unsigned int j = 0; j < d2.size(); j++) {
            double dist = getDistance(d1[i], d2[j]);
            if (dist < minDistance) {
                indexSimilar = j;
                prevDistance = minDistance;
                minDistance = dist;
            }
        }

        if (minDistance / prevDistance > treshhold) {
            continue;      // отбрасываем
        } else {
            similar.emplace_back(d1[i], d2[indexSimilar]);
        }
    }
    return similar;
}



