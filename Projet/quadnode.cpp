#include "quadnode.h"
#include "mainwidget.h"
#include <QVector2D>
#include <QVector3D>
#include <QImage>
#include <cmath>
#include <iostream>

int QuadNode::startDepth = 8;
float QuadNode::width = 100.f;
float QuadNode::height = width;
int QuadNode::nb_vertices;
float QuadNode::startx = -width / 2.f;
float QuadNode::starty = width / 2.f;
float QuadNode::size = width / 2.f;
float QuadNode::maxDist = (pow(QuadNode::width , 2) + pow(QuadNode::height, 2)) / 80.f;
QVector3D QuadNode::p = QVector3D(startx / 2.f, starty / 2.f, 0.f);

/*
float distance(QVector3D p, float x, float y, float size_x, float size_y)
{
    float dist = sqrt(pow(x - p.x(), 2) + pow(y - p.y(), 2));
    float min = dist;
    dist = sqrt(pow(x + size_x - p.x(), 2) + pow(y - p.y(), 2));
    if (dist < min) min = dist;
    dist = sqrt(pow(x - p.x(), 2) + pow(y - size_y - p.y(), 2));
    if (dist < min) min = dist;
    dist = sqrt(pow(x + size_x - p.x(), 2) + pow(y - size_y - p.y(), 2));
    if (dist < min) min = dist;
    dist = sqrt(pow(x + size_x / 2 - p.x(), 2) + pow(y - size_y / 2 - p.y(), 2));
    if (dist < min) min = dist;
    //std::cout << "dist (" << x << ", " << y << ") = " << min << std::endl;
    return min;
}*/

float distance(const QVector3D p, float x, float y, float size_x, float size_y)
{
    float min_x, min_y;
    if (p.x() >= x && p.x() <= x + size_x)
        min_x = p.x();
    else
        min_x = std::abs(x - p.x()) < std::abs(x + size_x - p.x())? x : x + size_x;
    if (p.y() >= y - size_y && p.y() <= y)
        min_y = p.y();
    else
        min_y = std::abs(y - p.y()) < std::abs(y - size_y - p.y())? y : y - size_y;
    //std::cerr << "distance : " << pow(min_x - p.x(), 2) + pow(min_y - p.y(), 2) << std::endl;
    return (pow(min_x - p.x(), 2) + pow(min_y - p.y(), 2)) / QuadNode::maxDist;
}

QuadNode::QuadNode(float x, float y, float size_x, float size_y, int profondeur_max)
    : x(x), y(y), size_x(size_x), size_y(size_y), profondeur(profondeur_max)
{
    nb_vertices = 0;
    float c = size_x / 2.f;
    float d = size_y / 2.f;
    text_x = text_y = .0f;
    size_tx = size_ty = 1.f;
    this->x -= c;
    this->y += d;
//    std::cout << "P = (" << p.x() << ", " << p.y() << ", " << p.z() << ")\n";

    if (profondeur > 0)
    {
        this->subdivision();
    }
    else
    {
        northEast = northWest = southEast = southWest = nullptr;
        nb_vertices++;
    }
}

QuadNode::QuadNode(float x, float y, float size_x, float size_y, float text_x, float text_y, float size_tx, float size_ty, int profondeur)
    : x(x), y(y), size_x(size_x), size_y(size_y), text_x(text_x), text_y(text_y),size_tx(size_tx), size_ty(size_ty), profondeur(profondeur)
{
    if (profondeur - distance(QuadNode::p, x, y, size_x, size_y) > 0)
    {
        this->subdivision();
    }
    else
    {
        this->profondeur = 0;
        northEast = northWest = southEast = southWest = nullptr;
        nb_vertices++;

    }
}

void QuadNode::subdivision()
{
    float c = size_x / 2.f;
    float d = size_y / 2.f;
    float tx = size_tx / 2.f;
    float ty = size_ty / 2.f;
    northWest = new QuadNode(x    , y    , c, d, text_x     , text_y     , tx, ty, profondeur - 1);
    northEast = new QuadNode(x + c, y    , c, d, text_x + tx, text_y     , tx, ty, profondeur - 1);
    southWest = new QuadNode(x    , y - d, c, d, text_x     , text_y + ty, tx, ty, profondeur - 1);
    southEast = new QuadNode(x + c, y - d, c, d, text_x + tx, text_y + ty, tx, ty, profondeur - 1);
}

int clamp(int num, int min, int max)
{
    if (num < min) return min;
    if (num > max) return max;
    return num;
}

int QuadNode::iteration(VertexData *vertices, int index)
{
    if(profondeur == 0)
    {
        float propw = std::abs(startx - x) / QuadNode::width;
        float proph = std::abs(starty - y) / QuadNode::height;

        vertices[index]     = { QVector3D(x         , y, static_cast<float>(qGray(GeometryEngine::heightMap.pixel(clamp(static_cast<int>(GeometryEngine::width * propw), 0, static_cast<int>(GeometryEngine::width) - 1), clamp(static_cast<int>(GeometryEngine::height * proph), 0, static_cast<int>(GeometryEngine::height) - 1))) / 128.0f * 1.5f + 1.5f)), QVector2D(text_x,text_y)};
//        vertices[index]     = { QVector3D(x         , y, .0f), QVector2D(text_x,text_y)};
        propw = std::abs(startx - (x + size_x)) / QuadNode::width;

        vertices[index + 1]     = { QVector3D(x + size_x, y, static_cast<float>(qGray(GeometryEngine::heightMap.pixel(clamp(static_cast<int>(GeometryEngine::width * propw), 0, static_cast<int>(GeometryEngine::width) - 1), clamp(static_cast<int>(GeometryEngine::height * proph), 0, static_cast<int>(GeometryEngine::height) - 1))) / 128.0f * 1.5f + 1.5f)), QVector2D(text_x,text_y)};
//        vertices[index + 1] = { QVector3D(x + size_x, y, .0f), QVector2D(text_x + size_tx,text_y)};
        propw = std::abs(startx - x) / QuadNode::width;
        proph = std::abs(starty - (y - size_y)) / QuadNode::height;

        vertices[index + 2]     = { QVector3D(x         , y - size_y, static_cast<float>(qGray(GeometryEngine::heightMap.pixel(clamp(static_cast<int>(GeometryEngine::width * propw), 0, static_cast<int>(GeometryEngine::width) - 1), clamp(static_cast<int>(GeometryEngine::height * proph), 0, static_cast<int>(GeometryEngine::height) - 1))) / 128.0f * 1.5f + 1.5f)), QVector2D(text_x,text_y)};
//        vertices[index + 2] = { QVector3D(x         , y - size_y, .0f), QVector2D(text_x,text_y + size_ty)};
        propw = std::abs(startx - (x + size_x)) / QuadNode::width;
        proph = std::abs(starty - (y - size_y)) / QuadNode::height;

        vertices[index + 3]     = { QVector3D(x + size_x,  y - size_y, static_cast<float>(qGray(GeometryEngine::heightMap.pixel(clamp(static_cast<int>(GeometryEngine::width * propw), 0, static_cast<int>(GeometryEngine::width) - 1), clamp(static_cast<int>(GeometryEngine::height * proph), 0, static_cast<int>(GeometryEngine::height) - 1))) / 128.0f * 1.5f + 1.5f)), QVector2D(text_x,text_y)};
//        vertices[index + 3] = { QVector3D(x + size_x, y - size_y, 0.f), QVector2D(text_x + size_tx,text_y + size_ty)};
         return index + 4;
    }
    else
    {
        index = northWest->iteration(vertices, index);
        index = northEast->iteration(vertices, index);
        index = southWest->iteration(vertices, index);
        index = southEast->iteration(vertices, index);

        return index;
    }
}

void QuadNode::delQuadNode()
{
    if (northWest != nullptr) northWest->delQuadNode();
    if (northEast != nullptr) northEast->delQuadNode();
    if (southWest != nullptr) southWest->delQuadNode();
    if (southEast != nullptr) southEast->delQuadNode();
    delete this;
}

VertexData *getVertices()
{
    QuadNode *root = new QuadNode(.0f, .0f, QuadNode::width, QuadNode::height, QuadNode::startDepth);
    VertexData *vertices = new VertexData[QuadNode::nb_vertices * 4];
//    std::cout << "nb_vertices = " << QuadNode::nb_vertices << std::endl;
    int index = 0;
    index = root->iteration(vertices, index);
//    std::cout << "index de sorti = " << index << std::endl;
    root->delQuadNode();
    return vertices;
}

void autoMovePoint()
{
    if (!qFuzzyCompare(QuadNode::p.x(), QuadNode::startx / 2.f + QuadNode::size) && qFuzzyCompare(QuadNode::p.y(), QuadNode::starty / 2.f))
        QuadNode::p.setX(QuadNode::p.x() + .1f);
    else if (qFuzzyCompare(QuadNode::p.x(), QuadNode::startx / 2.f + QuadNode::size) && !qFuzzyCompare(QuadNode::p.y(), QuadNode::starty / 2.f - QuadNode::size))
        QuadNode::p.setY(QuadNode::p.y() - .1f);
    else if (!qFuzzyCompare(QuadNode::p.x(), QuadNode::startx / 2.f) && qFuzzyCompare(QuadNode::p.y(), QuadNode::starty / 2.f - QuadNode::size))
        QuadNode::p.setX(QuadNode::p.x() - .1f);
    else if (qFuzzyCompare(QuadNode::p.x(), QuadNode::startx / 2.f) && !qFuzzyCompare(QuadNode::p.y(), QuadNode::starty / 2.f))
        QuadNode::p.setY(QuadNode::p.y() + .1f);
}
