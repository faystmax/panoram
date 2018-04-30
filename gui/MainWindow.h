#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "Image.h"
#include "Pyramid.h"
#include "InterestPoints.h"
#include "Ransac.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_openButton_clicked();
    void on_gluePanoramButton_clicked();
    void on_imageFirstButton_clicked();
    void on_imageSecondButton_clicked();
    void on_imageThirdButton_clicked();
    void on_imageAllButton_clicked();

    void on_imagesDefaultButton_clicked();
    void on_action_open_triggered();
    void on_action_save_triggered();
    void on_action_default_triggered();
    void on_action_exit_triggered();

    void on_action_about_triggered();



private:
    Ui::MainWindow *ui;
    Image images[3];

    void openFiles();
    void saveImage();
    void showImage(const Image &image);
    void showImage(const QImage &image);
    void showAllImages();
};

#endif // MAINWINDOW_H
