#include "objloader.h"

using std::vector;
using std::string;

#define GET_CODE(s) s[0].unicode() << 16 | s[1].unicode()
const int s_v = 0x760020; // GET_CODE(QString("v "));
const int s_vn = 0x76006e; // GET_CODE(QString("vn"));
const int s_vt = 0x760074; // GET_CODE(QString("vt"));
const int s_f = 0x660020; // GET_CODE(QString("f "));

GLuint objloader::load(const QString &filename) {
    vector<QVector3D>().swap(vertex);
    vector<QVector2D>().swap(uvs);
    vector<QVector3D>().swap(normals);
    vector<face>().swap(faces);

    QFile in(filename);

    if(!in.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "Не удалось открыть файл с моделькой!";
        return 0;
    }

    char buff[256];
    float x, y, z;
    while(!in.atEnd()) {
        in.readLine(buff, 256);
        QString line = QString(buff).simplified();
        if (line.startsWith("#") || line.length() < 2) continue;

        QStringList arr = line.split(" ");
        switch (GET_CODE(line)) {
        case s_v: // вершина
            x = arr[1].toFloat(); y = arr[2].toFloat(); z = arr[3].toFloat();
            // qDebug() << "vertex:" << line << QVector3D(x, y, z); Всё OK!
            vertex.push_back(QVector3D(x, y, z));
            break;
        case s_vn: // нормаль
            x = arr[1].toFloat(); y = arr[2].toFloat(); z = arr[3].toFloat();
            // qDebug() << "normal:" << line << QVector3D(x, y, z); Всё OK!
            normals.push_back(QVector3D(x, y, z));
            break;
        case s_vt: // текстуроточка
            x = arr[1].toFloat(); y = arr[2].toFloat();
            // qDebug() << "TexCoord0:" << line << QVector2D(x, 1 - y); Всё OK!
            uvs.push_back(QVector2D(x, 1 - y));
            break;
        case s_f: // личико (полигон)
            face::vertex v1, v2, v3;

            QStringList list = arr[1].split("/");
            v1.v_i = list[0].toUInt() - 1; v1.vt_i = list[1].toUInt() - 1; v1.vn_i = list[2].toUInt() - 1;

            list = arr[2].split("/");
            v2.v_i = list[0].toUInt() - 1; v2.vt_i = list[1].toUInt() - 1; v2.vn_i = list[2].toUInt() - 1;

            list = arr[3].split("/");
            v3.v_i = list[0].toUInt() - 1; v3.vt_i = list[1].toUInt() - 1; v3.vn_i = list[2].toUInt() - 1;

            // qDebug() << "SallyFace:" << line << v1.v_i << v2.v_i << v3.v_i; Всё OK!
            faces.push_back(face(v1, v2, v3));
            break;
        }
    }
    GLuint num = draw();
    qDebug() << "Model_Idx:" << QString::number(num);
    return num;
}

GLuint objloader::draw(GLfloat offset)
{
    GLuint num = glGenLists(1);
    glNewList(num,GL_COMPILE);
    size_t N_faces = faces.size();
    bool use_tex = uvs.size() > 0;
    for(size_t i=0; i < N_faces; ++i)
    {
        face f = faces[i];

        glBegin(GL_TRIANGLES);

        QVector3D normal = normals[f.v[0].vn_i];
        QVector3D v = vertex[f.v[0].v_i];
        glNormal3f(normal.x(), normal.y(), normal.z());

        if (use_tex) glTexCoord2f(uvs[f.v[0].vt_i].x() + offset, uvs[f.v[0].vt_i].y() + offset);
        else glTexCoord2f(0, 0); // glTexCoord2f(0.5f*(normal.x()+1), 0.5f*(normal.y()+1));
        glVertex3f(v.x(), v.y(), v.z()); // читаем текстурную вершину из файла

        normal = normals[f.v[1].vn_i];
        v = vertex[f.v[1].v_i];
        glNormal3f(normal.x(), normal.y(), normal.z());

        if (use_tex) glTexCoord2f(uvs[f.v[1].vt_i].x() + offset, uvs[f.v[1].vt_i].y() + offset);
        else glTexCoord2f(0, 1); // glTexCoord2f(0.5f*(normal.x()+1), 0.5f*(normal.y()+1));
        glVertex3f(v.x(), v.y(), v.z());

        normal = normals[f.v[2].vn_i];
        v = vertex[f.v[2].v_i];
        glNormal3f(normal.x(), normal.y(), normal.z());

        if (use_tex) glTexCoord2f(uvs[f.v[2].vt_i].x() + offset, uvs[f.v[2].vt_i].y() + offset);
        else glTexCoord2f(1, 0); // glTexCoord2f(0.5f*(normal.x()+1), 0.5f*(normal.y()+1));
        glVertex3f(v.x(), v.y(), v.z());

        glEnd();
    }
    glEndList();
    return num;     // возвращаем номер дисплей листа
}
