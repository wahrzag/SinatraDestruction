#ifndef QUADNODE_H
#define QUADNODE_H

#include "geometryengine.h"

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

VertexData *getVertices();
int clamp(int num, int min, int max);
float distance(QVector3D p, float x, float y, float size_x, float size_y);
void autoMovePoint();

class QuadNode
{
public:
    QuadNode(float x, float y, float size_x, float size_y, int profondeur_max);
    QuadNode(float x, float y, float size_x, float size_y, float text_x, float text_y, float size_tx, float size_ty, int profondeur);
    void delQuadNode();
    int iteration(VertexData *vertices, int index);
    int static nb_vertices, startDepth;
    float static width, height;
    float static startx, starty, size, maxDist;
    QVector3D static p;


protected:
    void subdivision();

private:
    float x;
    float y;
    float size_x;
    float size_y;
    float text_x;
    float text_y;
    float size_tx;
    float size_ty;
    int profondeur;
    QuadNode *northWest;
    QuadNode *northEast;
    QuadNode *southWest;
    QuadNode *southEast;
};

#endif // QUADNODE_H
