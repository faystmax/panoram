#ifndef RANSAC_H
#define RANSAC_H

#include "linalg.h"
#include "Descriptor.h"

#include <array>

using namespace alglib;

template <int Rows, int Cols>
class Matrix {
  public:
    Matrix() = default;
    Matrix(const int x, const int y);
    Matrix<Rows,Cols>(const array<double, Rows * Cols>& array)  { this->data = array;}
    Matrix(const Matrix &) = default ;
    Matrix(Matrix &&) = default;
    Matrix &operator=(Matrix &&) = default;
    Matrix &operator=(const Matrix &) = default;

    /* Перемножение */
    template<int Cols_2>
    Matrix<Rows, Cols_2> operator*(const Matrix<Cols, Cols_2>& a){
        Matrix<Rows, Cols_2> result;
        for (auto i = 0; i < Rows; i++) {
            for (auto j = 0; j < Cols_2; j++) {
                double sum = 0;
                for (auto k = 0; k < Cols; k++) {
                    sum += this->at(i, k) * a.at(k, j);
                }
                result.set(i, j, sum);
            }
        }
        return result;
    }

    Matrix<Cols,Rows> operator~();

    int getRows() const {return Rows;}
    int getCols() const {return Cols;}

    double at(const int i, const int j) const {return data[i * Cols + j];}
    void set(const int i, const int j, const double value) { data[i * Cols + j] = value;}
    void setData(const array<double, Rows * Cols> &data) { this->data = data;}
    array<double, Rows * Cols> getData() const { return data;}

private:
    array<double, Rows * Cols> data;
};

class Ransac {
public:
    // Поиск матрицы преобразования
    static Matrix<9, 1> search(vector<Vector> &lines, const double threshhold = 10);

    // Получает новые координаты из старых и матрицы преобразования
    static Matrix<3, 1> convert(const Matrix<9, 1>& transMatrix, const int x, const int y);

  private:
    static Matrix<9, 1> getHypothesis(Vector &line_1, Vector &line_2, Vector &line_3, Vector &line_4);
    static int countInliers(const Matrix<9, 1> &hyp, const vector<Vector> &lines, const double threshhold);
    static Matrix<9, 1> correctDLT(const vector<int> &indxLines, vector<Vector> &lines);

    static vector<double> multiply(int rows, int cols_rows, int cols, const vector<double>& m1, const vector<double>& m2);
    static vector<double> transpose(int rows, int cols, const vector<double>& m1);
};

#endif // RANSAC_H
