#ifndef CAMERA_H
#define CAMERA_H


#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

enum Direction {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera
{
public:
    Camera(float x, float y, float z, float pitch = 0.f, float yaw = -90.f);
    QMatrix4x4 getViewMatrix();
    void processMovement(Direction dir, float dist);
    void processMouseMovement(float offset_x, float offset_y);
    float getX();
    void setY(float val);
    float getY();
    void setZ(float val);
    float getZ();
    QVector3D getPosition();
    void ApplyGravity(float gravityForce);

protected:
    void updateVectors();

private:
    QVector3D position;
    float pitch;
    float yaw;
    QVector3D front;
    QVector3D right;
    QVector3D up;
    float mouseSensitivity;

};

#endif // CAMERA_H
