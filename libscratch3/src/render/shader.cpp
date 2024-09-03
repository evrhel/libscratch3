#include "shader.hpp"

#include <cstdio>

bool BaseShader::Load(const char *vertex, GLint vertexSize, const char *fragment, GLint fragmentSize)
{
    if (_program)
        return false;

    int success;
    char infoLog[512];

    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vertex, &vertexSize);
    glCompileShader(vert);
    
    // check for compile errors
    glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vert, 512, NULL, infoLog);
        printf("Vertex shader compilation failed\n%s\n", infoLog);
        return false;
    }

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragment, &fragmentSize);
    glCompileShader(frag);

    // check for compile errors
    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(frag, 512, NULL, infoLog);
        printf("Fragment shader compilation failed\n%s\n", infoLog);

        glDeleteShader(vert);
        return false;
    }

    _program = glCreateProgram();
    glAttachShader(_program, vert);
    glAttachShader(_program, frag);
    glLinkProgram(_program);

    glDeleteShader(vert);
    glDeleteShader(frag);

    // check for linking errors
    glGetProgramiv(_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(_program, 512, NULL, infoLog);
        printf("Program linking failed\n%s\n", infoLog);
        return false;
    }

    Init();

    return true;
}

void BaseShader::Use()
{
    glUseProgram(_program);
}

int BaseShader::GetUniformLocation(const char *name)
{
    return glGetUniformLocation(_program, name);
}

void BaseShader::SetFloat(int loc, float value)
{
    glUniform1f(loc, value);
}

void BaseShader::SetInt(int loc, int value)
{
    glUniform1i(loc, value);
}

void BaseShader::SetVec3(int loc, const Vector3 &value)
{
    glUniform3fv(loc, 1, value.vec);
}

void BaseShader::SetVec4(int loc, const Vector4 &value)
{
    glUniform4fv(loc, 1, value.vec);
}

void BaseShader::SetMat4(int loc, const Matrix4 &value)
{
    glUniformMatrix4fv(loc, 1, GL_FALSE, value.mat);
}

BaseShader::BaseShader() :
    _program(0) { }

BaseShader::~BaseShader()
{
    if (_program)
        glDeleteProgram(_program);
}

void SpriteShader::SetProj(const Matrix4 &proj)
{
    SetMat4(_projLoc, proj);
}

void SpriteShader::SetModel(const Matrix4 &model)
{
    SetMat4(_modelLoc, model);
}

void SpriteShader::SetColorEffect(float amount)
{
    SetFloat(_colorEffectLoc, amount);
}

void SpriteShader::SetBrightnessEffect(float amount)
{
    SetFloat(_brightnessEffectLoc, amount);
}

void SpriteShader::SetFisheyeEffect(float amount)
{
    SetFloat(_fisheyeEffectLoc, amount);
}

void SpriteShader::SetWhirlEffect(float amount)
{
    SetFloat(_whirlEffectLoc, amount);
}

void SpriteShader::SetPixelateEffect(float amount)
{
    SetFloat(_pixelateEffectLoc, amount);
}

void SpriteShader::SetMosaicEffect(float amount)
{
    SetFloat(_mosaicEffectLoc, amount);
}

void SpriteShader::SetGhostEffect(float amount)
{
    SetFloat(_ghostEffectLoc, amount);
}

void SpriteShader::SetTexture(GLuint texture)
{
    SetInt(_textureLoc, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void SpriteShader::SetColor(const Vector4 &color)
{
    SetVec4(_colorLoc, color);
}

void SpriteShader::SetUseColorMask(bool use)
{
    SetInt(_useColorMaskLoc, use);
}

void SpriteShader::SetColorMask(const Vector3 &color)
{
    SetVec3(_colorMaskLoc, color);
}

SpriteShader::SpriteShader() :
    _projLoc(-1), _modelLoc(-1),
    _colorEffectLoc(-1), _brightnessEffectLoc(-1),
    _fisheyeEffectLoc(-1), _whirlEffectLoc(-1),
    _pixelateEffectLoc(-1), _mosaicEffectLoc(-1),
    _ghostEffectLoc(-1),
    _textureLoc(-1),
    _colorLoc(-1),
    _useColorMaskLoc(-1), _colorMaskLoc(-1) { }

void SpriteShader::Init()
{
    _projLoc = GetUniformLocation("uProj");
    _modelLoc = GetUniformLocation("uModel");

    _colorEffectLoc = GetUniformLocation("uColorEffect");
    _brightnessEffectLoc = GetUniformLocation("uBrightnessEffect");
    _fisheyeEffectLoc = GetUniformLocation("uFisheyeEffect");
    _whirlEffectLoc = GetUniformLocation("uWhirlEffect");
    _pixelateEffectLoc = GetUniformLocation("uPixelateEffect");
    _mosaicEffectLoc = GetUniformLocation("uMosaicEffect");
    _ghostEffectLoc = GetUniformLocation("uGhostEffect");

    _textureLoc = GetUniformLocation("uTexture");

    _colorLoc = GetUniformLocation("uColor");

    _useColorMaskLoc = GetUniformLocation("uUseColorMask");
    _colorMaskLoc = GetUniformLocation("uColorMask");
}
