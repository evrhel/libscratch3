#pragma once

#include <cstdint>

#include <mutil/mutil.h>
#include <glad/glad.h>

using namespace mutil;

class BaseShader
{
public:
    bool Load(const char *vertex, GLint vertexSize, const char *fragment, GLint fragmentSize);

    void Use();

    int GetUniformLocation(const char *name);

    void SetFloat(int loc, float value);
    void SetInt(int loc, int value);
    void SetVec4(int loc, const Vector4 &value);
    void SetMat4(int loc, const Matrix4 &value);

    BaseShader();
    virtual ~BaseShader();
protected:
    virtual void Init() = 0;
private:
    GLuint _program;
};

class SpriteShader : public BaseShader
{
public:
    void SetProj(const Matrix4 &proj);
    void SetModel(const Matrix4 &model);

    void SetColorEffect(float amount);
    void SetBrightnessEffect(float amount);
    void SetFisheyeEffect(float amount);
    void SetWhirlEffect(float amount);
    void SetPixelateEffect(float amount);
    void SetMosaicEffect(float amount);
    void SetGhostEffect(float amount);

    void SetTexture(GLuint texture);

    void SetColor(const Vector4 &color);

    SpriteShader();
    virtual ~SpriteShader() = default;
protected:
    virtual void Init() override;
private:
    int _projLoc;
    int _modelLoc;

    int _colorEffectLoc;
    int _brightnessEffectLoc;
    int _fisheyeEffectLoc;
    int _whirlEffectLoc;
    int _pixelateEffectLoc;
    int _mosaicEffectLoc;
    int _ghostEffectLoc;

    int _textureLoc;

    int _colorLoc;
};
