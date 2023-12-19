#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QOpenGLWidget>
#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QPainter>
#include <QTime>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>

//#include <GL/glu.h>
#include "camera.h"
#include "resultator.h"
#include "myutils.h"
#include "obstacles.h"
#include "asdloader.h"
#include "celler.h"

namespace Ui {
    class MyWidget;
}

const int models = 6;
const int textures = 5;
const int asdmodels = 31 + 28 + 26 + 25 + 55 + 5 + 1 + 30 + 43 + 13;
const int fps_len = 25;

class MyWidget;

class Imager {
    int id; MyWidget *eng;
    renderable_p m = nullptr;
    int w, h;
    QImage res;
public:
    Imager(MyWidget *engine, int id, int width = 64, int height = 64) {
        if (id < 0 || id >= asdmodels) id = 170;
        eng = engine; this->id = id; w = width; h = height;
    }
    void glThread();
    QImage noglThread(int width, int height);
    void setSize(int width, int height) { w = width; h = height; }
};

class MyWidget: public QOpenGLWidget {
    Q_OBJECT
    QTimer *paintTimer;
    GLuint model[models];
    GLuint texture[textures];
    GLuint torus = 0;
    renderable *asd_model[asdmodels] {nullptr};
    bool models_ready = false;
    edger_awaiter awaiter;

    QTime time;
    float current_time = 0, prev_time = 0, time_delta = 0;

    GLuint current_texture = 0;
    GLuint current_model = 0;

    QPoint screenSize;
    Camera3D m_camera;
    float FOV = 90;
    QMatrix4x4 proj_mat;
    QMatrix4x4 view_mat;

    void initLight();
    GLuint drawQube(float repeat = 1);
    void loadGLTextures();
    void initTexture(uint index, QImage &texture);

    virtual void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *, QEvent *event);
    void eventHandler();
    bool isKeyPressed[512 + 33 + 2]; // 2 -> padding
    bool is_click;
    bool isMousePressed[32];
    QPoint pressMousePos;
    QPoint lastMousePos;
    QPoint accumMousePos;

    Resultator res_w = Resultator(this);
    bool was_maximized;
    bool valid2 = false;
    bool view_2d_place = false, view_tier2 = false, view_torus = false, view_color_reality = false;
    int view_model_pages = -1;
    //bool pad[1];

    QVector3D pl_p2 = QVector3D(0, -5, 0);
    Obstacles obstacles;

    asdloader asd_loader = asdloader(":/models/all.asd");
    Celler celler = Celler(this);

    QOpenGLFramebufferObject *buffer = nullptr;
    QOpenGLFramebufferObject *buffer2 = nullptr;
    QOpenGLShaderProgram myShader;

    float fps_arr[fps_len] {0};
    int fps_pos = 0;
    uint current_uid = 0;
    renderable_p prev_model = nullptr, cur_model = nullptr, pressed_model[3] = {nullptr, nullptr, nullptr};

    float alt_pitch = -35.4f, alt_yaw = 39.3f, alt_roll = 0, alt_R = 2.5f, alt_Down = 0.2f;
    QList<Imager*> imagers;
public:
    MyWidget(QWidget *parent = nullptr);
    ~MyWidget();
    void closeEvent(QCloseEvent *event);

    bool modelsReady() { return models_ready; }
    renderable_p getModel(int id) { return id < 0 || id >= asdmodels ? asd_model[170] : asd_model[id]; }
    Imager* getImager(int id, int width = 64, int height = 64);
    void releaseImager(Imager *imager);

    int get_pet_id() { return celler.get_pet_id(); }
    void set_pet_id(int id) { celler.set_pet_id(id); }
    QString& get_pet_name() { return celler.get_pet_name(); }
    void set_pet_name(QString& name) { celler.set_pet_name(name); }

private:
    bool shaderState = false;
    void shaderUse(bool use);
    void mainRender(bool colorReality);
    void colorReality();
    void drawModelBox(QPainter &painter, QFontMetrics &metrics, int id, int X, int Y, int WH);

protected:
    void initializeGL();
    void resizeGL(int nWidth, int nHeight);
    void paintGL();

signals:
    void sender(QString);
};

#endif // MYWIDGET_H
