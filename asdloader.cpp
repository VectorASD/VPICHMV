#include <asdloader.h>

#include <pthread.h>

uint uleb128(byte* &arr) {
    uint res = 0, i = 0;
    byte b;
    do {
        b = *(arr++);
        res |= (b & 127u) << i;
        i += 7;
    } while (b > 127);
    return res;
}
int sleb128(byte* &arr) {
    uint n = uleb128(arr);
    return n & 1 ? -(n >> 1)-1 : n >> 1;
}
float fleb128(byte* &arr) {
    return sleb128(arr) / 1000000.f;
}
QString* str_leb128(byte* &arr) {
    int len = static_cast<int>(uleb128(arr));
    text pos = reinterpret_cast<text>(arr);
    arr += len;
    return new QString(QByteArray(pos, len));
}
QImage img_leb128(byte* &arr) {
    int len = static_cast<int>(uleb128(arr));
    text pos = reinterpret_cast<text>(arr);
    arr += len;
    return QImage::fromData(QByteArray(pos, len));
}

typedef asdloader::mesh::face face;
typedef face::polygon polygon;
struct th_args {
    text filename;
    asdloader *me;
};

void* th_func(void *argv) {
    qDebug() << "YEAH!";
    th_args *args = static_cast<th_args*>(argv);
    text filename = args->filename;
    asdloader *self = args->me;
    free(args);

    self->th_inst(filename);
    return nullptr;
}

asdloader::asdloader(text filename) {
    th_args *args = static_cast<th_args*>(malloc(sizeof(th_args)));
    args->filename = filename;
    args->me = this;
    pthread_create(&th, nullptr, th_func, args);
}

#include <GL/glext.h>

static pthread_spinlock_t spin;
void asdloader::main_th() {
    int cb = this->cb;
    this->cb = 0;
    if (cb == 1) {
        glGenTextures(GLsizei(tex_n), textures);
        pthread_spin_unlock(&spin);
        return;
    }

    cb -= 2;
    QImage *image = static_cast<QImage*>(cb2);
    int SX = image->width(), SY = image->height();
    glBindTexture(GL_TEXTURE_2D, textures[cb]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR);
    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GLsizei(SX), GLsizei(SY), 0, GL_RGBA, GL_UNSIGNED_BYTE, image->bits());
    pthread_spin_unlock(&spin);
}

void asdloader::th_inst(text filename) {
    qDebug() << "Открытие ASD-файла...";
    QFile in(filename);
    if(!in.open(QFile::ReadOnly)){
        qDebug() << "Не удалось открыть файл с ASD-моделькой!";
        return;
    }
    QByteArray ba = in.readAll();
    in.close();
    size_t size = static_cast<size_t>(ba.size());
    byte *data = reinterpret_cast<byte*>(ba.data());
    size_t dsize = static_cast<size_t>(uleb128(data));
    size_t orig_size = dsize;
    char *ddata = static_cast<char*>(malloc(dsize));

    qDebug("Декомпрессия ASD-файла (%.5f Мб -> %.5f Мб)...", size / 1024.f / 1024, dsize / 1024.f / 1024);
    lzma::Status status;
    lzma::Lzma2Decode(ddata, dsize, data, size, 0x18, lzma::FinishMode::End, status);
    ba.clear();
    if (status != lzma::Status::FinishedWithMark || dsize != orig_size) {
        qDebug() << "Неудачная декомпрессия ASD-модельки!";
        free(ddata);
        return;
    }
    qDebug() << "Считывание ASD-модели...";
    data = reinterpret_cast<byte*>(ddata);

    uint verts = uleb128(data);
    qDebug() << "Verts:" << QString::number(verts);
    vertexes = static_cast<QVector3D*>(malloc(sizeof(QVector3D) * verts));
    for (uint i = 0; i < verts; i++) vertexes[i] = QVector3D(fleb128(data), fleb128(data), fleb128(data));
    //qDebug() << "Vert:" << QString::number(vert.x) << QString::number(vert.y) << QString::number(vert.z);
    //if (i > 5) break;

    uint texcoords = uleb128(data);
    qDebug() << "UVs:" << QString::number(texcoords);
    uvs = static_cast<QVector2D*>(malloc(sizeof(QVector2D) * texcoords));
    for (uint i = 0; i < texcoords; i++) {
        float x = fleb128(data), y = fleb128(data);
        uvs[i] = QVector2D(x, y);
        //uvs[i] = QVector2D(fleb128(data), fleb128(data));
        //if (i < 100) qDebug() << "UVS:" << QString::number(x) << QString::number(y);
    }

    uint norms = uleb128(data);
    qDebug() << "Norms:" << QString::number(norms);
    normals = static_cast<QVector3D*>(malloc(sizeof(QVector3D) * norms));
    for (uint i = 0; i < norms; i++) normals[i] = QVector3D(fleb128(data), fleb128(data), fleb128(data));

    mesh_n = uleb128(data);
    qDebug() << "Meshes:" << QString::number(mesh_n);
    meshes = static_cast<mesh*>(malloc(sizeof(mesh) * mesh_n));
    for (uint i = 0; i < mesh_n; i++) {
        mesh *cur_mesh = meshes + i;
        QString *name = str_leb128(data);
        uint face_n = uleb128(data);
        channel albedo = {fleb128(data), fleb128(data), fleb128(data), uleb128(data)};
        material mat = {albedo};
        //qDebug() << "Name:" << *name << QString::number(face_n) << albedo.toString();
        face *faces = static_cast<face*>(malloc(sizeof(face) * face_n));
        for (uint j = 0; j < face_n; j++) {
            face *face = faces + j;
            face->v[0] = {vertexes + uleb128(data), uvs + uleb128(data), normals + uleb128(data)};
            face->v[1] = {vertexes + uleb128(data), uvs + uleb128(data), normals + uleb128(data)};
            face->v[2] = {vertexes + uleb128(data), uvs + uleb128(data), normals + uleb128(data)};
        }
        cur_mesh->name = name;
        cur_mesh->face_n = face_n;
        cur_mesh->faces = faces;
        cur_mesh->mat = mat;
        name2mesh[*name] = cur_mesh;
    }
    ready = true; // походу ненужен, ибо есть name2mesh ;'-}}}

    tex_n = uleb128(data);
    qDebug() << "Textures:" << QString::number(tex_n);
    textures = static_cast<GLuint*>(malloc(sizeof(GLuint) * tex_n));

    //while (spin < 1) Sleep(1);
    pthread_spin_init(&spin, 0);
    pthread_spin_lock(&spin);

    cb = 1;
    pthread_spin_lock(&spin);

    for (uint i = 0; i < tex_n; i++) {
        QImage image = img_leb128(data).convertToFormat(QImage::Format_RGBA8888);
        int SX = image.width(), SY = image.height();
        if (SX == 0 || SY == 0) {
            textures[i] = 0;
            tex_loaded = i + 1;
            continue;
        }

        cb2 = &image;
        cb = int(i + 2);
        pthread_spin_lock(&spin);
        //qDebug() << "TextureLoaded:" << QString::number(i) << "|" << SX << "x" << SY;

        tex_loaded = i + 1;
    }
    ready2 = true;

    free(ddata);
    qDebug("Поток парсера сдулся ;'-}");
}

asdloader::~asdloader() {
    if (ready) {
        free(vertexes);
        free(uvs);
        free(normals);
        for (uint i = 0; i < mesh_n; i++) free(meshes[i].name);
        free(meshes);
    } else pthread_kill(th, SIGTERM);
    if (ready2) free(textures);
}





void asdloader::mesh::calc_edger(edger &edger) {
    uint face_n = this->face_n;
    face *faces = this->faces;
    float min_x = edger.min_x, max_x = edger.max_x, min_y = edger.min_y, max_y = edger.max_y, min_z = edger.min_z, max_z = edger.max_z;
    for (uint i = 0; i < face_n; i++) {
        face *face = faces + i;
        for (int j = 0; j < 3; j++) {
            QVector3D *v = face->v[j].vertex;
            float x = v->x(), y = v->y(), z = v->z();
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
            if (z < min_z) min_z = z;
            if (z > max_z) max_z = z;
        }
    }
    edger.min_x = min_x; edger.max_x = max_x; edger.min_y = min_y; edger.max_y = max_y; edger.min_z = min_z; edger.max_z = max_z;
}

asdloader::edger asdloader::find_edger(mesh *mesh) {
    edger res = {INFINITY, -INFINITY, INFINITY, -INFINITY, INFINITY, -INFINITY};
    if (mesh == nullptr)
        for (uint i = 0; i < mesh_n; i++) meshes[i].calc_edger(res);
    else mesh->calc_edger(res);
    return res;
}

GLuint asdloader::mesh::draw(edger::edger2 edger, GLfloat offset) {
    GLuint num = glGenLists(1);
    uint face_n = this->face_n;
    face *faces = this->faces;
    glNewList(num, GL_COMPILE);
    //auto edger = find_edger(mesh).toEdger2();
    QVector3D dxyz(edger.dx, edger.dy, edger.dz);
    float S = edger.S;

    bool use_tex = true;
    for(uint i = 0; i < face_n; i++) {
        face *face = faces + i;
        polygon *p_arr = face->v;

        glBegin(GL_TRIANGLES);

        for (int j = 0; j < 3; j++) {
            polygon *p = p_arr + j;
            QVector3D v = ((*p->vertex) - dxyz) / S;
            QVector2D *uv = p->texcoord;
            QVector3D *normal = p->normal;

            glNormal3f(normal->x(), normal->y(), normal->z());

            //qDebug() << "UV:" << QString::number(uv->x()) << QString::number((1 - uv->y()));

            if (use_tex) glTexCoord2f(uv->x() + offset, (1 - uv->y()) + offset);
            else glTexCoord2f(0, 0); // glTexCoord2f(0.5f*(normal.x()+1), 0.5f*(normal.y()+1));
            glVertex3f(v.x(), v.y(), v.z());
        }
        glEnd();
    }
    glEndList();
    return num;     // возвращаем номер дисплей листа
}

renderable* asdloader::get_model(QString name, edger_awaiter *awaiter, bool H) {
    return new asdmodel(this, name, awaiter, H);
}



edger edger_awaiter::update() {
    if (ready || model == nullptr) return f_edger;
    bool ok = true;
    f_edger = model->get_edger(ok);
    if (ok) ready = true;
    return f_edger;
}
