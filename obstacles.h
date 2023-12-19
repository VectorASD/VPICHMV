#ifndef OBSTRACLES_H
#define OBSTRACLES_H

#include <QOpenGLWidget>
#include <QVector2D>

typedef struct obstacle obstracle_t;
struct obstacle {
    QVector2D pos;
    float rotate;
    float scale;
    bool dirty;
    QVector2D dots[4];
};

class Obstacles {
    uint count = 100;
    obstracle_t *my_objs;

public:
    Obstacles();

    float checkCollision(QVector2D *fly);
    void drawObstacles(GLuint texture, GLuint model);

    QString dbg;
};


#endif // OBSTRACLES_H
