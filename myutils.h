#ifndef MYUTILS_H
#define MYUTILS_H

#include "math.h"
#include "qvector3d.h"

class MyUtils {
public:
    static int gauss(double **a, int n, double *res) {
        const double eps = 0.000001;  // точность
        //double *res = new double[n];
        int n1 = n + 1;
        for (int k = 0; k < n; k++) {
            // Поиск строки с максимальным a[i][k]
            double max = abs(a[k][k]);
            int index = k;
            for (int i = k + 1; i < n; i++)
                if (abs(a[i][k]) > max) {
                    max = abs(a[i][k]);
                    index = i;
                }
            // Перестановка строк
            if (max < eps) return 0;
            for (int j = 0; j < n1; j++) {
                double temp = a[k][j];
                a[k][j] = a[index][j];
                a[index][j] = temp;
            }
            // Нормализация уравнений
            for (int i = k; i < n; i++) {
                double temp = a[i][k];
                if (abs(temp) < eps) continue; // для нулевого коэффициента пропустить
                for (int j = 0; j < n1; j++)
                    a[i][j] /= temp;
                if (i == k)  continue; // уравнение не вычитать само из себя
                for (int j = 0; j < n1; j++)
                    a[i][j] -= a[k][j];
            }
        }
        // обратная подстановка
        for (int k = n - 1; k >= 0; k--) {
            res[k] = a[k][n];
            for (int i = 0; i < k; i++)
                a[i][n] -= a[i][k] * res[k];
        }
        return 1;
    }

    static void points2plane(double x, double y, double z, double x2, double y2, double z2, double x3, double y3, double z3, double &A, double &B, double &C, double &D) {
        double row_a[] = {x, y, z, 1, 0};
        double row_b[] = {x2, y2, z2, 1, 0};
        double row_c[] = {x3, y3, z3, 1, 0};
        double row_d[] = {x3, y3, z3, 1, 0};
        double* a[] = { row_a, row_b, row_c, row_d };
        double res[] = {0, 0, 0, 0};
        if (gauss(a, 4, res)) {
            A = res[0]; B = res[1]; C = res[2]; D = res[3];
        } else A = 123;
    }

    // point - координата точки линии
    // u - направляющий вектор линии
    // p_co - plain coordinate
    // p_no - plain normal
    static inline const QVector3D zero_point; // Причина CONFIG += c++1z
    static QVector3D insect_line_plane(QVector3D point, QVector3D u, QVector3D p_co, QVector3D p_no, int &valid) {
        float dot = QVector3D::dotProduct(p_no, u);
        if (abs(dot) < 0.000001) { valid = 0; return zero_point; } // Линия параллельна прямой

        float fac = -QVector3D::dotProduct(p_no, point - p_co) / dot;
        if (fac < 0) { valid = 0; return zero_point; } // Линия позади точки/камеры
        valid = 1;
        return point + u * fac;
    }
};
// QVector3D MyUtils::zero_point = QVector3D();

#endif // MYUTILS_H
