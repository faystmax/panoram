#include "Ransac.h"

#include <QtGlobal>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <utility>
#include <unordered_set>

template<int Rows, int Cols>
Matrix<Rows, Cols>::Matrix(const int x, const int y){
    this->data[0] = x;
    this->data[1] = y;
    this->data[2] = 1;
}

/* Транспонированеие */
template<int Rows, int Cols>
Matrix<Cols, Rows> Matrix<Rows, Cols>::operator~() {
    Matrix<Cols, Rows> result;
    for (auto i = 0; i < Rows; i++) {
        for (auto j = 0; j < Cols; j++) {
            result.set(j, i, this->at(i, j));
        }
    }
    return result;
}

Ransac::Ransac() {
    srand(time(0));
}

Matrix<9, 1> Ransac::search(vector<Vector> &lines, const double threshhold) {

    array<int,4> numbers;
    int bestInliers = -1;
    int const count = 600;
    array<pair<array<int,4>,int>,count> linesInliers;
    Matrix<9, 1> best;
    for (auto i = 0; i < count; i++) {
        // Генерим рандомные числа
        std::generate_n(numbers.begin(), 4, [&lines]() {return std::rand() % lines.size();});

        // Получаем гипотезу
        Matrix<9, 1> hypothesis = getHypothesis(lines[numbers[0]],lines[numbers[1]],lines[numbers[2]],lines[numbers[3]]);

        // Считаем inliers
        int inliers = countInliers(hypothesis, lines, threshhold);
        if (inliers > bestInliers) {
            bestInliers = inliers;
            best = hypothesis;
        }
        linesInliers[i] = std::make_pair(numbers, inliers);
    }
    vector<int> indxLines;
    for (auto i = 0; i < count; i++) {
        if(linesInliers[i].second == bestInliers){
            indxLines.push_back(linesInliers[i].first[0]);
            indxLines.push_back(linesInliers[i].first[1]);
            indxLines.push_back(linesInliers[i].first[2]);
            indxLines.push_back(linesInliers[i].first[3]);
        }
    }
    // удаление повторяющихся элементов
    std::sort(indxLines.begin(), indxLines.end());
    auto last = std::unique(indxLines.begin(), indxLines.end());
    indxLines.erase(last, indxLines.end());

    return correctDLT(indxLines, lines);
}

Matrix<9, 1> Ransac::correctDLT(const vector<int> &indxLines, vector<Vector> &lines)
{
    // Инициализируем матрицу A
    int size = indxLines.size() > 24 ? 24 : indxLines.size();
    int rows = size * 2;
    int cols = 9;
    vector<double> matr_A(rows * cols, 0);
    for(int i = 0;i < size; i++){

        double x1 = lines[indxLines[i]].second.getInterPointRef().x;
        double y1 = lines[indxLines[i]].second.getInterPointRef().y;
        double x1_s = lines[indxLines[i]].first.getInterPointRef().x;
        double y1_s = lines[indxLines[i]].first.getInterPointRef().y;

        int idx = i * 2 * 9;
        matr_A[idx + 0] = x1;         matr_A[idx + 1] = y1;          matr_A[idx + 2] = 1;
        matr_A[idx + 6] = -x1_s * x1; matr_A[idx + 7] = -x1_s * y1;  matr_A[idx + 8]  = -x1_s;

        idx  +=  9;
        matr_A[idx + 3] = x1;         matr_A[idx + 4] = y1;          matr_A[idx + 5] = 1;
        matr_A[idx + 6] = -y1_s * x1; matr_A[idx + 7] = -y1_s * y1;  matr_A[idx + 8]  = -y1_s;
    }


    // Транспонируем и перемножаем
    vector<double> transp_A = transpose(rows , cols, matr_A);
    vector<double> res = multiply(cols, rows , cols,transp_A, matr_A);

    // Кладём в real_2d_array
    real_2d_array matr, u, vt;
    matr.setcontent(9, 9, &res[0]);

    // Считаем SVD
    real_1d_array w;
    bool isSucces = rmatrixsvd(matr, 9, 9, 2, 0, 2, w, u, vt);
    Q_ASSERT(isSucces);

    // так как  W - contains singular values in descending order.
    // берём последний столбец в u
    Matrix<9, 1> hypothesis;
    double koef = 1.0 / u[8][u.cols()-1];  // Делим на последний элемент в матрице - чтоб h22 = 1
    for (auto i = 0; i < hypothesis.getRows(); i++) {
        hypothesis.set(i, 0, koef * u[i][u.cols()-1]);
    }
    return hypothesis;
}

/* Перемножение для векторов */
vector<double> Ransac::multiply(int rows, int cols_rows, int cols,const vector<double>& m1,const vector<double>& m2){
    vector<double> result(rows * cols);
    for (auto i = 0; i < rows; i++) {
        for (auto j = 0; j < cols; j++) {
           double sum = 0;
           for (auto k = 0; k < cols_rows; k++) {
               sum += m1[i * cols_rows + k] * m2[k * cols + j];
           }
            result[i * cols + j] = sum;
        }
    }
    return result;
}

vector<double> Ransac::transpose(int rows, int cols, const vector<double> &m1){
    vector<double> result(m1.size());
    for (auto i = 0; i < rows; i++) {
        for (auto j = 0; j < cols; j++) {
            result[j * rows + i] = m1[i * cols + j];
        }
    }
    return result;
}

Matrix<3, 1> Ransac::convert(const Matrix<9, 1> &transMatrix, const int x, const int y) {
    Matrix<3, 1> a(x, y);

    // Строим матрицу h 3 на 3
    Matrix<3, 3> h(transMatrix.getData());

    // Находим новые координаты
    return h * a;
}

Matrix<9, 1> Ransac::getHypothesis(Vector &line_1, Vector &line_2, Vector &line_3, Vector &line_4) {
    // Переименовываем для простоты использования
    double x1 = line_1.second.getInterPointRef().x;
    double y1 = line_1.second.getInterPointRef().y;
    double x1_s = line_1.first.getInterPointRef().x;
    double y1_s = line_1.first.getInterPointRef().y;

    double x2 = line_2.second.getInterPointRef().x;
    double y2 = line_2.second.getInterPointRef().y;
    double x2_s = line_2.first.getInterPointRef().x;
    double y2_s = line_2.first.getInterPointRef().y;

    double x3 = line_3.second.getInterPointRef().x;
    double y3 = line_3.second.getInterPointRef().y;
    double x3_s = line_3.first.getInterPointRef().x;
    double y3_s = line_3.first.getInterPointRef().y;

    double x4 = line_4.second.getInterPointRef().x;
    double y4 = line_4.second.getInterPointRef().y;
    double x4_s = line_4.first.getInterPointRef().x;
    double y4_s = line_4.first.getInterPointRef().y;

    // Инициализируем матрицу A
    array<double, 8 * 9> matr_A_data = { x1, y1, 1, 0,   0,  0, -x1_s * x1, -x1_s * y1, -x1_s,
                                         0,  0,  0, x1, y1,  1, -y1_s * x1, -y1_s * y1, -y1_s,
                                         x2, y2, 1, 0,   0,  0, -x2_s * x2, -x2_s * y2, -x2_s,
                                         0,  0,  0, x2, y2,  1, -y2_s * x2, -y2_s * y2, -y2_s,
                                         x3, y3, 1, 0,   0,  0, -x3_s * x3, -x3_s * y3, -x3_s,
                                         0,  0,  0, x3, y3,  1, -y3_s * x3, -y3_s * y3, -y3_s,
                                         x4, y4, 1, 0,   0,  0, -x4_s * x4, -x4_s * y4, -x4_s,
                                         0,  0,  0, x4, y4,  1, -y4_s * x4, -y4_s * y4, -y4_s,
                                       };

    // Транспонируем и перемножаем
    Matrix<8, 9> matr_A(matr_A_data);
    matr_A.setData(matr_A_data);
    Matrix<9, 8> transp_A = ~matr_A;
    Matrix<9, 9> matr_ATA = transp_A * matr_A;

    // Кладём в real_2d_array
    real_2d_array matr, u, vt;
    matr.setcontent(9, 9, &matr_ATA.getData().at(0));

    // Считаем SVD
    real_1d_array w;
    bool isSucces = rmatrixsvd(matr, 9, 9, 2, 0, 2, w, u, vt);
    Q_ASSERT(isSucces);

    // так как  W - contains singular values in descending order.
    // берём последний столбец в u
    Matrix<9, 1> hypothesis;
    double koef = 1.0 / u[8][u.cols()-1];  // Делим на последний элемент в матрице - чтоб h22 = 1
    for (auto i = 0; i < hypothesis.getRows(); i++) {
        hypothesis.set(i, 0, koef * u[i][u.cols()-1]);
    }

    return hypothesis;
}

int Ransac::countInliers(const Matrix<9, 1> &hyp, const vector<Vector> &lines, const double threshhold) {
    int inliers = 0;
    for (auto i = 0; i < lines.size(); i++) {
        // Находим новые координаты
        Matrix<3, 1> newCoord = convert(hyp, lines[i].second.getInterPoint().x, lines[i].second.getInterPoint().y);

        // Считаем разницу
        double distance = sqrt(pow(newCoord.at(0,0) - lines[i].first.getInterPoint().x, 2) +
                               pow(newCoord.at(1,0) - lines[i].first.getInterPoint().y, 2));
        if (distance <= threshhold) {
            inliers++;
        }
    }
    return inliers;
}
