#include "camera.h"
#include <QtMath>

Camera::Camera(float x, float y, float z, float pitch, float yaw)
    : position(QVector3D(x, y, z)), pitch(pitch), yaw(yaw)
{
    mouseSensitivity = .1f;
    updateVectors();
}

float Camera::getX()
{
    return position.x();
}

void Camera::setY(float val)
{
    position.setY(val);
}


float Camera::getY()
{
    return position.y();
}

void Camera::setZ(float val)
{
    position.setZ(val);
}

float Camera::getZ()
{
    return position.z();
}

QVector3D Camera::getPosition(){
    return position;
}

void Camera::ApplyGravity(float gravityForce)
{
    position.setY(position.y() - gravityForce);
}

QMatrix4x4 Camera::getViewMatrix()
{
    QMatrix4x4 viewMatrix;
    viewMatrix.lookAt(position, position + front, up);
    return viewMatrix;
}

void Camera::processMovement(Direction dir, float dist)
{
    switch (dir)
    {
    case FORWARD:
        position += front * dist;
        break;
    case BACKWARD:
        position -= front * dist;
        break;
    case LEFT:
        position -= right * dist;
        break;
    case RIGHT:
        position += right * dist;
        break;
    case UP:
        position += up * dist;
        break;
    case DOWN:
        position -= up * dist;
        break;
    }
}

void Camera::processMouseMovement(float offset_x, float offset_y)
{
    offset_x *= mouseSensitivity;
    offset_y *= mouseSensitivity;
    pitch += offset_y;
    yaw += offset_x;
    if (pitch < -89.f) pitch = -89.f;
    if (pitch > 89.f) pitch = 89.f;
    updateVectors();
}

void Camera::updateVectors()
{
    front.setX(qCos(qDegreesToRadians(pitch)) * qCos((qDegreesToRadians(yaw))));
    front.setY(qSin(qDegreesToRadians(pitch)));
    front.setZ(qCos(qDegreesToRadians(pitch)) * qSin((qDegreesToRadians(yaw))));

    right = QVector3D::crossProduct(front, QVector3D(0,1,0)).normalized();
    up = QVector3D::crossProduct(right, front).normalized();
}
