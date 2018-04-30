#include "MainWindow.h"
#include <QApplication>
#include <stdio.h>
#include <omp.h>

int main(int argc, char *argv[]) {

#if defined(_OPENMP)
    printf("Compiled by an OpenMP-compliant implementation.\n");
    printf("The result of omp_get_num_threads %i\n", omp_get_num_threads());
#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
