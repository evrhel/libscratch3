#pragma once

#include <SDL.h>
#include <glad/glad.h>

class GLRenderer
{
public:
    void Resize(int width, int height);

    GLRenderer();
    ~GLRenderer();
private:
};
