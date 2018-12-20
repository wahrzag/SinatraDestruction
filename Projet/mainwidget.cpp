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

#include "mainwidget.h"
#include "quadnode.h"
#include "destructmesh.h"
#include <QMouseEvent>

#include <math.h>

double MainWidget::speedChange = .0;
Camera MainWidget::camera = Camera(.0f,.0f, 20.f);

QImage MainWidget::heightMap;

MainWidget::MainWidget(int fps, Season season, QWidget *parent) :
    QOpenGLWidget(parent),
    geometries(nullptr),
    texture(nullptr),
    rotationAxis(0, 0, 1),
    angularSpeed(1),
    fps(fps),
    season(season),
    gravity(.05f)
{
    resize(1280, 720);
    setMouseTracking(true);
    updateSeason();
    objTab = QVector<Mesh*>();
}

MainWidget::~MainWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
    makeCurrent();
    delete texture;
    delete geometries;
    doneCurrent();
}

//! [0]
void MainWidget::mousePressEvent(QMouseEvent *e)
{
    // Save mouse press position
    mousePressPosition = QVector2D(e->localPos());
}

void MainWidget::mouseReleaseEvent(QMouseEvent *e)
{
    // Mouse release position - mouse press position
    QVector2D diff = QVector2D(e->localPos()) - mousePressPosition;

    // Rotation axis along the z axis
    //QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();
    QVector3D n = QVector3D(0.0,0.0,1.0).normalized();

    // Accelerate angular speed relative to the length of the mouse sweep
    qreal acc = static_cast<double>(diff.length()) / 100.0;

    // Calculate new rotation axis as weighted sum
    rotationAxis = (rotationAxis * static_cast<float>(angularSpeed) + n * static_cast<float>(acc)).normalized();

    // Increase angular speed
    angularSpeed += acc;
}
//! [0]

void MainWidget::mouseMoveEvent(QMouseEvent *e)
{
    QPoint center = mapToGlobal(QPoint(width() / 2.f, height() / 2.f));
    camera.processMouseMovement(-(width() / 2.f - e->pos().x()), height() / 2.f - e->pos().y());
    QCursor c = cursor();
    c.setPos(center);
    c.setShape(Qt::BlankCursor);
    setCursor(c);
}

//! [1]
void MainWidget::timerEvent(QTimerEvent *)
{
    // Decrease angular speed (friction)
    //angularSpeed *= 0.99;

    // Stop rotation when speed goes below threshold
    //if (angularSpeed < 0.01) {
    //    angularSpeed = 0.0;
    //} else {
        // Update rotation
    angularSpeed = speedChange;
    rotation = QQuaternion::fromAxisAndAngle(rotationAxis, static_cast<float>(angularSpeed)) * rotation;

        // Request an update
        update();
    //}
}
//! [1]

void MainWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    initShaders();
    initTextures();

//! [2]
    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);
//! [2]

    geometries = new GeometryEngine;
    objTab.append(fromCubeToTrueMesh(CubeCreator(-3,-3,30,5,5,5),10));
    heightMap.load(":/heightmap-1.png");

    // Use QBasicTimer because its faster than QTimer
    timer.start((1000 / fps), this);
}

//! [3]
void MainWidget::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();
}
//! [3]

//! [4]
void MainWidget::initTextures()
{
    // Load cube.png image
    //texture = new QOpenGLTexture(QImage(":/heightmap-1.png"));//.mirrored());
    texture = new QOpenGLTexture(QImage(":/blanc.png"));//.mirrored());

    // Set nearest filtering mode for texture minification
    texture->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    texture->setWrapMode(QOpenGLTexture::Repeat);
}
//! [4]

//! [5]
void MainWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 1.0, far plane to 10.0, field of view 45 degrees
    const qreal zNear = 1.0, zFar = 1000.0, fov = 45.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, static_cast<float>(aspect), zNear, zFar);
}
//! [5]

void MainWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    texture->bind();

//! [6]
    // Calculate model view transformation
    QMatrix4x4 matrix;

    matrix.translate(posX, posY, posZ);

    QQuaternion framing = QQuaternion::fromAxisAndAngle(QVector3D(1,0,0),-45.0);
    matrix.rotate(framing);

    matrix.translate(0.f, -1.8f, 0.f);

    // QVector3D eye = QVector3D(0.0,0.5,-5.0);
    // QVector3D center = QVector3D(0.0,0.0,2.0);
    // QVector3D up = QVector3D(-1,0,0);
    // matrix.lookAt(eye,center,up);

    //camera.ApplyGravity(gravity);
    //if (camera.getY() < .0f) camera.setY(.0f);

    matrix.rotate(rotation);

    program.setUniformValue("a_color", groundColor);
    autoMovePoint();

    // Set modelview-projection matrix
    program.setUniformValue("m_matrix", matrix);
    program.setUniformValue("v_matrix", camera.getViewMatrix());
    program.setUniformValue("p_matrix", projection);



    // Use texture unit 0 which contains cube.png
    program.setUniformValue("texture", 0);

    // Draw cube geometry
    makeCurrent();
    //geometries->drawPlaneGeometry(&program);
    geometries->initQuadTree();
    geometries->drawQuadTree(&program);
    applyGravity();
    Mesh** daObj = objTab.data();
    for(int i=0;i<objTab.size();i++){
        drawObj(daObj[i],&program);
    }
    doneCurrent();

}

void MainWidget::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_Plus:
        speedChange += 0.1;
        break;
    case Qt::Key_Minus:
        speedChange -= 0.1;
        break;
    case Qt::Key_Up:
        QuadNode::p.setY(QuadNode::p.y() + .1f);
        break;
    case Qt::Key_Z:
        camera.processMovement(Direction::FORWARD, 1.0f);
        break;
    case Qt::Key_Down:
        QuadNode::p.setY(QuadNode::p.y() - .1f);
        break;
    case Qt::Key_S:
        camera.processMovement(Direction::BACKWARD, 1.0f);
        break;
    case Qt::Key_Left:
        QuadNode::p.setX(QuadNode::p.x() - .1f);
        break;
    case Qt::Key_Q:
        camera.processMovement(Direction::LEFT, 1.0f);
        break;
    case Qt::Key_Right:
        QuadNode::p.setX(QuadNode::p.x() + .1f);
        break;
    case Qt::Key_D:
        camera.processMovement(Direction::RIGHT, 1.0f);
        break;
    case Qt::Key_A:
        posZ -= 1.f/10.f;
        break;
    case Qt::Key_E:
        posZ += 1.f/10.f;
        break;
    case Qt::Key_N:
        destroyEverything();
        break;
    case Qt::Key_Space:
        camera.processMovement(Direction::UP, 3.f);
        break;
    case Qt::Key_Escape:
        std::exit(EXIT_SUCCESS);
    default:
        break;
    }
}

///////////////////////////////////////////

void MainWidget::applyGravity(){
    Mesh** daObj = objTab.data();
    for(int i=0;i<objTab.size();i++){
        giveStrenght(daObj[i],0,0,-gravity);
        Cube cube = daObj[i]->cube;
        if(Collision(cube,CubeCreator(cube.x,cube.y,qGray(heightMap.pixel((cube.x*heightMap.width()),(cube.y*heightMap.height())))/128.f,0,0,cube.sizeZ))) applyForce(daObj[i]);
    }
}

void MainWidget::drawObj(Mesh* mush,QOpenGLShaderProgram *program)
{
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf(QOpenGLBuffer::IndexBuffer);
    arrayBuf.create();
    indexBuf.create();
    Cube cube = mush->cube;
    VertexData* VD = (VertexData*) malloc(sizeof(VertexData)*24);
    //////Face 1 Devant
    VD[0]={QVector3D(cube.x,cube.y,cube.z), QVector2D(0.0f, 0.0f)};
    VD[1]={QVector3D(cube.x+cube.sizeX,cube.y,cube.z), QVector2D(0.0f, 0.0f)};
    VD[2]={QVector3D(cube.x+cube.sizeX,cube.y+cube.sizeY,cube.z), QVector2D(0.0f, 0.0f)};
    VD[3]={QVector3D(cube.x,cube.y+cube.sizeY,cube.z), QVector2D(0.0f, 0.0f)};
    //////Face Dessus
    VD[4]={QVector3D(cube.x,cube.y,cube.z), QVector2D(0.0f, 0.0f)};
    VD[5]={QVector3D(cube.x,cube.y,cube.z+cube.sizeZ), QVector2D(0.0f, 0.0f)};
    VD[6]={QVector3D(cube.x+cube.sizeX,cube.y,cube.z+cube.sizeZ), QVector2D(0.0f, 0.0f)};
    VD[7]={QVector3D(cube.x+cube.sizeX,cube.y,cube.z), QVector2D(0.0f, 0.0f)};
    //////Face Derrière
    VD[8]={QVector3D(cube.x,cube.y,cube.z+cube.sizeZ), QVector2D(0.0f, 0.0f)};
    VD[9]={QVector3D(cube.x,cube.y+cube.sizeY,cube.z+cube.sizeZ), QVector2D(0.0f, 0.0f)};
    VD[10]={QVector3D(cube.x+cube.sizeX,cube.y+cube.sizeY,cube.z+cube.sizeZ), QVector2D(0.0f, 0.0f)};
    VD[11]={QVector3D(cube.x+cube.sizeX,cube.y,cube.z+cube.sizeZ), QVector2D(0.0f, 0.0f)};
    //////Face Dessous
    VD[12]={QVector3D(cube.x,cube.y+cube.sizeY,cube.z), QVector2D(0.0f, 0.0f)};
    VD[13]={QVector3D(cube.x+cube.sizeX,cube.y+cube.sizeY,cube.z), QVector2D(0.0f, 0.0f)};
    VD[14]={QVector3D(cube.x+cube.sizeX,cube.y+cube.sizeY,cube.z+cube.sizeZ), QVector2D(0.0f, 0.0f)};
    VD[15]={QVector3D(cube.x,cube.y+cube.sizeY,cube.z+cube.sizeZ), QVector2D(0.0f, 0.0f)};
    //////Face Gauche
    VD[16]={QVector3D(cube.x,cube.y,cube.z), QVector2D(0.0f, 0.0f)};
    VD[17]={QVector3D(cube.x,cube.y+cube.sizeY,cube.z), QVector2D(0.0f, 0.0f)};
    VD[18]={QVector3D(cube.x,cube.y+cube.sizeY,cube.z+cube.sizeZ), QVector2D(0.0f, 0.0f)};
    VD[19]={QVector3D(cube.x,cube.y,cube.z+cube.sizeZ), QVector2D(0.0f, 0.0f)};
    //////Face Droite
    VD[20]={QVector3D(cube.x+cube.sizeX,cube.y,cube.z), QVector2D(0.0f, 0.0f)};
    VD[21]={QVector3D(cube.x+cube.sizeX,cube.y,cube.z+cube.sizeZ), QVector2D(0.0f, 0.0f)};
    VD[22]={QVector3D(cube.x+cube.sizeX,cube.y+cube.sizeY,cube.z+cube.sizeZ), QVector2D(0.0f, 0.0f)};
    VD[23]={QVector3D(cube.x+cube.sizeX,cube.y+cube.sizeY,cube.z), QVector2D(0.0f, 0.0f)};

    GLushort indices[36];
    for(int i=0,j=0;i<36;i+=6,j+=4){
        indices[i] = j;
        indices[i+1] = j+1;
        indices[i+2] = j+2;
        indices[i+3] = j+2;
        indices[i+4] = j+3;
        indices[i+5] = j;
    }
    arrayBuf.bind();
    arrayBuf.allocate(VD,24*sizeof(VertexData));
    indexBuf.bind();
    indexBuf.allocate(indices,36*sizeof(GLushort));

    // Offset for position
    quintptr offset = 0;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation("a_position");
    program->enableAttributeArray(vertexLocation);
    program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for texture coordinate
    offset += sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int texcoordLocation = program->attributeLocation("a_texcoord");
    program->enableAttributeArray(texcoordLocation);
    program->setAttributeBuffer(texcoordLocation, GL_FLOAT, offset, 2, sizeof(VertexData));

    // Draw cube geometry using indices from VBO 1
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
}

void MainWidget::destroyEverything(){
    for(int i=0;i<objTab.size();i++){
        fprintf(stderr,"|%d : %f %f %f|",objTab.at(i)->nbp,objTab.at(i)->cube.dirX,objTab.at(i)->cube.dirY,objTab.at(i)->cube.dirY);
    }
    fprintf(stderr,"]  --->  [");
    QVector<Mesh*> newList = QVector<Mesh*>();
    for(;objTab.size()!=0;){
        float x = 1.0*((float)qrand()/RAND_MAX);
        float y = 1.0*((float)qrand()/RAND_MAX);
        float z = 1.0*((float)qrand()/RAND_MAX);
        int nbmesh=0;
        Mesh** meshTab = breakMesh(objTab.takeAt(0),x,y,z,&nbmesh);
        for(int i=0;i<nbmesh;i++){
            newList.append(meshTab[i]);
        }
        //free(meshTab);
    }
    for(;newList.size()!=0;){
        objTab.append(newList.takeAt(0));
    }
    for(int i=0;i<objTab.size();i++){
        fprintf(stderr,"|%d : %f %f %f|",objTab.at(i)->nbp,objTab.at(i)->cube.dirX,objTab.at(i)->cube.dirY,objTab.at(i)->cube.dirY);
    }
    fprintf(stderr,"] size : %d",objTab.size());
}

///////////////////////////////////////////

void MainWidget::nextSeason() {
    switch (season)
    {
        case Season::Printemps:
            season = Season::Ete;
            break;
        case Season::Ete:
            season = Season::Automne;
            break;
        case Season::Automne:
            season = Season::Hiver;
        break;
        case Season::Hiver:
            season = Season::Printemps;
        break;
    };
    updateSeason();

}

void MainWidget::updateSeason() {
    switch (season)
    {
        case Season::Printemps:
            setWindowTitle("Printemps");
            groundColor = QVector4D(0.9f,1.f,0.5f,1.f);
            break;

        case Season::Ete:
            setWindowTitle("Été");
            groundColor = QVector4D(0.9f,0.8f,0.1f,1.f);
            break;

        case Season::Automne:
            setWindowTitle("Automne");
            groundColor = QVector4D(1.f,0.5f,0.1f,1.f);
            break;

        case Season::Hiver:
            setWindowTitle("Hiver");
            groundColor = QVector4D(1.f,1.f,1.f,1.f);
    }
}
