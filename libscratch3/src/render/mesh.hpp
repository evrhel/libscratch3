#pragma once

#include <cstdint>

#include <glad/glad.h>
#include <mutil/mutil.h>

using namespace mutil;

class Mesh final
{
public:
    void Load(const Vector4 *vertices, size_t vertexCount, const uint32_t *indices, size_t indexCount);

    void Render();

    Mesh();
    ~Mesh();
private:
    GLuint _vao, _vbo, _ebo;
    size_t _indexCount;
};
