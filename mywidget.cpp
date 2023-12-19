#include "mywidget.h"
#include "ui_mywidget.h"
#include "objloader.h"

#include "math.h"

MyWidget::MyWidget(QWidget *parent): QOpenGLWidget(parent) {
    resize(800, 600);
    paintTimer = new QTimer(this);
    connect(paintTimer, SIGNAL(timeout()), this, SLOT(repaint()));
    paintTimer->start(5); // по идее должно быть 200 fps, но на практике 60

    connect(this, SIGNAL(sender(QString)), &res_w, SLOT(receiver(QString)));
    res_w.move((1960 - 800 - 2) / 2 - 421, (1080 - 600 - 30 - 2) / 2 - 35);
    //res_w.show();

    setWindowTitle("Мой игровой движок");

    time.start();
}

MyWidget::~MyWidget() {
    //qDebug() << "FREE CORE";
    for (int i = 0; i < asdmodels; i++)
        if (asd_model[i] != nullptr) {
            asd_model[i]->free_storage();
            delete asd_model[i];
        }
    if (buffer != nullptr) delete buffer;
    if (buffer2 != nullptr) delete buffer2;

    for (auto imager : imagers) delete imager;
}

void MyWidget::closeEvent(QCloseEvent *event) {
    if (res_w.isVisible() && !res_w.isHidden()) res_w.close();
    event->accept();
}





void Imager::glThread() {
    if (m == nullptr) {
        if (!eng->modelsReady()) return;
        m = eng->getModel(id);
    }

    res = m->toImage(w, h, 39.3f, -35.4f, 0, 1.5f, 0.2f);
    //qDebug("post img %ux%u", res.size().width(), res.size().height());
}
QImage Imager::noglThread(int width, int height) {
    w = width; h = height;
    return res;
};
Imager* MyWidget::getImager(int id, int width, int height) {
    Imager *res = new Imager(this, id, width, height);
    imagers.append(res);
    return res;
}
void MyWidget::releaseImager(Imager *imager) {
    //int prev = imagers.size();
    imagers.removeOne(imager);
    //qDebug("REMOVE ONE %u -> %u", prev, imagers.size());
}





void MyWidget::initTexture(uint index, QImage &texturel) {
    texturel = texturel.convertToFormat(QImage::Format_RGBA8888);
    glBindTexture(GL_TEXTURE_2D, texture[index]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GLsizei(texturel.width()), GLsizei(texturel.height()), 0, GL_RGBA, GL_UNSIGNED_BYTE, texturel.bits());
    qDebug() << "Texture_Idx:" << QString::number(index);
}

void MyWidget::loadGLTextures() {
    glGenTextures(5, texture);
    QImage texture;

    texture.load(":/images/brick.png");
    initTexture(0, texture);

    texture.load(":/images/planks_acacia.png");
    initTexture(1, texture);

    texture.load(":/images/stone.png");
    initTexture(2, texture);

    texture.load(":/images/grass_top.png");
    initTexture(3, texture);

    texture.load(":/images/Steampunk_Fly_albedo.jpg");
    initTexture(4, texture);
}

void MyWidget::initLight() {
    GLfloat light_ambient[] = {0, 0, 0, 0};
    GLfloat light_diffuse[] = {1, 1, 1, 1};
    GLfloat light_position[] = {0, 0, 2, 1};

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

GLuint MyWidget::drawQube(float repeat) {
    GLuint num = glGenLists(1);
    glNewList(num, GL_COMPILE);

    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1); // Фронтальная грань
    glTexCoord2f(0, 0); glVertex3f(-1, -1,  1);
    glTexCoord2f(1, 0); glVertex3f( 1, -1,  1);
    glTexCoord2f(1, 1); glVertex3f( 1,  1,  1);
    glTexCoord2f(0, 1); glVertex3f(-1,  1,  1);

    glNormal3f(0, 0, -1); // Тыльная грань
    glTexCoord2f(1, 0); glVertex3f(-1, -1, -1);
    glTexCoord2f(1, 1); glVertex3f(-1,  1, -1);
    glTexCoord2f(0, 1); glVertex3f( 1,  1, -1);
    glTexCoord2f(0, 0); glVertex3f( 1, -1, -1);

    glNormal3f(0, 1, 0); // Верхняя грань
    glTexCoord2f(0, repeat); glVertex3f(-1,  1, -1);
    glTexCoord2f(0, 0); glVertex3f(-1,  1,  1);
    glTexCoord2f(repeat, 0); glVertex3f( 1,  1,  1);
    glTexCoord2f(repeat, repeat); glVertex3f( 1,  1, -1);

    glNormal3f(0, -1, 0); // Нижняя грань
    glTexCoord2f(repeat, repeat); glVertex3f(-1, -1, -1);
    glTexCoord2f(0, repeat); glVertex3f( 1, -1, -1);
    glTexCoord2f(0, 0); glVertex3f( 1, -1,  1);
    glTexCoord2f(repeat, 0); glVertex3f(-1, -1,  1);

    glNormal3f(-1, 0, 0); // Левая грань
    glTexCoord2f(0, 0); glVertex3f(-1, -1, -1);
    glTexCoord2f(1, 0); glVertex3f(-1, -1,  1);
    glTexCoord2f(1, 1); glVertex3f(-1,  1,  1);
    glTexCoord2f(0, 1); glVertex3f(-1,  1, -1);

    glNormal3f(1, 0, 0); // Правая грань
    glTexCoord2f(1, 0); glVertex3f( 1, -1, -1);
    glTexCoord2f(1, 1); glVertex3f( 1,  1, -1);
    glTexCoord2f(0, 1); glVertex3f( 1,  1,  1);
    glTexCoord2f(0, 0); glVertex3f( 1, -1,  1);
    glEnd();

    glEndList();
    return num;
}



void MyWidget::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_X:
        if (paintTimer->isActive()) paintTimer->stop();
        else paintTimer->start();
        break;
    case Qt::Key_C:
        ++current_texture %= textures;
        break;
    case Qt::Key_V:
        ++current_model %= models;
        break;
    case Qt::Key_1:
        view_2d_place = not view_2d_place;
        //if (view_2d_place) view_tier2 = false;
        break;
    case Qt::Key_2:
        view_tier2 = not view_tier2;
        break;
    case Qt::Key_3:
        view_torus = not view_torus;
        break;
    case Qt::Key_4:
        view_color_reality = not view_color_reality;
        break;
    case Qt::Key_5:
        view_model_pages = (view_model_pages + 2) % ((asdmodels + 63) / 64 + 1) - 1;
        break;
    case Qt::Key_F11:
        if (isFullScreen()) was_maximized ? showMaximized() : showNormal();
        else { was_maximized = isMaximized(); showFullScreen(); }
        break;
    }
}

const int myKey_Control = 256 | (Qt::Key_Control & 255);
const int myKey_Shift   = 256 | (Qt::Key_Shift   & 255);
//const int myKey_F11     = 256 | (Qt::Key_F11     & 255);
const int myKey_Escape  = 256 | (Qt::Key_Escape  & 255);
const int myKey_Up      = 256 | (Qt::Key_Up      & 255);
const int myKey_Left    = 256 | (Qt::Key_Left    & 255);
const int myKey_Right   = 256 | (Qt::Key_Right   & 255);
const int myKey_Down    = 256 | (Qt::Key_Down    & 255);
const int myRus_W       = 512 | 23; // Ц
const int myRus_A       = 512 | 21; // Ф
const int myRus_S       = 512 | 28; // Ы
const int myRus_D       = 512 | 2;  // В

const int myLeftButton = 1;
//const int myRightButton = 2;
// const int myMiddleButton = 3;
// const int myBackButton = 4; // Хз что за кнопка ;'-}

bool MyWidget::eventFilter(QObject *sender, QEvent *event) {
    if (sender != this) return false;

    QEvent::Type type = event->type();
    //if (type != 12 && type != 77) qDebug("EVENT: %u", type);
    switch (type) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        // emit sender("Object: " + obj->objectName() + " " + (sender == this ? "true" : "false"));
        bool pressed = type == QEvent::KeyPress;
        QKeyEvent *ev = static_cast<QKeyEvent *>(event);
        int key = ev->key();
        //if (pressed && !ev->isAutoRepeat()) qDebug() << "key:" << QString::number(key);
        /*if (!ev->isAutoRepeat()) {
            char arr[34];
            for (int i = 0; i < 33; i++) arr[i] = isKeyPressed[512 | i] ? '1' : '0';
            arr[33] = 0;
            qDebug() << "Рашка:" << arr;
        }*/

        if (key >= 0 && key < 256) isKeyPressed[key] = pressed;
        if ((key & 0x7fffff00) == 0x01000000) {
            key ^= 0x01000000;
            if (key >= 0 && key < 256) isKeyPressed[256 | key] = pressed;
        }
        if (key == 1025) isKeyPressed[512 | 6] = pressed; // Ё
        if (key >= 1040 && key <= 1071) isKeyPressed[512 | (key - 1040 + (key >= 1046))] = pressed; // А-Я без Ё
        break; }
    case QEvent::MouseMove: {
        QMouseEvent *ev = static_cast<QMouseEvent *>(event);
        QPoint pos = ev->pos() - lastMousePos;
        float dx = pos.x();
        float dy = pos.y();
        accumMousePos += pos;

        if (isMousePressed[myLeftButton]) {

            static const float rotSpeed = -.1f;
            dx *= rotSpeed;
            dy *= rotSpeed;
            m_camera.rotate(dx, dy);
            if (view_model_pages >= 0) {
                alt_yaw += dx; alt_pitch += dy;
                if (alt_pitch < -90) alt_pitch = -90;
                if (alt_pitch >  90) alt_pitch =  90;
            }

            QCursor c = cursor();
            c.setPos(mapToGlobal(lastMousePos));
            setCursor(c);
        }
        lastMousePos = mapFromGlobal(cursor().pos());

        auto buttons = ev->buttons();
        for (int mbit = 0; mbit < 3; mbit++) {
            if (!(buttons & 1 << mbit)) continue;

            bool yeah = mbit >= 0 && mbit < 3 && pressed_model[mbit] != nullptr;

            if (yeah) {
                QPoint shift = pressMousePos + accumMousePos;

                pressed_model[mbit]->emit_event(ModelEvents::Move, shift.x(), shift.y(), mbit + 1);

                float L = sqrtf(powf(accumMousePos.x(), 2) + powf(accumMousePos.y(), 2));
                if (is_click && L > 8) is_click = false;
            }
        };

        break; }
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick: // тот же самый MouseButtonPress, только MouseButtonDblClick ворует у MouseButtonPress события ;'-}
    case QEvent::MouseButtonRelease: {
        bool pressed = type != QEvent::MouseButtonRelease;
        QMouseEvent *ev = static_cast<QMouseEvent*>(event);
        int baton = int(ev->button()), bit = 0;
        while (baton) {
            baton >>= 1;
            bit++;
        }
        isMousePressed[bit] = pressed;
        //qDebug("Bit: %u", bit);

        if (bit == myLeftButton) {
            QCursor c = cursor();
            if (pressed) {
                lastMousePos = mapFromGlobal(c.pos());
                c.setShape(Qt::BlankCursor);
            } else c.setShape(Qt::ArrowCursor);
            setCursor(c);
        }
        lastMousePos = mapFromGlobal(cursor().pos());

        if (pressed) {
            pressMousePos = lastMousePos;
            accumMousePos = QPoint(0, 0);
            if (cur_model != nullptr) cur_model->emit_event(ModelEvents::Press, pressMousePos.x(), pressMousePos.y(), bit);
            if (bit > 0 && bit <= 3) pressed_model[bit - 1] = cur_model;
            is_click = true;
        } else {
            bool yeah = bit > 0 && bit <= 3 && pressed_model[bit - 1] != nullptr;
            if (yeah) {
                int mbit = bit - 1;
                QPoint shift = pressMousePos + accumMousePos;
                int m_x = shift.x(), m_y = shift.y();

                pressed_model[mbit]->emit_event(ModelEvents::Release, m_x, m_y, bit);

                float L = sqrtf(powf(accumMousePos.x(), 2) + powf(accumMousePos.y(), 2));
                if (is_click && L <= 8) pressed_model[mbit]->emit_event(ModelEvents::Click, m_x, m_y, bit);

                pressed_model[mbit] = nullptr;
            }
        }
        break; }
    case QEvent::Wheel: {
        QWheelEvent *ev = static_cast<QWheelEvent *>(event);
        if (view_model_pages >= 0) {
            float delta = ev->delta() < 0 ? 1.05f : 1 / 1.05f;
            alt_R *= delta;
        }
        break; }
    default: {}
    }

    return false;
}

void MyWidget::eventHandler() {
    QVector3D translation;
    if (isKeyPressed[Qt::Key_Space]) translation += m_camera.up();
    if (isKeyPressed[myKey_Control]) translation -= m_camera.up();

    bool Up    = isKeyPressed[Qt::Key_W] | isKeyPressed[myKey_Up]    | isKeyPressed[myRus_W];
    bool Left  = isKeyPressed[Qt::Key_A] | isKeyPressed[myKey_Left]  | isKeyPressed[myRus_A];
    bool Right = isKeyPressed[Qt::Key_D] | isKeyPressed[myKey_Right] | isKeyPressed[myRus_D];
    bool Down  = isKeyPressed[Qt::Key_S] | isKeyPressed[myKey_Down]  | isKeyPressed[myRus_S];
    if (Up   ) translation += m_camera.forward();
    if (Left ) translation -= m_camera.right();
    if (Right) translation += m_camera.right();
    if (Down ) translation -= m_camera.forward();

    translation.normalize();

    float transSpeed = 0.05f;
    if (isKeyPressed[myKey_Shift]) transSpeed *= 3;
    m_camera.translate(transSpeed *translation);

    if (isKeyPressed[myKey_Escape]) close(); //emit exit(0);

    if (view_model_pages >= 0) {
        if (Up   ) alt_Down += 0.01f;
        if (Left ) alt_roll -= 1;
        if (Right) alt_roll += 1;
        if (Down ) alt_Down -= 0.01f;
        if (alt_roll < -90) alt_roll = -90;
        if (alt_roll >  90) alt_roll =  90;
    }
}





#define TWO_MODELS_AP(n, m1, m2) asd_model[n] = new model_group(2, new renderable_p[2] { asd_loader.get_model(m1, ap), asd_loader.get_model(m2, ap) })
#define TWO_MODELS(n, m1, m2) asd_model[n] = new model_group(2, new renderable_p[2] { asd_loader.get_model(m1), asd_loader.get_model(m2) })
#define THREE_MODELS_AP(n, m1, m2, m3) asd_model[n] = new model_group(3, new renderable_p[3] { asd_loader.get_model(m1, ap), asd_loader.get_model(m2, ap), asd_loader.get_model(m3, ap) })
#define FOUR_MODELS(n, m1, m2, m3, m4) asd_model[n] = new model_group(4, new renderable_p[4] { asd_loader.get_model(m1), asd_loader.get_model(m2), asd_loader.get_model(m3), asd_loader.get_model(m4) })
#define MANY_NODELS(n, str) do { \
    QStringList models = QString(str).split("|"); \
    uint L = uint(models.count()); \
    renderable_p *arr = new renderable_p[L]; \
    for (uint i = 0; i < L; i++) arr[i] = asd_loader.get_model(models[int(i)]); \
    asd_model[n] = new model_group(L, arr); \
} while(false);

void MyWidget::initializeGL() {
    //glEnable(GL_MULTISAMPLE); // сглаживание MSAA включено
    loadGLTextures();
    glEnable(GL_TEXTURE_2D);
    glClearColor(.8f, .9f, 1, 1);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    // initLight();
    model[0] = objloader::Instance().load(":/models/monkey2.obj");
    model[1] = objloader::Instance().load(":/models/sidor.obj");
    model[2] = drawQube();
    model[3] = objloader::Instance().load(":/models/Steampunk_Fly_low_poly.obj");
    model[4] = drawQube(16);
    torus = objloader::Instance().load(":/models/torus.obj");

    for (int i = 0; i < 512 + 33; i++) isKeyPressed[i] = false;
    for (int i = 0; i < 32; i++) isMousePressed[i] = false;

    model[5] = model[0];
    //asd_model[0] = asd_loader.get_model("f12");
    //asd_model[0] = asd_loader.get_model("(1)Spider");
    /*renderable_p arr[] = {
        asd_loader.get_model("c09a"),
        asd_loader.get_model("c09b")
    };*/
    renderable_p model_a06 = asd_loader.get_model("a06");
    awaiter = model_a06->get_awaiter();
    edger_awaiter *ap = &awaiter;
    asd_model[0] = asd_loader.get_model("a01", ap); // бревно, повёрнутое на Y
    asd_model[1] = asd_loader.get_model("a02", ap); // бревно 1/4, повёрнутое на Y
    asd_model[2] = asd_loader.get_model("a03", ap); // 4 бревна, повёрнутые на Z
    asd_model[3] = asd_loader.get_model("a04", ap); // полублок - 2 бревна, повёрнутое на Z
    asd_model[4] = asd_loader.get_model("a05", ap); // трава
    asd_model[5] = model_a06;                       // трава, но чуть светлее
    asd_model[6] = asd_loader.get_model("a07", ap); // железная жила №1
    asd_model[7] = asd_loader.get_model("a08", ap); // железная жила №2
    asd_model[8] = asd_loader.get_model("a13", ap); // железный блок
    asd_model[9] = asd_loader.get_model("a14", ap); // железные слитки
    asd_model[10] = asd_loader.get_model("a19", ap); // железный полублок-куча
    asd_model[11] = asd_loader.get_model("a20", ap); // железный слиток, повёрнутый на Z
    asd_model[12] = asd_loader.get_model("a09", ap); // золотая жила №1
    asd_model[13] = asd_loader.get_model("a10", ap); // золотая жила №2
    asd_model[14] = asd_loader.get_model("a15", ap); // золотой блок
    asd_model[15] = asd_loader.get_model("a16", ap); // золотые слитки
    asd_model[16] = asd_loader.get_model("a21", ap); // золотой полублок-куча
    asd_model[17] = asd_loader.get_model("a22", ap); // золотой слиток, повёрнутый на Z
    asd_model[18] = asd_loader.get_model("a11", ap); // угольная жила №1
    asd_model[19] = asd_loader.get_model("a12", ap); // угольная жила №2
    asd_model[20] = asd_loader.get_model("a17", ap); // угольный блок
    asd_model[21] = asd_loader.get_model("a18", ap); // угольная куча
    asd_model[22] = asd_loader.get_model("a23", ap); // угольный полублок-куча
    asd_model[23] = asd_loader.get_model("a24", ap); // кристаллы рубина в блоке
    asd_model[24] = asd_loader.get_model("a25", ap); // рубиновый блок
    asd_model[25] = asd_loader.get_model("a28", ap); // кристаллы рубина в полублоке
    asd_model[26] = asd_loader.get_model("a29", ap); // кристалл рубина
    asd_model[27] = asd_loader.get_model("a26", ap); // кристаллы алмаза в блоке
    asd_model[28] = asd_loader.get_model("a27", ap); // алмазный блок
    asd_model[29] = asd_loader.get_model("a30", ap); // кристаллы алмаза в полублоке
    asd_model[30] = asd_loader.get_model("a31", ap); // кристалл алмаза

    asd_model[31] = asd_loader.get_model("b01", ap);
    asd_model[32] = asd_loader.get_model("b02", ap);
    asd_model[33] = asd_loader.get_model("b03", ap);
    asd_model[34] = asd_loader.get_model("b04", ap);
    asd_model[35] = asd_loader.get_model("b05", ap);
    asd_model[36] = asd_loader.get_model("b06", ap);
    asd_model[37] = asd_loader.get_model("b07", ap);
    asd_model[38] = asd_loader.get_model("b08", ap);
    asd_model[39] = asd_loader.get_model("b09", ap);
    asd_model[40] = asd_loader.get_model("b10", ap);
    asd_model[41] = asd_loader.get_model("b11", ap);
    asd_model[42] = asd_loader.get_model("b12", ap);
    TWO_MODELS_AP(43, "b13a", "b13b");
    TWO_MODELS_AP(44, "b14a", "b14b");
    TWO_MODELS_AP(45, "b15a", "b15b");
    TWO_MODELS_AP(46, "b16a", "b16b");
    TWO_MODELS_AP(47, "b17a", "b17b");
    TWO_MODELS_AP(48, "b18a", "b18b");
    TWO_MODELS_AP(49, "b19a", "b19b");
    TWO_MODELS_AP(50, "b20a", "b20b");
    asd_model[51] = asd_loader.get_model("b21", ap);
    asd_model[52] = asd_loader.get_model("b22", ap);
    asd_model[53] = asd_loader.get_model("b23", ap);
    asd_model[54] = asd_loader.get_model("b24", ap);
    asd_model[55] = asd_loader.get_model("b25", ap);
    asd_model[56] = asd_loader.get_model("b26", ap);
    asd_model[57] = asd_loader.get_model("b27", ap);
    asd_model[58] = asd_loader.get_model("b28", ap);

    asd_model[59] = asd_loader.get_model("c01", ap);
    asd_model[60] = asd_loader.get_model("c02", ap);
    asd_model[61] = asd_loader.get_model("c03", ap);
    asd_model[62] = asd_loader.get_model("c04", ap);
    TWO_MODELS_AP(63, "c09a", "c09b");
    TWO_MODELS_AP(64, "c10a", "c10b");
    asd_model[65] = asd_loader.get_model("c11", ap);
    asd_model[66] = asd_loader.get_model("c12", ap);
    asd_model[67] = asd_loader.get_model("c13", ap);
    asd_model[68] = asd_loader.get_model("c14", ap);
    asd_model[69] = asd_loader.get_model("c15", ap);
    asd_model[70] = asd_loader.get_model("c16", ap);
    asd_model[71] = asd_loader.get_model("c17", ap);
    asd_model[72] = asd_loader.get_model("c18", ap);
    asd_model[73] = asd_loader.get_model("c19", ap);
    asd_model[74] = asd_loader.get_model("c20", ap);
    asd_model[75] = asd_loader.get_model("c21", ap);
    asd_model[76] = asd_loader.get_model("c22", ap);
    asd_model[77] = asd_loader.get_model("c23", ap);
    asd_model[78] = asd_loader.get_model("c24", ap);
    asd_model[79] = asd_loader.get_model("c25", ap);
    asd_model[80] = asd_loader.get_model("c26", ap);
    asd_model[81] = asd_loader.get_model("c27", ap);
    asd_model[82] = asd_loader.get_model("c28", ap);
    asd_model[83] = asd_loader.get_model("c29", ap);
    asd_model[84] = asd_loader.get_model("c30", ap);

    TWO_MODELS(85, "AfricanElephant_M", "AfricanElephant_M_APFur");                         // Африканский слон
    TWO_MODELS(86, "AmericanBison", "AmericanBison_APFur");                                 // Американский бизон
    TWO_MODELS(87, "BaldEagle_Main", "BaldEagle_Feathers");                                 // Белоголовый орёл
    asd_model[88] = asd_loader.get_model("BighornSheep_F");                                 // Снежная бараниха
    asd_model[89] = asd_loader.get_model("BighornSheep_M");                                 // Снежный баран

    asd_model[90] = asd_loader.get_model("Bluebird");                                       // Синяя птица
    TWO_MODELS(91, "Chicken_M_Main", "Chicken_Feathers_M");                                 // Петух
    asd_model[92] = asd_loader.get_model("Cougar");                                         // Пума
    TWO_MODELS(93, "Cow_F", "Cow_F_APFur");                                                 // Корова
    asd_model[94] = asd_loader.get_model("Dachshund");                                      // Такса

    TWO_MODELS(95, "Cat_Main", "Cat_Main.001");                                             // Кисюлька (котяра) (киса) (кот) (котище) (котлета) (котострофа)
    TWO_MODELS(96, "Elk_M", "Elk_M_Antlers");                                               // Жирный (большой) олень
    asd_model[97] = asd_loader.get_model("GermanShepherd");                                 // Немецкая овчарка
    FOUR_MODELS(98, "Gorilla_M_Main", "Gorilla_M_Main.001", "Gorilla_M_Main.002", "Gorilla_M_Main.003"); // Самец гориллы
    asd_model[99] = asd_loader.get_model("GrayWolf");                                       // Серенький волчок

    asd_model[100] = asd_loader.get_model("GrayWolf_Y");                                    // Детёныш серенького волчка
    TWO_MODELS(101, "GreenDarnerDragonfly_Main", "GreenDarnerDragonfly_Wings");             // Зелёная стрекоза-штопальщица
    TWO_MODELS(102, "Moose_M", "Moose_M.001");                                              // Лось
    TWO_MODELS(103, "MuleDeer_M", "MuleDeer_Antlers");                                      // Олень-мул
    asd_model[104] = asd_loader.get_model("Pangolin_Main");                                 // Утилизатор муравьёв (муравьед)

    asd_model[105] = asd_loader.get_model("Pig_F_Main");                                    // Хрякосамка (свинья)
    asd_model[106] = asd_loader.get_model("RedDeer_M");                                     // Благородный (красный) олень
    asd_model[107] = asd_loader.get_model("Spider");                                        // Пау-пау-паучёк ;'-}
    TWO_MODELS(108, "Tiger_M", "Tiger_M_APFur");                                            // Тигр
    TWO_MODELS(109, "WildBoar_M", "WildBoar_M_APFur");                                      // Дикий кабан

    asd_model[110] = asd_loader.get_model("o01", ap); // ПШЕНИЦА: единица;
    asd_model[111] = asd_loader.get_model("o02", ap); // росток №1;
    asd_model[112] = asd_loader.get_model("o03", ap); // росток №2;
    asd_model[113] = asd_loader.get_model("o04", ap); // росток №3;
    asd_model[114] = asd_loader.get_model("o05", ap); // мешок урожая
    asd_model[115] = asd_loader.get_model("o06", ap); // и табличка.

    asd_model[116] = asd_loader.get_model("o07", ap); // КУКУРУЗА: единица;
    asd_model[117] = asd_loader.get_model("o08", ap); // росток №1;
    asd_model[118] = asd_loader.get_model("o09", ap); // росток №2;
    asd_model[119] = asd_loader.get_model("o10", ap); // росток №3;
    asd_model[120] = asd_loader.get_model("o11", ap); // мешок урожая
    asd_model[121] = asd_loader.get_model("o12", ap); // и табличка.

    asd_model[122] = asd_loader.get_model("o13", ap); // КАРТОШКА: единица;
    asd_model[123] = asd_loader.get_model("o14", ap); // росток №1;
    asd_model[124] = asd_loader.get_model("o15", ap); // росток №2;
    asd_model[125] = asd_loader.get_model("o16", ap); // росток №3;
    asd_model[126] = asd_loader.get_model("o17", ap); // мешок урожая
    asd_model[127] = asd_loader.get_model("o18", ap); // и табличка.

    asd_model[128] = asd_loader.get_model("o19", ap); // КАПУСТА: единица;
    asd_model[129] = asd_loader.get_model("o20", ap); // росток №1;
    asd_model[130] = asd_loader.get_model("o21", ap); // росток №2;
    asd_model[131] = asd_loader.get_model("o22", ap); // росток №3;
    asd_model[132] = asd_loader.get_model("o23", ap); // мешок урожая
    asd_model[133] = asd_loader.get_model("o24", ap); // и табличка.

    asd_model[134] = asd_loader.get_model("o25", ap); // ПОМИДОРКА: единица;
    asd_model[135] = asd_loader.get_model("o26", ap); // росток №1;
    asd_model[136] = asd_loader.get_model("o27", ap); // росток №2;
    asd_model[137] = asd_loader.get_model("o28", ap); // росток №3;
    asd_model[138] = asd_loader.get_model("o29", ap); // мешок урожая
    asd_model[139] = asd_loader.get_model("o30", ap); // и табличка.

    asd_model[140] = asd_loader.get_model("o31", ap); // МОРКОВКА: единица;
    asd_model[141] = asd_loader.get_model("o32", ap); // росток №1;
    asd_model[142] = asd_loader.get_model("o33", ap); // росток №2;
    asd_model[143] = asd_loader.get_model("o34", ap); // росток №3;
    asd_model[144] = asd_loader.get_model("o35", ap); // мешок урожая
    asd_model[145] = asd_loader.get_model("o36", ap); // и табличка.

    asd_model[146] = asd_loader.get_model("o37", ap); // Вымощенная камнями дорога №1
    asd_model[147] = asd_loader.get_model("o38", ap); // Вымощенная камнями дорога №2
    asd_model[148] = asd_loader.get_model("o39", ap); // Вымощенная камнями дорога №3
    asd_model[149] = asd_loader.get_model("o40", ap); // Вымощенная камнями дорога №4
    asd_model[150] = asd_loader.get_model("o41", ap); // Пустой мешок урожая
    asd_model[151] = asd_loader.get_model("o42", ap); // Пустая табличка

    asd_model[152] = asd_loader.get_model("o43", ap); // Двойные горизонтальные две палки забора
    asd_model[153] = asd_loader.get_model("o54", ap); // Одинарные горизонтальные две палки забора
    asd_model[154] = asd_loader.get_model("o53", ap); // Одинарные горизонтальные две палки забора (чуть кривее)
    asd_model[155] = asd_loader.get_model("o44", ap); // Вертикальная палка забора
    asd_model[156] = asd_loader.get_model("o45", ap); // Вертикальная палка забора (чуть кривее)
    asd_model[157] = asd_loader.get_model("o46", ap); // Грядка (место посадки) №1
    asd_model[158] = asd_loader.get_model("o47", ap); // Грядка (место посадки) №2
    asd_model[159] = asd_loader.get_model("o48", ap); // Два камыша
    asd_model[160] = asd_loader.get_model("o49", ap); // Трава
    asd_model[161] = asd_loader.get_model("o50", ap); // Подорожник
    asd_model[162] = asd_loader.get_model("o51", ap); // Жёлтый и белый одуванчики
    asd_model[163] = asd_loader.get_model("o52", ap); // Два белых одуванчика
    THREE_MODELS_AP(164, "o55a", "o55b", "o55c"); // Калитка

    TWO_MODELS(165, "lvl2_toilet", "lvl2_toilet_lid");
    MANY_NODELS(166, "lvl2_toilet|lvl2_toilet_lid|lvl2_head|lvl2_mouth|lvl2_eyeball_L|lvl2_eyeball_R|lvl2_barrel_1|lvl2_barrel_2|lvl2_barrel_3|lvl2_barrel_4|lvl2_barrel_5|lvl2_barrel_6")
    MANY_NODELS(167, "lvl3_toilet|lvl3_siren|lvl3_head|lvl3_mouth|lvl3_eyeballs|lvl3_cap")
    MANY_NODELS(168, "lvl4_toilet|lvl4_combine_interface|lvl4_combine_interface_disp|lvl4_combine_barricade_tall|lvl4_stasisfield_sheet|lvl4_eric_facemap|lvl4_mouth|lvl4_eyeball_L|lvl4_eyeball_R");
    MANY_NODELS(169, "lvl5_toilet|lvl5_seat|lvl5_pipes|lvl5_columns|lvl5_ring|lvl5_bottom|lvl5_gun|lvl5_head|lvl5_mouth|lvl5_eyeball_L|lvl5_eyeball_R|lvl5_cap");

    asd_model[170] = asd_loader.get_model("UNKNOWN", ap);

    asd_model[171] = asd_loader.get_model("l01", ap);
    asd_model[172] = asd_loader.get_model("l02", ap);
    asd_model[173] = asd_loader.get_model("l03", ap);
    asd_model[174] = asd_loader.get_model("l04", ap);
    asd_model[175] = asd_loader.get_model("l05", ap);
    asd_model[176] = asd_loader.get_model("l06", ap);
    asd_model[177] = asd_loader.get_model("l07", ap, false);
    asd_model[178] = asd_loader.get_model("l08", ap, false);
    asd_model[179] = asd_loader.get_model("l09", ap, false);
    asd_model[180] = asd_loader.get_model("l10", ap, false);
    asd_model[181] = asd_loader.get_model("l11", ap);
    asd_model[182] = asd_loader.get_model("l12", ap);
    asd_model[183] = asd_loader.get_model("l13", ap);
    asd_model[184] = asd_loader.get_model("l14", ap);
    asd_model[185] = asd_loader.get_model("l15", ap);
    asd_model[186] = asd_loader.get_model("l16", ap, false);
    asd_model[187] = asd_loader.get_model("l17", ap, false);
    asd_model[188] = asd_loader.get_model("l18", ap, false);
    asd_model[189] = asd_loader.get_model("l19", ap, false);
    asd_model[190] = asd_loader.get_model("l20", ap, false); // Кусок мяса (индикатор голода)
    asd_model[191] = asd_loader.get_model("l21", ap);
    asd_model[192] = asd_loader.get_model("l22", ap, false);
    asd_model[193] = asd_loader.get_model("l23", ap, false);
    asd_model[194] = asd_loader.get_model("l24", ap, false);
    asd_model[195] = asd_loader.get_model("l25", ap, false);
    asd_model[196] = asd_loader.get_model("l26", ap);
    asd_model[197] = asd_loader.get_model("l27", ap);
    asd_model[198] = asd_loader.get_model("l28", ap);
    asd_model[199] = asd_loader.get_model("l29", ap);
    asd_model[200] = asd_loader.get_model("l30", ap);

    asd_model[201] = asd_loader.get_model("r01", ap);
    asd_model[202] = asd_loader.get_model("r02", ap);
    asd_model[203] = asd_loader.get_model("r03", ap);
    asd_model[204] = asd_loader.get_model("r04", ap);
    asd_model[205] = asd_loader.get_model("r05", ap);
    asd_model[206] = asd_loader.get_model("r06", ap);
    asd_model[207] = asd_loader.get_model("r07", ap);
    asd_model[208] = asd_loader.get_model("r08", ap);
    asd_model[209] = asd_loader.get_model("r09", ap);
    asd_model[210] = asd_loader.get_model("r10", ap);
    asd_model[211] = asd_loader.get_model("r11", ap);
    asd_model[212] = asd_loader.get_model("r12", ap);
    asd_model[213] = asd_loader.get_model("r13", ap);
    asd_model[214] = asd_loader.get_model("r14", ap);
    asd_model[215] = asd_loader.get_model("r15", ap);
    asd_model[216] = asd_loader.get_model("r16", ap);
    asd_model[217] = asd_loader.get_model("r17", ap);
    asd_model[218] = asd_loader.get_model("r18", ap);
    asd_model[219] = asd_loader.get_model("r19", ap);
    asd_model[220] = asd_loader.get_model("r20", ap);
    asd_model[221] = asd_loader.get_model("r21", ap);
    asd_model[222] = asd_loader.get_model("r22", ap);
    asd_model[223] = asd_loader.get_model("r23", ap);
    asd_model[224] = asd_loader.get_model("r24", ap);
    asd_model[225] = asd_loader.get_model("r25", ap);
    asd_model[226] = asd_loader.get_model("r26", ap);
    asd_model[227] = asd_loader.get_model("r27", ap); // Кровать (индикатор усталости)
    asd_model[228] = asd_loader.get_model("r28", ap);
    asd_model[229] = asd_loader.get_model("r29", ap);
    THREE_MODELS_AP(230, "r30a", "r30b", "r30c");
    asd_model[231] = asd_loader.get_model("r31", ap);
    asd_model[232] = asd_loader.get_model("r32", ap);
    asd_model[233] = asd_loader.get_model("r33", ap);
    asd_model[234] = asd_loader.get_model("r34", ap);
    asd_model[235] = asd_loader.get_model("r35", ap);
    TWO_MODELS_AP(236, "r36a", "r36b");
    TWO_MODELS_AP(237, "r37a", "r37b");
    TWO_MODELS_AP(238, "r38a", "r38b");
    asd_model[239] = asd_loader.get_model("r39", ap);
    TWO_MODELS_AP(240, "r40a", "r40b");
    TWO_MODELS_AP(241, "r41a", "r41b");
    TWO_MODELS_AP(242, "r42a", "r42b");
    asd_model[243] = asd_loader.get_model("r43", ap);

    asd_model[244] = asd_loader.get_model("g01", ap, false);
    asd_model[245] = asd_loader.get_model("g02", ap, false);
    asd_model[246] = asd_loader.get_model("g03", ap, false);
    asd_model[247] = asd_loader.get_model("g04", ap, false);
    asd_model[248] = asd_loader.get_model("g05", ap, false);
    asd_model[249] = asd_loader.get_model("g06", ap, false);
    asd_model[250] = asd_loader.get_model("g07", ap, false); // Монетка (индикатор богатства)
    asd_model[251] = asd_loader.get_model("g08", ap);
    asd_model[252] = asd_loader.get_model("g09", ap);
    asd_model[253] = asd_loader.get_model("g10", ap);
    asd_model[254] = asd_loader.get_model("g11", ap);
    asd_model[255] = asd_loader.get_model("g12", ap);
    asd_model[256] = asd_loader.get_model("g13", ap);



    models_ready = true;

    /*for (uint i = 0; i < 100; i++) {
        UID uid = asd_loader.get_uid(i);
        UID uid2 = asd_loader.get_uid(uid.r, uid.g, uid.b);
        qDebug("color: %3u | %02x%02x%02x | %3u", uid.id, uid.r, uid.g, uid.b, uid2.id);
    }*/

    celler.init_plants(asd_model);

    buffer = new QOpenGLFramebufferObject(width(), height());
    buffer2 = new QOpenGLFramebufferObject(256, 256, QOpenGLFramebufferObject::Depth);

    bool R = myShader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/test.vsh");
    if (R) R = myShader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/test.fsh");
    if (R) R = myShader.link();
    if (!R) {
        qDebug("ShaderError");
        //close(); всё равно ничего не закрывает, зато Esc не работает после этого ;'-}
    }
}

void MyWidget::resizeGL(int nWidth, int nHeight) {
    glViewport(0, 0, nWidth, nHeight);
    float aspectration = float(nWidth) / nHeight;

    glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    QMatrix4x4 mat = QMatrix4x4();
    mat.perspective(FOV, aspectration, 0.001f, 1000000);
    glLoadMatrixf(mat.constData());
    //gluPerspective(FOV, aspectration, 0.001, 1000000);

    GLfloat proj_m[16];
    glGetFloatv(GL_PROJECTION_MATRIX, proj_m);
    proj_mat = QMatrix4x4(static_cast<float*>(proj_m)).transposed(); // ТАК ОНО ТРАНСПОНИРОВАНО!!!!! ;'-}

    //GLfloat *m = mat.data();
    //qDebug("Mat: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);
    //m = proj_mat.data();
    //qDebug("Mat2: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);

    glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();

    screenSize = QPoint(nWidth, nHeight);

    if (buffer != nullptr) delete buffer;
    buffer = new QOpenGLFramebufferObject(nWidth, nHeight, QOpenGLFramebufferObject::Depth);
}





void MyWidget::shaderUse(bool use) {
    if (use) {
        if (!shaderState) {
            myShader.bind();

            myShader.setUniformValue("ProjectionMatrix", proj_mat);
            myShader.setUniformValue("ViewMatrix", view_mat);

            GLfloat viewport[4];
            glGetFloatv(GL_VIEWPORT, viewport);
            myShader.setUniformValue("viewport", viewport[0], viewport[1], viewport[2], viewport[3]);

            shaderState = true;
        }

        GLfloat model_m[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, model_m);
        auto viewmodel_mat = QMatrix4x4(static_cast<float*>(model_m)).transposed();
        auto model_mat = view_mat.inverted() * viewmodel_mat;
        //GLfloat *m = (modelview_mat * view_mat.inverted()).data();
        //qDebug("Mat: %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9], m[10], m[11], m[12], m[13], m[14], m[15]);
        myShader.setUniformValue("ModelMatrix", model_mat);
        myShader.setUniformValue("sampler0", 0);
    } else if (shaderState) {
        myShader.release();
        shaderState = false;
    }
}

void MyWidget::mainRender(bool colorReality) {
    if (view_torus) {
        //glBindTexture(GL_TEXTURE_2D, asd_loader.get_texture(3));
        glBindTexture(GL_TEXTURE_2D, texture[4]);
        glDeleteLists(torus, 1);
        torus = objloader::Instance().draw(current_time / 5); // цикл = 5 секундам
        glPushMatrix();
            glTranslatef(0, 0, -5);
            glScalef(2, 2, 2);
            glCallList(torus);
        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, texture[current_texture]);
        glPushMatrix();
            glTranslatef(0, 0, 0.2f - 5);
            glRotatef(current_time * 20, 0, 1, 0); // 20 градусов в секунду
            glScalef(2, 2, 2);
            if (current_model == 0) asd_model[85]->render();
            else glCallList(model[current_model]);
        glPopMatrix();
    }



    renderable_p pet = asd_model[85 + celler.get_pet_id()];
    bool use_shader = !colorReality && !view_color_reality;

    if (pet->ready()) {
        auto map = celler.fart_check();
        for (auto pair : *map) {
            long8 pos = pair.first;
            int zx, zy;
            auto data = pair.second;
            POS_TO_X_Y(zx, zy, pos);

            glPushMatrix();
                glTranslatef(zx * 2, -7, zy * 2);
                glScalef(.333f, .333f, .333f);

                auto type = data.model;
                shaderUse(use_shader && type->get_uid().id == current_uid);

                type->render();
            glPopMatrix();
        }
    }
    shaderUse(false);

    uint size;
    long8* plate = celler.get_plates(size);
    for (uint i = 0; i < size; i++) {
        long8 pos = plate[i];
        float lock = float(celler.is_locked_plate(pos)) * 1.5f;
        int zx, zy;
        POS_TO_X_Y(zx, zy, pos);
        glPushMatrix();
            glTranslatef(zx * 2, -10 + lock, zy * 2);
            asd_model[8]->render();
        glPopMatrix();
    }
    long8* border = celler.get_borders(size);
    for (uint i = 0; i < size; i++) {
        long8 pos = border[i];
        int zx, zy;
        POS_TO_X_Y(zx, zy, pos);
        glPushMatrix();
            glTranslatef(zx * 2, -10, zy * 2);
            asd_model[14]->render();
            glTranslatef(0, 2, 0);
            asd_model[28]->render();
        glPopMatrix();
    }

    auto plants = celler.get_plants(size);
    for (uint i = 0; i < size; i++) {
        auto plant = plants[i];
        int zx = plant->x, zy = plant->y;

        glPushMatrix();
            glTranslatef(zx * 2, -9, zy * 2);

            auto type = plant->type;
            shaderUse(use_shader && type->get_uid().id == current_uid);

            type->set_option(plant->state());
            type->render();
        glPopMatrix();
    }
    auto beds = celler.get_beds();
    for (auto pair : beds) {
        long8 pos = pair.first;
        int zx, zy;
        POS_TO_X_Y(zx, zy, pos);

        glPushMatrix();
            glTranslatef(zx * 2, -8, zy * 2);

            renderable_p model = pair.second;
            shaderUse(use_shader && model->get_uid().id == current_uid);

            model->render();
        glPopMatrix();
    }
    shaderUse(false);

    float PetX, PetY, PetZ, angle;
    celler.get_pet_pos(pet->ready() ? time_delta : 0, PetX, PetY, PetZ, angle);
    glPushMatrix();
        glTranslatef(PetX * 2, PetY * 2 -8, PetZ * 2);
        glRotatef(angle, 0, 1, 0);
        pet->render();
    glPopMatrix();

    /*for (uint i = 0; i < 25; i++) {
        int x = i / 5, y = i % 5;
        glPushMatrix();
            glTranslatef(20 + x * 2.5f, 0, y * 2.5f);
            asd_model[i + 85]->render();
        glPopMatrix();
    }*/

    /*for (uint i = 0; i < 5; i++) {
        glPushMatrix();
            glTranslatef(i * 2.5f, 0, 0);
            asd_model[i + 165]->render();
        glPopMatrix();
    }*/
}

void MyWidget::colorReality() {
    buffer->bind();

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(.8f, .9f, 1, 1);
    //glEnable(GL_DEPTH_TEST);

    asd_loader.set_render_mode(true);
    float td = time_delta;
    time_delta = 0;
    mainRender(true);
    time_delta = td;

    int m_x = lastMousePos.x(), m_y = lastMousePos.y();
    int SX = screenSize.x(), SY = screenSize.y();
    if (m_x >= 0 || m_y >= 0 || m_x < SX || m_y < SY) {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        byte pixel[4];
        glReadPixels(m_x, viewport[3] - m_y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
        UID uid = asd_loader.get_uid(pixel[0], pixel[1], pixel[2]);
        current_uid = uid.id;



        cur_model = asd_loader.uid2model(uid);

        bool changed = cur_model != prev_model;
        if (changed && cur_model != nullptr) cur_model->emit_event(ModelEvents::Hover, m_x, m_y);
        if (cur_model != nullptr) cur_model->emit_event(ModelEvents::Hovered, m_x, m_y);
        if (changed && prev_model != nullptr) prev_model->emit_event(ModelEvents::Unhover, m_x, m_y);

        prev_model = cur_model;
    }

    buffer->release();
}

void MyWidget::drawModelBox(QPainter &painter, QFontMetrics &metrics, int id, int X, int Y, int WH) {
    painter.beginNativePainting();
    //glPushAttrib(GL_ALL_ATTRIB_BITS);

    bool is_model = id >= 0 && id < asdmodels;
    QImage res;
    QString text;
    if (is_model) {
        renderable_p cur_model = asd_model[id];
        glEnable(GL_DEPTH_TEST);
        res = cur_model->toImage(WH, WH, alt_yaw, alt_pitch, alt_roll, alt_R, alt_Down);
        glDisable(GL_DEPTH_TEST);
        text = cur_model->name();
    } else text = "...";

    //glPopAttrib();
    painter.endNativePainting();

    QRect zone(X, Y, WH, WH);
    if (is_model) painter.drawImage(zone, res);
    else painter.fillRect(zone, QColor(0, 128, 64, 128));

    int textSX = metrics.width(text) + 1, textSY = metrics.height();
    if (textSX > WH) textSX = WH;
    if (textSX < 6) textSX = 6;
    painter.drawText(QRect(X + WH - textSX, Y + WH - textSY, textSX, textSY), Qt::TextSingleLine, text);
}

void MyWidget::paintGL() {
    current_time = time.elapsed() / 1000.f;
    time_delta = current_time - prev_time;
    fps_arr[fps_pos++] = 1 / time_delta;
    fps_pos %= fps_len;
    float fps_sum = 0;
    for (int i = 0; i < fps_len; i++) fps_sum += fps_arr[i];
    fps_sum /= fps_len;
    //qDebug("FPS: %f -> %f", 1 / time_delta, fps_sum);

    if (asd_loader.cb != 0) asd_loader.main_th();
    for (auto imager : imagers) imager->glThread();

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glLoadIdentity();

    view_mat = m_camera.toMatrix();
    glLoadMatrixf(view_mat.constData());
    glScalef(.5f, .5f, .5f);

    //if (isMousePressed[myRightButton])
    colorReality();

    asd_loader.set_render_mode(view_color_reality);
    glColor3f(1, 1, 1);
    mainRender(false);

    QStringList str_list;
    str_list.push_back(QString("FPS: %1").arg(fps_sum));
    str_list.push_back(QString("uid: %1").arg(current_uid));
    if (view_model_pages >= 0) {
        str_list.push_back(QString("yaw: %1").arg(alt_yaw));
        str_list.push_back(QString("pitch: %1").arg(alt_pitch));
        str_list.push_back(QString("roll: %1").arg(alt_roll));
        str_list.push_back(QString("R: %1").arg(alt_R));
        str_list.push_back(QString("Down: %1").arg(alt_Down));
    }

    celler.draw(asd_model, view_2d_place, view_tier2, str_list.join("\n"));
    glEnable(GL_DEPTH_TEST);

    eventHandler();

    prev_time = current_time;




    if (view_model_pages >= 0) {
        QPainter painter;
        painter.begin(this);

        float sWidth = screenSize.x(), sHeight = screenSize.y();
        float edgeX = 0, edgeY = 0, boxS;
        if (sWidth > sHeight) { boxS = sHeight; edgeX = (sWidth - sHeight) / 2; }
        else { boxS = sWidth; edgeY = (sHeight - sWidth) / 2; }

        int count = 8;
        float aSize = 10, bSize = 1;
        float allSize = aSize * count + bSize * (count + 1);
        float delta = boxS / allSize;
        aSize *= delta; bSize *= delta;
        float iw = aSize + bSize;

        QFont textFont;
        textFont.setPixelSize(int(aSize / 7));
        textFont.setBold(true);
        QFontMetrics metrics(textFont);
        painter.setFont(textFont);
        painter.setPen(QPen(QColor(128, 192, 255)));

        int page = view_model_pages;
        for (int Y = 0; Y < count; Y++) {
            float posY = edgeY + bSize + Y * iw;
            for (int X = 0; X < count; X++) {
                float posX = edgeX + bSize + X * iw;
                int id = (page * count + Y) * count + X;
                drawModelBox(painter, metrics, id, int(posX), int(posY), int(aSize));
            }
        }

        painter.end();
        glEnable(GL_DEPTH_TEST);
    }
}





/*glBindTexture(GL_TEXTURE_2D, texture[3]);
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
glPushMatrix();
    glTranslatef(0, -5, 0);
    glScalef(16, 0.01f, 16);
    glCallList(model[4]);
glPopMatrix();
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );



QVector3D pos = m_camera.translation();
float pitch, yaw, roll;
m_camera.rotation().getEulerAngles(&pitch, &yaw, &roll);
QVector3D fw = m_camera.forward();

int valid;
QVector3D pl_p = MyUtils::insect_line_plane(pos, fw, QVector3D(0, -5, 0), QVector3D(0, 1, 0), valid); // plain point

if (valid) {
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glPushMatrix();
        glTranslatef(pl_p.x(), pl_p.y() + 0.25f, pl_p.z());
        glScalef(0.25f, 0.25f, 0.25f);
        QVector3D rot = m_camera.rotation().toEulerAngles();
        glRotatef(rot.y(), 0, 1, 0);
        glCallList(model[0]);
    glPopMatrix();
}

float m_x = 0, m_y = 0;
float res_x = 0, res_y = 0, res_z = 0, w = 0;


obstacles.drawObstacles(texture[2], model[2]);

if (isMousePressed[myRightButton]) {
    m_x = lastMousePos.x(); m_y = lastMousePos.y();
    float SX = screenSize.x(), SY = screenSize.y();
    m_x = (m_x - SX / 2) / SX * 1000;
    m_y = (SY / 2 - m_y) / SY * 1000;

    QMatrix4x4 inv_mat = (proj_mat * m_camera.toMatrix()).inverted();
    QVector4D res = inv_mat * QVector4D(m_x, m_y, 1, 1);
    w = res.w();
    valid2 = w != 0.f;
    if (valid2) {
        res_x = res.x() / w - pos.x();
        res_y = res.y() / w - pos.y();
        res_z = res.z() / w - pos.z();

        QVector3D new_pl_p = MyUtils::insect_line_plane(pos, QVector3D(res_x, res_y, res_z), QVector3D(0, -5, 0), QVector3D(0, 1, 0), valid); // plain point
        if (valid) {
            QVector3D delta = new_pl_p - pl_p2;
            float L = delta.length();
            delta.normalize();

            int repeats = 1;
            if (L > .03f) { repeats = static_cast<int>(L / .03f); L = .03f; }

            for (int i = 0; i < repeats; i++) {
                QVector3D next = pl_p2 + delta * L;
                QVector2D fly_pos(next.x(), next.z());
                if (obstacles.checkCollision(&fly_pos) >= 0.5f) pl_p2 = next;
                else break;
            }
        }
    }
}
glBindTexture(GL_TEXTURE_2D, texture[4]);
glPushMatrix();
    glTranslatef(pl_p2.x(), pl_p2.y() + .1f, pl_p2.z());
    glScalef(.5f, .5f, .5f);
    glRotatef(90, -1, 0, 0);
    glCallList(model[3]);
glPopMatrix();

GLint viewport[4];
glGetIntegerv(GL_VIEWPORT, viewport);
QString data;
data.sprintf("Camera: %f %f %f\nRotate: %f %f %f\nForward: %f %f %f\nplane (обезьяникс): %lf %lf %lf %s\nlastMousePos: %f %f\nForward: %f %f %f\nplane (муха): %lf %lf %lf %s\nObstacles: %s",
    pos.x(), pos.y(), pos.z(),
    pitch, yaw, roll,
    fw.x(), fw.y(), fw.z(),
    pl_p.x(), pl_p.y(), pl_p.z(), valid ? "true" : "false",
    m_x, m_y,
    res_x, res_y, res_z,
    pl_p2.x(), pl_p2.y(), pl_p2.z(), valid2 ? "true" : "false",
    obstacles.dbg.toStdString().c_str()
);
emit sender(data);*/





/*QLinearGradient gradient(QPointF(50, -20), QPointF(80, 20));
gradient.setColorAt(0.0, Qt::white);
gradient.setColorAt(1.0, QColor(0xa6, 0xce, 0x39));
auto circleBrush = QBrush(gradient);

painter.save();
painter.restore();*/
