
#include "mesh.h"

//---------------------------------------------------------------------------
// define - Mesh
//---------------------------------------------------------------------------
Mesh::Mesh(const std::string& filename)
{
    IndexedModel model = ObjModel(filename).ToIndexedModel();
    InitMesh(model);
}

Mesh::Mesh(Vertex* vertices, unsigned int numVertices, unsigned int* indices, unsigned int numIndices)
{
    IndexedModel model;

    for (int index = 0; index < numVertices; ++index)
    {
        model.positions.push_back(vertices[index].GetPosition());
        model.texCoords.push_back(vertices[index].GetTexCoord());
        model.normals.push_back(vertices[index].GetNormal());
    }

    for (int index = 0; index < numIndices; ++index)
    {
        model.indices.push_back(indices[index]);
    }

    InitMesh(model);
}

Mesh::~Mesh(void)
{
}

void Mesh::Draw(void)
{
    glBindVertexArray(vertexArrayObject_);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::InitMesh(const IndexedModel& model)
{
    indexCount_ = model.indices.size();

    glGenVertexArrays(1, &vertexArrayObject_);
    glBindVertexArray(vertexArrayObject_);

    glGenBuffers(Buffers::NumBuffers, vertexArrayBuffers_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffers_[Buffers::Position_VB]);
    glBufferData(GL_ARRAY_BUFFER, model.positions.size() * sizeof(model.positions[0]), &model.positions[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffers_[Buffers::Texture_VB]);
    glBufferData(GL_ARRAY_BUFFER, model.texCoords.size() * sizeof(model.texCoords[0]), &model.texCoords[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffers_[Buffers::Normal_VB]);
    glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(model.normals[0]), &model.normals[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexArrayBuffers_[Buffers::Index_VB]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(model.indices[0]), &model.indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
}
