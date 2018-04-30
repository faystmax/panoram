#include "AboutDialog.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog) {
    ui->setupUi(this);
}

AboutDialog::~AboutDialog() {
    delete ui;
}

void AboutDialog::on_closeButton_clicked() {
    this->close();
}
