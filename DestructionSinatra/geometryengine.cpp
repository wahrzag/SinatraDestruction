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

#include "geometryengine.h"
#include "quadnode.h"

#include <QVector2D>
#include <QVector3D>
#include <QImage>
#include <iostream>

unsigned int GeometryEngine::width;
unsigned int GeometryEngine::height;
QImage GeometryEngine::heightMap;

//! [0]
GeometryEngine::GeometryEngine()
    : indexBuf(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions();

    // Generate 2 VBOs
    arrayBuf.create();
    indexBuf.create();

    // Initializes cube geometry and transfers it to VBOs
    //initPlaneGeometry();
    initQuadTree();
}

GeometryEngine::~GeometryEngine()
{
    arrayBuf.destroy();
    indexBuf.destroy();
}
//! [0]

void GeometryEngine::initPlaneGeometry()
{
    if(!heightMap.load(":/heightmap-1.png")) {
            std::cerr << "Error : no such a    file." << std::endl;
            return;
    }

    height = static_cast<unsigned int>(heightMap.height());
    width = static_cast<unsigned int>(heightMap.width());

    int size = 64;

    // Create array of 16 x 16 vertices facing the camera  (z=cte)
    VertexData *vertices = new VertexData[size*size];

    for (int i=0;i<size;i++)
        for (int j=0;j<size;j++)
            {
                // Vertex data for face 0
                vertices[size*i+j] = { QVector3D(0.1f*(i-size/2),0.1f*(j-size/2), qGray(heightMap.pixel(static_cast<int>(height / size * i), static_cast<int>(width / size * j))) / 255.0f * 1.5f + 1.5f), QVector2D(static_cast<float>(i) / size,static_cast<float>(j) / static_cast<float>(size))};
                // add height field eg (i-8)*(j-8)/256.0
        }

    // Draw 15 bands each with 32 vertices, with repeated vertices at the end of each band
    int nbv = size * 2 + 4;
        taille_vertices = nbv * (size - 1);
        GLushort *indices = new GLushort[(size - 1) * nbv];

        for (unsigned int i=0;i<size-1;i++)
            {
                indices[nbv*i] = size * i;
                indices[nbv*i+1] = size * i;

                for (unsigned int j=2;j<nbv;j+=2)
                    {
                        indices[nbv*i+j] = size*i +(j-2)/2;
                        indices[nbv*i+j+1] = size*(i+1) + (j-2)/2;
                    }

                indices[nbv*i+nbv - 2] = size*(i+1) + size - 1;
                indices[nbv*i+nbv - 1] = size*(i+1) + size - 1;
        }

    //! [1]
        // Transfer vertex data to VBO 0
        arrayBuf.bind();
        arrayBuf.allocate(vertices, size*size * sizeof(VertexData));

        // Transfer index data to VBO 1
        indexBuf.bind();
        indexBuf.allocate(indices, (size - 1) * nbv * sizeof(GLushort));
    //! [1]
        free(vertices);
        free(indices);
    }

void GeometryEngine::initQuadTree()
{
    if(!heightMap.load(":/heightmap-1.png")) {
            std::cerr << "Error : no such file." << std::endl;
            return;
    }

    height = heightMap.height();
    width = heightMap.width();

    // Create array of 16 x 16 vertices facing the camera  (z=cte)
    VertexData *vertices = getVertices();
    taille_vertices = QuadNode::nb_vertices * 4;
    /*
    for (int i = 0; i < taille_vertices; i++)
    {
        std::cout << vertices[i].position.x() << "  " << vertices[i].position.y() << "  " << vertices[i].position.z() << std::endl;
    }
    */
    // Draw 15 bands each with 32 vertices, with repeated vertices at the end of each band
    taille_indices = QuadNode::nb_vertices * 6;
    GLushort *indices = new GLushort[taille_indices];
//    std::cerr << "taille vertices = " << taille_vertices << "\ntaille indice = " << taille_indices << std::endl;
    for(int i = 0, j = 0; i < taille_indices; i += 6, j += 4)
    {
        //horaire
        /*
        indices[i]     = j;
        indices[i + 1] = j + 1;
        indices[i + 2] = j + 3;
        indices[i + 3] = j + 3;
        indices[i + 4] = j + 2;
        indices[i + 5] = j;
        */
        //antihoraire
        indices[i]     = j;
        indices[i + 1] = j + 2;
        indices[i + 2] = j + 3;
        indices[i + 3] = j + 3;
        indices[i + 4] = j + 1;
        indices[i + 5] = j;
    }
    /*
    for (int i = 0; i < taille_indices; i++)
    {
        std::cout << indices[i] << std::endl;
    }
    */
    //! [1]
    // Transfer vertex data to VBO 0
    arrayBuf.bind();
    arrayBuf.allocate(vertices, taille_vertices * sizeof(VertexData));

    // Transfer index data to VBO 1
    indexBuf.bind();
    indexBuf.allocate(indices, taille_indices * sizeof(GLushort));
    //! [1]
    free(vertices);
    free(indices);
}

//! [2]
void GeometryEngine::drawPlaneGeometry(QOpenGLShaderProgram *program)
{
    // Tell OpenGL which VBOs to use
    arrayBuf.bind();
    indexBuf.bind();

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

    // Draw plane geometry using indices from VBO 1
    glDrawElements(GL_TRIANGLE_STRIP, taille_vertices, GL_UNSIGNED_SHORT, nullptr);
}
//! [2]

//! [2]
void GeometryEngine::drawQuadTree(QOpenGLShaderProgram *program)
{
    // Tell OpenGL which VBOs to use
    arrayBuf.bind();
    indexBuf.bind();

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

    // Draw plane geometry using indices from VBO 1
    glDrawElements(GL_TRIANGLES, taille_indices, GL_UNSIGNED_SHORT, nullptr);
}
//! [2]
