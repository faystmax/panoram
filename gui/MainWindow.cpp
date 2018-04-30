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
    images[0] = constructImage(QImage(":/resource/img/resource/img/1.jpg"));
    images[1] = constructImage(QImage(":/resource/img/resource/img/2.jpg"));
    images[2] = constructImage(QImage(":/resource/img/resource/img/3.jpg"));;
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

    vector<vector<Descriptor>> descriptors(3);
    #pragma omp parallel for
    for (int i = 0; i < 3; i++) {
        Pyramid pyramid(images[i]);
        vector <Point> points = InterestPoints::blob(pyramid);
        descriptors[i] = DescriptorCreator::getDescriptorsInvRotationScale(pyramid, points);
    }

    // Glue and draw Panoram
    vector<Vector>  similar12 = DescriptorCreator::findSimilar(descriptors[0], descriptors[1]);
    vector<Vector>  similar23 = DescriptorCreator::findSimilar(descriptors[1], descriptors[2]);
    vector<Vector>  similar13 = DescriptorCreator::findSimilar(descriptors[0], descriptors[2]);
    cout<<similar12.size()<<" "<<similar23.size()<<" "<<similar13.size()<<std::endl;

    auto transformMatrix = Ransac::search(similar23);
    QImage panoram = glueImagesPanoram(images[1],images[2], transformMatrix);
    showImage(panoram);
}

