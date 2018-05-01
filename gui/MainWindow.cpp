#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDir>
#include <QLineEdit>
#include <QImage>
#include <QMessageBox>
#include <QFileDialog>
#include <QImageReader>
#include <QGraphicsPixmapItem>
#include <iostream>
#include <omp.h>

#include "ImageUtil.cpp"
#include "Kernel.h"
#include "ImageConverter.h"
#include "Descriptor.h"
#include "AboutDialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->statusBar->setStyleSheet("background-color: rgb(217, 217, 217);");
    this->ui->graphicsView->setScene(new QGraphicsScene());

    loadDefaultImages();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_openButton_clicked() {
    openFiles();
}

void MainWindow::on_gluePanoramButton_clicked() {
    gluePanoram();
}

void MainWindow::on_imageFirstButton_clicked() {
    showImage(images[0]);
}

void MainWindow::on_imageSecondButton_clicked() {
    showImage(images[1]);
}

void MainWindow::on_imageThirdButton_clicked() {
    showImage(images[2]);
}

void MainWindow::on_imageAllButton_clicked() {
    showAllImages();
}

void MainWindow::on_imagesDefaultButton_clicked() {
    loadDefaultImages();
}

void MainWindow::on_action_open_triggered() {
    openFiles();
}

void MainWindow::on_action_save_triggered() {
    saveImage();
}

void MainWindow::on_action_default_triggered() {
    loadDefaultImages();
}

void MainWindow::on_action_exit_triggered() {
    QApplication::quit();
}


void MainWindow::on_action_about_triggered() {
    AboutDialog dialog(this);
    dialog.setModal(true);
    dialog.exec();
}

void MainWindow::openFiles() {
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Выберите 3 изображения", nullptr, "Изображения (*.png *.jpg *.bmp)");

    if (fileNames.size() != 3) {
        QMessageBox::critical(this, "Ошибка", "Нужно выбрать 3 файла изображения для склеивания панорамы!");
    } else {
        for (int i= 0; i < fileNames.size(); i++) {
            QImage qImage(fileNames[i]);
            if (!qImage.isNull()) {
                images[i] = constructImage(qImage);
            } else {
                QMessageBox::critical(this, "Ошибка", "Ошибка чтения изображения: " + fileNames[i]);
            }
        }
        showAllImages();
    }
}

void MainWindow::saveImage() {
    QString fileName = QFileDialog::getSaveFileName(this,tr("Сохранение панорамы"), "out",tr("Изображения (*.png *.jpg *.bmp)"));

    if (fileName.isEmpty()) {
        return;
    } else {
        QImage outputImage(this->ui->graphicsView->scene()->sceneRect().size().toSize(), QImage::Format_RGB32);
        QPainter painter(&outputImage);
        this->ui->graphicsView->scene()->render(&painter);
        outputImage.save(fileName);
    }
}

void MainWindow::loadDefaultImages() {
    images[2] = constructImage(QImage(":/resource/img/resource/img/11111.jpg"));
    images[1] = constructImage(QImage(":/resource/img/resource/img/22222.jpg"));
    images[0] = constructImage(QImage(":/resource/img/resource/img/33333.jpg"));;
    showAllImages();
}

void MainWindow::showImage(const Image &image) {
    showImage(getOutputImage(image));
}


void MainWindow::showImage(const QImage &image) {
    this->ui->graphicsView->scene()->clear();
    this->ui->graphicsView->scene()->setSceneRect(0, 0, image.width(), image.height());
    this->ui->graphicsView->scene()->addItem(new QGraphicsPixmapItem(QPixmap::fromImage(image)));
}

void MainWindow::showAllImages() {
    showImage(glueImages(images[0],images[1],images[2]));
}

void MainWindow::gluePanoram() {

    vector<Descriptor> descriptors[3];
    this->ui->statusBar->showMessage("Ищем дескрипторы изображений...");
    #pragma omp parallel for
    for (int i = 0; i < 3; i++) {
        Pyramid pyramid(images[i]);
        vector <Point> points = InterestPoints::blob(pyramid);
        descriptors[i] = DescriptorCreator::getDescriptorsInvRotationScale(pyramid, points);
    }

    this->ui->statusBar->showMessage("Ищем правильный порядок в панораме...");

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
    vector<Vector>  similarCL , similarCR;
    do {
        similarCL = DescriptorCreator::findSimilar(descriptors[center], descriptors[left]);
        similarCR = DescriptorCreator::findSimilar(descriptors[center], descriptors[right]);

        transformMatrix_1 = Ransac::search(similarCL);
        transformMatrix_2 = Ransac::search(similarCR);

        shift_Left = transformMatrix_1.at(2,0);
        shift_Right = transformMatrix_2.at(2,0);

        if(shift_Left > 0  && shift_Right <0){
            swap(left, right);
        }
    } while (shift_Left>=0 || shift_Right <= 0);

    this->ui->statusBar->showMessage("Правильный порядок: " +  QString::number(left) + " " + QString::number(center) + " " + QString::number(right));
    QImage panoram = glueImagesPanoram(images[left],images[center],images[right], transformMatrix_1, transformMatrix_2);
    showImage(panoram);

//    Для отладки...
//    vector<Vector>  similar12 = DescriptorCreator::findSimilar(descriptors[1], descriptors[0]);
//    vector<Vector>  similar23 = DescriptorCreator::findSimilar(descriptors[1], descriptors[2]);
//    auto transformMatrix_1 = Ransac::search(similar12);
//    auto transformMatrix_2 = Ransac::search(similar23);
//    QImage panoram = glueImagesPanoram(images[0],images[1],images[2], transformMatrix_1, transformMatrix_2);
//    showImage(panoram);

//    auto transformMatrix = Ransac::search(similar12);
//    QImage panoram = glueImagesPanoram(images[1],images[0], transformMatrix);
//    showImage(panoram);

//    auto transformMatrix = Ransac::search(similar23);
//    QImage panoram = glueImagesPanoram(images[1],images[2], transformMatrix);
//    showImage(panoram);

//    Для отладки.. отрисовка связей дескриипторов
//   QImage result = glueImages(images[1],images[2]);
//   drawLinesAndCircles(result, images[1].getWidth(), similar23);
//   showImage(result);
}

