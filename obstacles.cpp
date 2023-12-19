#include "obstacles.h"

#include "math.h"

int randint(int a, int b) {
    return rand() % (b - a + 1) + a;
}
float randfloat(float a, float b) {
    return a + static_cast <float> (rand()) / static_cast<float>(RAND_MAX/(b - a));
}
QVector2D rand_pos() {
    while (true) {
        QVector2D res(randint(-16, 16), randint(-16, 16));
        if (res.length() >= 3) return res;
    }
}

Obstacles::Obstacles() {
    my_objs = static_cast<obstracle_t*>(malloc(sizeof(obstracle_t) * count));
    for (uint i = 0; i < count; i++) {
        my_objs[i].pos = rand_pos();
        my_objs[i].scale = randfloat(0.2f, 2.f);
        my_objs[i].rotate = randint(-180, 180);
        my_objs[i].dirty = true;
    }
}



#define line_dist2(a, b) (powf(a->x() - b->x(), 2) + powf(a->y() - b->y(), 2))
#define min(a, b) (a < b ? a : b)
#define min4(a, b, c, d) (min(a, min(b, min(c, d))))
#define max(a, b) (a > b ? a : b)
#define max4(a, b, c, d) (max(a, max(b, max(c, d))))
#define dot_to_canon_dist(p, A, B, C) ((A * p->x() + B * p->y() + C) / hypot(A, B))

float dot_to_line_dist(QVector2D *dot, QVector2D *p, QVector2D *p2) {
    float x = p->x(), y = p->y(), x2 = p2->x(), y2 = p2->y();
    float A = y - y2, B = x2 - x, C = x * y2 - x2 * y;
    return dot_to_canon_dist(dot, A, B, C);
};
float dot_to_segment_dist(QVector2D *dot, QVector2D *p, QVector2D *p2) {
    float dist = dot_to_line_dist(dot, p, p2);
    if (dist <= 0) return 0;
    float a = line_dist2(dot, p);
    float b = line_dist2(dot, p2);
    float c = line_dist2(p, p2);
    if (a + c <= b || b + c <= a) return sqrt(min(a, b));
    return dist;
}
float dot_to_quadrilateral(QVector2D *dot, QVector2D *points) {
    float a = dot_to_segment_dist(dot, points, points + 1);
    float b = dot_to_segment_dist(dot, points + 1, points + 2);
    float c = dot_to_segment_dist(dot, points + 2, points + 3);
    float d = dot_to_segment_dist(dot, points + 3, points);
    return max4(a, b, c, d);
    //return min((a > 0 ? a : c), (b > 0 ? b : d));
    //return min(max(a, c), max(b, d));
}



float Obstacles::checkCollision(QVector2D *fly) {
    float mimimin = INFINITY;
    const float deg2rad = 3.14159265358979323846f / 180.f;

    for (uint i = 0; i < count; i++) {
        obstracle_t *obj = my_objs + i;
        QVector2D *dots = obj->dots;
        if (obj->dirty) {
            QVector2D pos = obj->pos;
            float scale = obj->scale / 2;
            float rotate = obj->rotate;
            float diag = sqrt(scale * scale * 2);
            for (int i = 0; i < 4; i++) {
                float rad = (rotate + 45 + i * 90) * deg2rad;
                dots[i] = QVector2D(-cos(rad), sin(rad)) * diag + pos;
            }
            obj->dirty = false;
        }
        float dist = dot_to_quadrilateral(fly, dots);
        mimimin = min(mimimin, dist);
    }
    dbg = QString("YEAH! ") + QString::number(mimimin);
    return mimimin;
}

void Obstacles::drawObstacles(GLuint texture, GLuint model) {
    for (uint i = 0; i < count; i++) {
        obstracle_t *obj = my_objs + i;
        QVector2D pos = obj->pos;
        float scale = obj->scale / 2;
        float rotate = obj->rotate;
        //obj->rotate++;

        glBindTexture(GL_TEXTURE_2D, texture);
        glPushMatrix();
            glTranslatef(pos.x(), -5.f + 0.012f + scale, pos.y());
            glScalef(scale, scale, scale);
            glRotatef(rotate, 0, 1, 0);
            glCallList(model);
        glPopMatrix();
    }

    /*const float deg2rad = 3.14159265358979323846f / 180.f;
    float diag = sqrt(scale * scale * 2);
    QVector2D dots[4];
    for (int i = 0; i < 4; i++) {
        float rad = (rotate + 45 + i * 90) * deg2rad;
        dots[i] = QVector2D(-cos(rad), sin(rad)) * diag + pos;
        glPushMatrix();
            glTranslatef(dots[i].x(), -5.f + 0.012f + .1f, dots[i].y());
            glScalef(.1f, .1f, .1f);
            glCallList(model);
        glPopMatrix();
    }
    dbg = QString("YEAH! ") + QString::number(dot_to_quadrilateral(fly, dots));*/
}
