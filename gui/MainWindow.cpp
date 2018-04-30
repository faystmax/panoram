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

#include "ImageUtil.cpp"
#include "Kernel.h"
#include "ImageConverter.h"
#include "Descriptor.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->setStyleSheet("background-color: rgb(217, 217, 217);");
    this->ui->graphicsView->setScene(new QGraphicsScene());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openButton_clicked()
{

}

void MainWindow::on_gluePanoramButton_clicked()
{

}

void MainWindow::on_imageFirstButton_clicked()
{

}

void MainWindow::on_imageSecondButton_clicked()
{

}

void MainWindow::on_imageThirdButton_clicked()
{

}

void MainWindow::on_imagesDefaultButton_clicked()
{

}

void MainWindow::on_action_open_triggered()
{

}

void MainWindow::on_action_save_triggered()
{

}

void MainWindow::on_action_default_triggered()
{

}

void MainWindow::on_action_exit_triggered()
{
    QApplication::quit();
}


void MainWindow::on_action_about_triggered()
{

}

void MainWindow::openFiles() {
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Выберите 3 изображения", nullptr, "Image Files (*.png *.jpg *.bmp)");

    if(fileNames.size() != 3){
        QMessageBox::critical(this, "Ошибка", "Нужно выбрать 3 файла изображения для склеивания панорамы!");
    } else{
        for(int i= 0; i < fileNames.size(); i++){
            QImage qImage(fileNames[i]);
            if(!qImage.isNull()){
                images[i] = constructImage(qImage);
            } else {
                QMessageBox::critical(this, "Ошибка", "Ошибка чтения изображения:" + fileNames[i]);
            }
        }
        showAllImages();
    }
}

void MainWindow::showImage(const Image &image) {
    showImage(getOutputImage(image));
}


void MainWindow::showImage(const QImage &image) {
    this->ui->graphicsView->scene()->clear();
    this->ui->graphicsView->scene()->addItem(new QGraphicsPixmapItem(QPixmap::fromImage(image)));
}

void MainWindow::showAllImages()
{
     showImage(glueImages(images[0],images[1],images[2]));
}
