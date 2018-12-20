/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "geometryengine.h"
#include "camera.h"

#include <QOpenGLWidget>
//#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_5_Core>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QBasicTimer>
#include <QTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#ifndef MESHSTRUCT
#define MESHSTRUCT

typedef struct point{
    float x;
    float y;
    float z;
}Point;

typedef struct cube{
    float x,y,z;
    float sizeX,sizeY,sizeZ;
    float dirX,dirY,dirZ;
    Point center;
}Cube;

typedef struct mesh{
    Point* tab;
    int** links;
    int nbp;
    int nbl;
    Cube cube;
    int** neighbours;
    int* nbNeighbours;
}Mesh;

#endif

class GeometryEngine;

enum class Season { Printemps = 0, Ete = 1, Automne = 2, Hiver = 3 };

class MainWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core


{
    Q_OBJECT

public:
    explicit MainWidget(int fps, Season season, QWidget *parent = nullptr);
    ~MainWidget() override;
    Camera static camera;


protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void timerEvent(QTimerEvent *e) override;

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void keyPressEvent(QKeyEvent *e) override;

    void destroyEverything();
    void drawObj(Mesh* mush,QOpenGLShaderProgram *program);
    void applyGravity();

    void initShaders();
    void initTextures();

private:
    void updateSeason();
    QBasicTimer timer;
    QOpenGLShaderProgram program;
    GeometryEngine *geometries;

    QOpenGLTexture *texture;
    static QImage heightMap;

    QMatrix4x4 projection;

    QVector2D mousePressPosition;
    QVector3D rotationAxis;
    qreal angularSpeed;
    QQuaternion rotation;
    float posX = 0.f, posY = 0.f, posZ = -10.f;
    int fps;
    Season season;
    QVector4D groundColor = QVector4D(1.0, 1.0, 1.0, 1.0);
    static double speedChange;
    float gravity;
    int i = 0;
    QVector<Mesh*> objTab;
    public slots:
        void nextSeason();

};

#endif // MAINWIDGET_H
