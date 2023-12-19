#include "camera.h"
#include <QDebug>

const QVector3D Camera3D::LocalForward(0.0f, 0.0f, -1.0f);
const QVector3D Camera3D::LocalUp(0.0f, 1.0f, 0.0f);
const QVector3D Camera3D::LocalRight(1.0f, 0.0f, 0.0f);

// Transform By (Add/Scale)
void Camera3D::translate(const QVector3D &dt)
{
  m_dirty = true;
  m_translation += dt;
}

void Camera3D::rotate(const QQuaternion &dr)
{
  m_dirty = true;
  m_rotation = dr * m_rotation;

  float pitch, yaw, roll;
  m_rotation.getEulerAngles(&pitch, &yaw, &roll);
  if (pitch > 70) pitch = 70;
  if (pitch < -65) pitch = -65;
  m_rotation = QQuaternion::fromEulerAngles(pitch, yaw, roll);
}
void Camera3D::rotate(float add_yaw, float add_pitch) {
    m_dirty = true;
    w_pitch += add_pitch;
    w_yaw += add_yaw;
    if (w_pitch > 70) w_pitch = 70;
    if (w_pitch < -65) w_pitch = -65;
    //qDebug("Rotate: %f %f %f", w_pitch, w_yaw, (m_translation - QVector3D(0, -4, 0)).length());
    m_rotation = QQuaternion::fromEulerAngles(w_pitch, w_yaw, 0);
}
QVector3D Camera3D::rotate_res(float add_yaw, float add_pitch) {
    QQuaternion rotation = QQuaternion::fromEulerAngles(add_pitch, add_yaw, 0);
    return rotation.rotatedVector(LocalForward);
}

// Transform To (Setters)
void Camera3D::setTranslation(const QVector3D &t)
{
  m_dirty = true;
  m_translation = t;
}

void Camera3D::setRotation(const QQuaternion &r)
{
  m_dirty = true;
  m_rotation = r;
}

// Accessors
const QMatrix4x4 &Camera3D::toMatrix() {
  if (m_dirty) {
    m_dirty = false;
    m_world.setToIdentity();
    m_world.rotate(m_rotation.conjugated());
    m_world.translate(-m_translation);
  }
  return m_world;
}

// Queries
QVector3D Camera3D::forward() const
{
  return m_rotation.rotatedVector(LocalForward);
}

QVector3D Camera3D::right() const
{
  return m_rotation.rotatedVector(LocalRight);
}

QVector3D Camera3D::up() const
{
  //return m_rotation.rotatedVector(LocalUp);
  return LocalUp;
}

// Qt Streams
QDebug operator<<(QDebug dbg, const Camera3D &transform)
{
  dbg << "Camera3D\n{\n";
  dbg << "Position: <" << transform.translation().x() << ", " << transform.translation().y() << ", " << transform.translation().z() << ">\n";
  dbg << "Rotation: <" << transform.rotation().x() << ", " << transform.rotation().y() << ", " << transform.rotation().z() << " | " << transform.rotation().scalar() << ">\n}";
  return dbg;
}

QDataStream &operator<<(QDataStream &out, const Camera3D &transform)
{
  out << transform.m_translation;
  out << transform.m_rotation;
  return out;
}

QDataStream &operator>>(QDataStream &in, Camera3D &transform)
{
  in >> transform.m_translation;
  in >> transform.m_rotation;
  transform.m_dirty = true;
  return in;
}
