#ifndef ASDLOADER_H
#define ASDLOADER_H

#include <GL/gl.h> // для типа GLuint и метода draw
#include <math.h> // INFINITY

#include <lzma/Lzma2Decoder.hpp>
#include <QDebug>
#include <QFile>
#include <QVector3D>
#include <QVector2D>
#include <QImage>
#include <QMatrix4x4>
#include <QOpenGLFramebufferObject>

#include <sboxer.h>

typedef const char* text;
typedef unsigned char byte;
typedef unsigned int uint;

class asdmodel;
interface renderable;
typedef renderable* renderable_p;
class edger_awaiter;

class asdloader {
public:
    struct channel {
        float r;
        float g;
        float b;
        uint tex;
        QString toString() { return "(" + QString::number(r) + " " + QString::number(g) + " " + QString::number(b) + ") " + QString::number(tex); }
    };
    struct material {
        channel albedo;
    };
    struct edger {
        struct edger2 {
            float dx, dy, dz;
            float S;
            QString toString() { return "(" + QString::number(dx) + "; " + QString::number(dy) + "; " + QString::number(dz) + "; " + QString::number(S) + ")"; }
        };
        float min_x, max_x;
        float min_y, max_y;
        float min_z, max_z;
        QString toString() { return "(" + QString::number(min_x) + ".." + QString::number(max_x) + "; " + QString::number(min_y) + ".." + QString::number(max_y) + "; " + QString::number(min_z) + ".." + QString::number(max_z) + ")"; }
        edger2 toEdger2() {
            float S = max_x - min_x, sy = max_x - min_x, sz = max_x - min_x;
            if (sy > S) S = sy;
            if (sz > S) S = sz;
            return {(min_x + max_x) / 2.f, (min_y + max_y) / 2.f, (min_z + max_z) / 2.f, S / 2};
        }
    };
    struct mesh {
        struct face {
            struct polygon {
                QVector3D *vertex;
                QVector2D *texcoord;
                QVector3D *normal;
            };
            polygon v[3];
        };
        QString *name;
        uint face_n;
        face *faces;
        material mat;
        void calc_edger(edger &edger);
        GLuint draw(edger::edger2 edger, GLfloat offset = 0);
    };
    //void (*cb)(asdloader me) = nullptr;
    int cb = 0;
    void *cb2;

private:
    QVector3D *vertexes;
    QVector2D *uvs;
    QVector3D *normals;
    uint mesh_n;
    mesh *meshes;
    std::map<QString, mesh*> name2mesh;
    uint tex_n;
    uint tex_loaded = 0;
    GLuint *textures;

    pthread_t th;
    bool ready = false;
    bool ready2 = false;
    bool render_mode = false;

    uint last_id = 0;
    Sboxers sboxers;
    std::map<uint, renderable_p> id2model;
public:
    asdloader(text filename);
    ~asdloader();
    void calc_edger(edger &edger, mesh *mesh);
    edger find_edger(mesh *mesh);

    void th_inst(text filename);
    renderable* get_model(QString name, edger_awaiter *awaiter = nullptr, bool H = true);
    mesh* get_mesh(QString name) { return name2mesh[name]; }
    GLuint get_texture(uint id) {
        //qDebug("GET TEXTURE %u", id);
        if (id >= tex_loaded) return 0;
        return textures[id];
    }
    void main_th();

    UID get_uid(renderable_p model) { UID res = sboxers.id2uid(++last_id); id2model[res.id] = model; return res; }
    UID get_uid(uint id) { return sboxers.id2uid(id); } // использовалось только для отладки...
    UID get_uid(byte r, byte g, byte b) { return sboxers.color2uid(r, g, b); }
    renderable_p uid2model(UID &uid) { return id2model[uid.id]; }

    bool get_render_mode() { return render_mode; }
    void set_render_mode(bool color_mode) { render_mode = color_mode; }
};





typedef asdloader::mesh mesh;
typedef asdloader::edger edger;

class edger_awaiter {
    renderable_p model;
    edger f_edger;
    bool ready = false;
    edger update();
public:
    edger_awaiter(): edger_awaiter(nullptr) {}
    edger_awaiter(renderable_p model) {
        this->model = model;
    }
    bool is_ready(edger &res) {
        res = update();
        return ready;
    }
};





enum ModelEvents {
    Hover,
    Hovered,
    Unhover,
    Press,
    Move,
    Release,
    Click,
    ModelEvents_SIZE
};
struct ModelEventData {
    int x, y, mouse_button;
    renderable_p model;
    ModelEvents type;
};

interface renderable {
private:
    bool uid_ready = false;
    bool pad[3];
    UID uid;
protected:
    asdloader *parent;
public:
    renderable(asdloader *p) {
        parent = p;
        pad[0] = pad[1]; // nowarning
    }
    virtual void free_storage() {}
    virtual ~renderable() {
        //qDebug("Delete buffer"); OK!
        if (buffer != nullptr) delete buffer;
        if (Slots != nullptr) delete Slots;
    }

  // abstract
    virtual void render(renderable_p root = nullptr, renderable_p root2 = nullptr, void* option = nullptr) = 0;
    virtual bool ready() = 0;
    virtual int ready_state(renderable_p root = nullptr) = 0;
    virtual int parts_count() = 0;

  // free
    virtual edger get_edger(bool &ok, edger *prev = nullptr) {
        ok = true;
        if (prev != nullptr) return *prev;
        return {INFINITY, -INFINITY, INFINITY, -INFINITY, INFINITY, -INFINITY};
    }
    virtual void set_option(int option) { option++; /* unwarning */ }
    virtual int get_option() { return 0; }
    virtual QString name() { return "??? ;'-} !this is abstract zone! :/ ???"; }

    asdloader* get_parent() { return parent; }
    UID get_uid() {
        if (uid_ready) return uid;
        uid = parent->get_uid(this);
        uid_ready = true;
        return uid;
    }
    edger_awaiter get_awaiter() { return edger_awaiter(this); }

private:
    QOpenGLFramebufferObject *buffer = nullptr;
    int prev_width = 0, prev_height = 0, prev_state = 0; uint prev_hash = 0;
    QImage image_cache;
public:
    QImage toImage(int width, int height, float yaw = 39.3f, float pitch = -35.4f, float roll = 0, float R = 2.5f, float Down = 0.2f) {
        bool change_WH = width != prev_width || height != prev_height;
        int state = ready_state();
        bool change_state = state != prev_state;
        uint opthash = (*(uint*) &pitch ^ 0x12345678) + (*(uint*) &yaw ^ 0x87654321) + (*(uint*) &roll * 37) + (*(uint*) &R * 19) + (*(uint*) &Down * 15);
        bool change_hash = opthash != prev_hash;
        bool change = change_WH || change_state || change_hash;
        if (change_WH) {
            if (buffer != nullptr) delete buffer;
            buffer = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::Depth);
            prev_width = width; prev_height = height;
        }
        //qDebug("draw: %u %u   %s", state, prev_state, name().toStdString().c_str());
        //if (state != prev_state) qDebug("State: %u", state);
        if (change) {
            GLfloat proj_m[16];
            glGetFloatv(GL_PROJECTION_MATRIX, proj_m);
            QMatrix4x4 proj_mat = QMatrix4x4(static_cast<float*>(proj_m)).transposed();
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);

            buffer->bind();
            glViewport(0, 0, width, height);

            glClearColor(0, 0, 0, .5f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(.8f, .9f, 1, 1);

            QMatrix4x4 persp = QMatrix4x4();
            float aspectration = float(width) / height;
            persp.perspective(90, aspectration, 0.001f, 1000000);
            //persp.ortho(-2, 2, -2, 2, 0.001f, 1000000);
            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(persp.constData());
            //qDebug("Rotate: %f %f", m_camera.w_pitch, m_camera.w_yaw);
            QQuaternion rotate = QQuaternion::fromEulerAngles(pitch, yaw, roll); //m_camera.w_pitch, m_camera.w_yaw, 0);
            QMatrix4x4 cam;
            cam.setToIdentity();
            cam.translate(QVector3D(0, Down, 0));
            cam.rotate(rotate.conjugated());
            cam.translate(rotate * QVector3D(0, 0, -R));
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(cam.constData());

            render();

            image_cache.fill(0); // заранее выбрасываем все пиксели перед переприсвоением
            image_cache = buffer->toImage();
            buffer->release();

            glMatrixMode(GL_PROJECTION);
            glLoadMatrixf(proj_mat.constData());
            glMatrixMode(GL_MODELVIEW);

            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
            prev_state = state;
            prev_hash = opthash;
        }
        return image_cache;
    }

private:
    struct ModelEventSlot {
        void* inst;
        void (*func)(void* inst, ModelEventData &event);
    };
    ModelEventSlot *Slots = nullptr;
    void check_slots() {
        if (Slots == nullptr) {
            uint size = ModelEvents::ModelEvents_SIZE;
            Slots = new ModelEventSlot[size];
            for (uint i = 0; i < size; i++) Slots[i] = {nullptr, nullptr};
        }
    }

public:
    /*template<typename Func> struct FunctionPointer { enum {ArgumentCount = -1, IsPointerToMemberFunction = false}; };
    template <typename Func>
    void bind_event(const typename FunctionPointer<Func>::Object *inst, Func func) {
        //typedef FunctionPointer<Func> InstType;
        qDebug("call binder");
        inst.*func();
    }

    template <typename T>
    void bind_event(T *inst, void (T::*func)()) {
        //typedef FunctionPointer<Func> InstType;
        qDebug("call binder");
        (inst->*func)();


        //ModelEventSlot<T> slot = {inst, func};
        //ModelEventSlot<T>* slot_p = &slot;

        //void *LOL = inst;
        //void (*FUNC)(void *inst) = func;
        //(FUNC)(LOL);
    }*/

    void bind_event(void* inst, void (*func)(void* inst, ModelEventData &event)) {
        check_slots();
        for (int i = 0; i < ModelEvents::ModelEvents_SIZE; i++) {
            ModelEventSlot *slot = Slots + i;
            slot->inst = inst;
            slot->func = func;
        }
    }
    void bind_event(void* inst, void (*func)(void* inst, ModelEventData &event), ModelEvents type) {
        check_slots();
        ModelEventSlot *slot = Slots + type;
        slot->inst = inst;
        slot->func = func;
    }
    void emit_event(ModelEvents type, int x, int y, int button = 0) {
        if (Slots == nullptr) return;
        ModelEventSlot *slot = Slots + type;
        if (slot->inst == nullptr) return;
        ModelEventData data = { x, y, button, this, type };
        slot->func(slot->inst, data);
    }
};





class asdmodel: public renderable {
private:
    QString myname;
    edger_awaiter *awaiter;
    mesh *mymesh = nullptr;
    GLuint drawer = 0;
    GLuint texture = 0;
    bool useHight;
public:
    asdmodel(): asdmodel(nullptr, "?") {}
    asdmodel(asdloader *p, QString name, edger_awaiter *awaiter = nullptr, bool H = true): renderable(p) {
        myname = name;
        this->awaiter = awaiter;
        useHight = H;
    }
    ~asdmodel() override {
        //qDebug() << "FREE MODEL";
    }
    bool recheck(renderable_p root) {
        if (mymesh == nullptr) {
            mymesh = parent->get_mesh(myname);
            if (mymesh == nullptr) return true;
        }
        if (drawer == 0) {
            //qDebug() << "MODEL GETTED!";
            bool ok;
            auto edgerA = root->get_edger(ok);
            if (!ok) return true;
            auto edger2 = edgerA.toEdger2();
            if (awaiter != nullptr) {
                edger edgerB;
                if (!awaiter->is_ready(edgerB)) { ok = false; return true; }
                auto edger2B = edgerB.toEdger2();
                edger2.S = edger2B.S;
                if (useHight) edger2.dy = edger2B.dy;
            }
            //qDebug() << "EDGER: " << edger2.toString();
            drawer = mymesh->draw(edger2);
        }
        if (!parent->get_render_mode() && texture == 0) {
            auto albedo = mymesh->mat.albedo;
            //if (albedo.tex == 41)
            if (albedo.tex != 0) {
                texture = parent->get_texture(albedo.tex);
                //if (texture != 0) qDebug() << "TEXTURE GETTED!" << QString::number(texture);
            }
        }
        return false;
    }

  // antiabstract
    void render(renderable_p root = nullptr, renderable_p root2 = nullptr, void* option = nullptr) override {
        void *lol = option; option = lol; // unwarning

        if (root == nullptr) root = root2 = this;
        if (recheck(root)) return;

        bool color_mode = parent->get_render_mode();
        if (color_mode) {
            UID uid = root2->get_uid();
            //qDebug("color: %02x%02x%02x", uid.r, uid.g, uid.b);
            glBindTexture(GL_TEXTURE_2D, 0);
            glColor3f(uid.r / 255.f, uid.g / 255.f, uid.b / 255.f);
            glCallList(drawer);
        } else {
            auto albedo = mymesh->mat.albedo;
            if (texture == 0) glColor3f(albedo.r, albedo.g, albedo.b);
            glBindTexture(GL_TEXTURE_2D, texture);
            glCallList(drawer);
            glColor3f(1, 1, 1);
        }
    }
    bool ready() override { return drawer != 0; }
    int ready_state(renderable_p root = nullptr) override {
        recheck(root == nullptr ? this : root);
        return (drawer != 0) + (texture != 0);
    }
    int parts_count() override { return 1; }

  // free
    edger get_edger(bool &ok, edger *prev = nullptr) override {
        edger res = renderable::get_edger(ok, prev);
        if (mymesh == nullptr) mymesh = parent->get_mesh(myname);
        ok = mymesh != nullptr;
        if (ok) mymesh->calc_edger(res);
        return res;
    }
    QString name() override { return QString("\"%0\"").arg(myname); }
};



class model_group: public renderable {
private:
    uint count;
    renderable_p *storage;
    bool cache = false;
    bool common_root;
    edger f_edger;
public:
    model_group(uint count, renderable_p *arr, bool common_root = true): renderable(arr[0]->get_parent()) {
        this->count = count;
        this->common_root = common_root;
        //size_t size = sizeof(renderable_p*) * count;
        //storage = static_cast<renderable_p*>(malloc(size));
        //memcpy(storage, arr, size);
        storage = arr;
    }
    void free_storage() override {
        //qDebug() << "FREE STORAGE OF GROUP";
        for (uint i = 0; i < count; i++) delete storage[i];
    }
    ~model_group() override {
        //qDebug() << "FREE GROUP";
        delete storage;
    }

  // antiabstract
    void render(renderable_p root = nullptr, renderable_p root2 = nullptr, void* option = nullptr) override {
        if (root == nullptr) root = root2 = this;
        for (uint i = 0; i < count; i++) storage[i]->render(common_root ? root : storage[i], root2, option);
    }
    bool ready() override { return cache; }
    int ready_state(renderable_p root = nullptr) override {
        if (root == nullptr) root = this;
        int res = 0;
        for (uint i = 0; i < count; i++) res += storage[i]->ready_state(root);
        return res;
    }
    int parts_count() override {
        int res = 0;
        for (uint i = 0; i < count; i++) res += storage[i]->parts_count();
        return res;
    }

  // free
    edger get_edger(bool &ok, edger *prev = nullptr) override {
        if (cache) {
            ok = true;
            return f_edger;
        }
        edger res = renderable::get_edger(ok, prev);
        for (uint i = 0; i < count; i++) {
            res = storage[i]->get_edger(ok, &res);
            if (!ok) return res;
        }
        f_edger = res;
        cache = true;
        return res;
    }
    QString name() override {
        QStringList list;
        for (uint i = 0; i < count; i++) list.append(storage[i]->name());
        return QString("g(%0)").arg(list.join(", "));
    }
};



class model_wrapper: public renderable {
private:
    renderable_p group;
    int selected = 0;
public:
    model_wrapper(renderable_p g): renderable(g->get_parent()) {
        group = g;
    }

  // antiabstract
    void render(renderable_p root = nullptr, renderable_p root2 = nullptr, void* option = nullptr) override {
        if (root == nullptr) root = root2 = this;
        if (option == nullptr) option = &selected;
        group->render(root, root2, option);
    }
    bool ready() override { return group->ready(); }
    int ready_state(renderable_p root = nullptr) override {
        return group->ready_state(root == nullptr ? this : root);
    }
    int parts_count() override { return group->parts_count(); }

  // free
    edger get_edger(bool &ok, edger *prev = nullptr) override {
        return group->get_edger(ok, prev);
    }
    void set_option(int option) override { selected = option; }
    int get_option() override { return selected; }
    QString name() override { return QString("w(%0)").arg(group->name()); }
};



class model_union: public renderable {
private:
    uint count;
    renderable_p *storage;
    int selected = 0;
public:
    model_union(uint count, renderable_p *arr): renderable(arr[0]->get_parent()) {
        this->count = count;
        storage = arr;
    }
    ~model_union() override {
        //qDebug() << "FREE UNION";
        // Не нужно чистить содержимое storage!
        delete storage;
    }
    renderable_p get_current() { return storage[uint(get_option()) % count]; }

  // antiabstract
    void render(renderable_p root = nullptr, renderable_p root2 = nullptr, void* option = nullptr) override {
        if (root == nullptr) root = root2 = this;
        if (option == nullptr) option = &selected;
        else set_option(*static_cast<int*>(option));
        get_current()->render(root, root2, option);
    }
    bool ready() override { return get_current()->ready(); }
    int ready_state(renderable_p root = nullptr) override {
        return get_current()->ready_state(root == nullptr ? this : root);
    }
    int parts_count() override { return get_current()->parts_count(); }

  // free
    edger get_edger(bool &ok, edger *prev = nullptr) override {
        return get_current()->get_edger(ok, prev);
    }
    void set_option(int option) override { selected = option; }
    int get_option() override { return selected; }
    QString name() override {
        QStringList list;
        for (uint i = 0; i < count; i++) list.append(storage[i]->name());
        return QString("u(%0 -> %1)").arg(selected).arg(list.join(", "));
    }
};

#endif // ASDLOADER_H
