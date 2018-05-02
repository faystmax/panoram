#include "Image.h"
#include "Pyramid.h"
#include "InterestPoints.h"
#include "Descriptor.h"
#include "Ransac.h"
#include <omp.h>

#include <QImage>
#include <QPixmap>
#include <QPainter>

QImage getOutputImage(const Image &image) {
    QImage resultImage(image.getWidth(), image.getHeight(), QImage::Format_ARGB32);
    Image outImage = image.getDeNormolize();
    for (int i = 0; i < outImage.getWidth(); i++) {
        for (int j = 0; j < outImage.getHeight(); j++) {
            double pixel = outImage.getPixel(i, j);
            resultImage.setPixel(i, j, qRgb(pixel, pixel, pixel));
        }
    }
    return resultImage;
}

Image constructImage(const QImage &image, const Image::EdgeEffect edgeEffect = Image::EdgeEffect::Repeat) {

    Image resultImage(image.width(), image.height(), edgeEffect);

    // Read pixels and form black and white image
    for (int i = 0; i < resultImage.getWidth(); i++) {
        for (int j = 0; j < resultImage.getHeight(); j++) {
            QRgb pixel = image.pixel(i, j);
            resultImage.setPixel(i, j, 0.213 * qRed(pixel) + 0.715 * qGreen(pixel) + 0.072 * qBlue(pixel));
        }
    }
    resultImage.normolizePixels();
    return resultImage;
}

QImage glueImages(const Image &imageLeft, const Image &imageRight) {

    // max height
    auto height = max(imageLeft.getHeight(),imageRight.getHeight());

    Image denormImageLeft = imageLeft.getDeNormolize();
    Image denormImageRight = imageRight.getDeNormolize();

    QImage resultImage(denormImageLeft.getWidth() + denormImageRight.getWidth(), height, QImage::Format_ARGB32);
    // imageLeft
    for (auto i = 0; i < denormImageLeft.getWidth(); i++) {
        for (auto j = 0; j < denormImageLeft.getHeight(); j++) {
            double pixel = denormImageLeft.getPixel(i, j);
            resultImage.setPixel(i, j, qRgb(pixel, pixel, pixel));
        }
    }

    // imageRight
    for (auto i = 0; i < denormImageRight.getWidth(); i++) {
        for (auto j = 0; j < denormImageRight.getHeight(); j++) {
            double pixel = denormImageRight.getPixel(i, j);
            resultImage.setPixel(i + denormImageLeft.getWidth(), j, qRgb(pixel, pixel, pixel));
        }
    }
    return resultImage;
}

QImage glueImages(const Image &imageLeft, const Image &imageCenter, const Image &imageRight) {

    // max height
    auto height = max(imageLeft.getHeight(),imageCenter.getHeight());
    height = max(height,imageRight.getHeight());

    Image denormImageLeft = imageLeft.getDeNormolize();
    Image denormImageCenter = imageCenter.getDeNormolize();
    Image denormImageRight = imageRight.getDeNormolize();

    QImage resultImage(denormImageLeft.getWidth() + denormImageCenter.getWidth() + denormImageRight.getWidth() + 9, height, QImage::Format_ARGB32);
    // imageLeft
    for (auto i = 0; i < denormImageLeft.getWidth(); i++) {
        for (auto j = 0; j < denormImageLeft.getHeight(); j++) {
            double pixel = denormImageLeft.getPixel(i, j);
            resultImage.setPixel(i, j, qRgb(pixel, pixel, pixel));
        }
    }

    // imageCenter
    for (auto i = 0; i < denormImageCenter.getWidth(); i++) {
        for (auto j = 0; j < denormImageCenter.getHeight(); j++) {
            double pixel = denormImageCenter.getPixel(i, j);
            resultImage.setPixel(i + denormImageLeft.getWidth() + 3, j, qRgb(pixel, pixel, pixel));
        }
    }

    // imageRight
    for (auto i = 0; i < denormImageRight.getWidth(); i++) {
        for (auto j = 0; j < denormImageRight.getHeight(); j++) {
            double pixel = denormImageRight.getPixel(i, j);
            resultImage.setPixel(i + denormImageLeft.getWidth() + denormImageCenter.getWidth() + 6, j, qRgb(pixel, pixel, pixel));
        }
    }
    return resultImage;
}

// Строим панораму из 2 изображений
QImage glueImagesPanoram(const Image &imageLeft, const Image &imageRight, const Matrix<9, 1> &matr) {

    QImage outputLeftImage = getOutputImage(imageLeft.getDeNormolize());
    QImage outputRightImage = getOutputImage(imageRight.getDeNormolize());

    // transform matrix
    QTransform transform(matr.at(0,0),matr.at(3,0),matr.at(6,0),
                         matr.at(1,0),matr.at(4,0),matr.at(7,0),
                         matr.at(2,0),matr.at(5,0),matr.at(8,0));


    // Создаём Новое изображение
    auto height = max(outputLeftImage.height(), outputRightImage.height() + 100); // высоту возьмём с запасом
    QImage resultImage(outputLeftImage.width() + outputRightImage.width(), height, QImage::Format_ARGB32);

    // Склеиваем панораму
    QPainter painter;
    painter.begin(&resultImage);
    painter.drawImage(0,0,outputLeftImage);
    painter.setTransform(transform);
    painter.drawImage(0,0,outputRightImage);

    return resultImage;
}

// Строим панораму из 3 изображений
QImage glueImagesPanoram(const Image &imageLeft,const Image &imageCenter, const Image &imageRight,
                         const Matrix<9, 1> &matr_1, const Matrix<9, 1> &matr_2) {

    QImage outputLeftImage = getOutputImage(imageLeft.getDeNormolize());
    QImage outputCenterImage = getOutputImage(imageCenter.getDeNormolize());
    QImage outputRightImage = getOutputImage(imageRight.getDeNormolize());

    // transform matrix
    QTransform transform_1(matr_1.at(0,0),matr_1.at(3,0),matr_1.at(6,0),
                           matr_1.at(1,0),matr_1.at(4,0),matr_1.at(7,0),
                           matr_1.at(2,0),matr_1.at(5,0),matr_1.at(8,0));


    QTransform transform_2(matr_2.at(0,0),matr_2.at(3,0),matr_2.at(6,0),
                           matr_2.at(1,0),matr_2.at(4,0),matr_2.at(7,0),
                           matr_2.at(2,0),matr_2.at(5,0),matr_2.at(8,0));


    // Создаём Новое изображение
    auto height = max(outputLeftImage.height(), outputRightImage.height());
    height = max(height, outputCenterImage.height() + 200);
    QImage resultImage(outputLeftImage.width() + outputCenterImage.width() + outputRightImage.width(), height, QImage::Format_ARGB32);
    resultImage.fill(QColor::fromRgb(255,255,255));

    // Склеиваем панораму
    QPainter painter;
    painter.begin(&resultImage);
    painter.translate(100,0);
    painter.drawImage(0, 0, outputCenterImage);
    painter.setTransform(transform_1,true);
    painter.drawImage(0, 0, outputLeftImage);
    painter.end();

    // Используем 2 painterтак как 2 setTransform ведут себя некоректно
    QPainter painter2;
    painter2.begin(&resultImage);
    painter2.translate(100,0);
    painter2.setTransform(transform_2, true);
    painter2.drawImage(0, 0, outputRightImage);
    painter2.end();
    return resultImage;
}


inline vector<QColor> randomColors(int count) {
    vector<QColor> colors;
    float currentHue = 0.0;
    for (int i = 0; i < count; i++) {
        colors.push_back(QColor::fromHslF(currentHue, 1.0, 0.5));
        currentHue += 0.618033988749895f;
        currentHue = std::fmod(currentHue, 1.0f);
    }
    return colors;
}

void drawLinesAndCircles(QImage &image, const int firstWidth, vector<Vector> similar) {
    QPainter painter(&image);
    QPen pen;
    pen.setWidth(1);
    vector<QColor> colors = randomColors(similar.size());
    for (unsigned int i = 0; i < similar.size(); i++) {
        pen.setColor(colors[i]);
        painter.setPen(pen);
        Point p1 = similar[i].first.getInterPoint();
        Point p2 = similar[i].second.getInterPoint();
        painter.drawLine(p1.x, p1.y, p2.x + firstWidth, p2.y);

        // Circle 1
        double radius1 = sqrt(2) * p1.sigmaEffect;
        painter.drawEllipse(QRect(p1.x - radius1, p1.y - radius1, 2 * radius1, 2 * radius1));

        // Circle 2
        double radius2 = sqrt(2) * p2.sigmaEffect;
        painter.drawEllipse(QRect(p2.x + firstWidth - radius2, p2.y - radius2, 2 * radius2, 2 * radius2));

    }
    painter.end();
}

void drawLines(QImage &image, const int firstWidth, vector<Vector> similar) {
    QPainter painter(&image);
    QPen pen;
    pen.setWidth(1);
    vector<QColor> colors = randomColors(similar.size());
    for (unsigned int i = 0; i < similar.size(); i++) {
        pen.setColor(colors[i]);
        painter.setPen(pen);
        Point p1 = similar[i].first.getInterPoint();
        Point p2 = similar[i].second.getInterPoint();
        painter.drawLine(p1.x, p1.y, p2.x + firstWidth,  p2.y);
    }
    painter.end();
}

QImage createImageWithPointsBlob(const Image &image, const vector <Point> &points) {
    QImage resultImage = getOutputImage(image);
    QPainter painter(&resultImage);
    QPen pen;
    pen.setWidth(2);
    vector<QColor> colors = randomColors(points.size());
    for (unsigned int i = 0; i < points.size(); i++) {
        pen.setColor(colors[i]);
        painter.setPen(pen);
        double radius = sqrt(2) * points[i].sigmaEffect;
        painter.drawEllipse(QRect(points[i].x - radius, points[i].y - radius, 2 * radius, 2 * radius));
        painter.drawPoint(points[i].x, points[i].y);
    }
    painter.end();
    return resultImage;
}

QImage gluePanoram(array<Image,3> images) {

    vector<Descriptor> descriptors[3];
    #pragma omp parallel for
    for (int i = 0; i < 3; i++) {
        Pyramid pyramid(images[i]);
        vector <Point> points = InterestPoints::blob(pyramid);
        descriptors[i] = DescriptorCreator::getDescriptorsInvRotationScale(pyramid, points);
    }

    // Сравниваем каждое с каждым
    int similarCount[3];
    vector<Vector>  similar11 = DescriptorCreator::findSimilar(descriptors[0], descriptors[1]);
    vector<Vector>  similar12 = DescriptorCreator::findSimilar(descriptors[0], descriptors[2]);
    similarCount[0] = similar11.size() + similar12.size();

    vector<Vector>  similar21 = DescriptorCreator::findSimilar(descriptors[1], descriptors[0]);
    vector<Vector>  similar22 = DescriptorCreator::findSimilar(descriptors[1], descriptors[2]);
    similarCount[1] = similar21.size() + similar22.size();

    vector<Vector>  similar31 = DescriptorCreator::findSimilar(descriptors[2], descriptors[0]);
    vector<Vector>  similar32 = DescriptorCreator::findSimilar(descriptors[2], descriptors[1]);
    similarCount[2] = similar31.size() + similar32.size();

    // Индекс максимального изображения
    int maxIndex = distance(similarCount, max_element(similarCount, similarCount + 3));

    int left = (maxIndex + 1) % 3;
    int center = maxIndex;
    int right = (maxIndex + 2) % 3;

    double shift_Left, shift_Right;
    Matrix<9, 1> transformMatrix_1, transformMatrix_2;
    vector<Vector>  similarCL, similarCR;

    // Корректировка левого и правого изображения
    do {
        similarCL = DescriptorCreator::findSimilar(descriptors[center], descriptors[left]);
        similarCR = DescriptorCreator::findSimilar(descriptors[center], descriptors[right]);

        transformMatrix_1 = Ransac::search(similarCL);
        transformMatrix_2 = Ransac::search(similarCR);

        shift_Left = transformMatrix_1.at(2,0);
        shift_Right = transformMatrix_2.at(2,0);

        if (shift_Left > 0  && shift_Right <0) {
            swap(left, right);
        }
    } while (shift_Left>=0 || shift_Right <= 0);

    return glueImagesPanoram(images[left],images[center],images[right], transformMatrix_1, transformMatrix_2);
}
