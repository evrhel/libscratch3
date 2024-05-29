#pragma once

#include <vector>
#include <cstdint>

#include <SDL.h>
#include <glad/glad.h>
#include <mutil/mutil.h>

#define VIEWPORT_WIDTH 480
#define VIEWPORT_HEIGHT 360

// The layer of the stage sprite
#define STAGE_LAYER 0

using namespace mutil;

class Mesh;

class SpriteRenderer;
class SpriteShader;

class SpriteRenderInfo final
{
public:
    constexpr const Vector2 &GetPosition() const { return _position; }
    constexpr void SetPosition(const Vector2 &position) { _position = position, _dirty = true; }

    constexpr float GetRotation() const { return _rotation; }
    constexpr void SetRotation(float rotation) { _rotation = rotation, _dirty = true; }

    constexpr const Matrix4 &GetModel() const { return _model; }

    void Update(SpriteShader *ss);

    SpriteRenderInfo();
    ~SpriteRenderInfo();

    float colorEffect;
    float brightnessEffect;
    float fisheyeEffect;
    float whirlEffect;
    float pixelateEffect;
    float mosaicEffect;
    float ghostEffect;
    GLuint texture;
    Vector4 color;
private:
    Vector2 _position;
    float _rotation;

    bool _dirty;

    Matrix4 _model;
};

class GLRenderer
{
public:
    constexpr SDL_Window *GetWindow() const { return _window; }

    constexpr bool HasError() const { return _window == nullptr; }
   
    //! \brief Get a sprite at a layer
    //!
    //! \param layer The layer of the sprite or STAGE_LAYER for the
    //! stage sprite
    //!
    //! \return The sprite at the layer, nullptr if the layer is
    //! invalid
    SpriteRenderInfo *GetLayer(int64_t layer);

    //! \brief Move a sprite at a layer a certain distance
    //!
    //! Does nothing if the layer is invalid, the distance is 0, or
    //! the stage sprite is targeted
    //!
    //! \param layer The layer of the sprite
    //! \param distance The distance to move the sprite
    void MoveLayer(int64_t layer, int64_t distance);

    //! \brief Set the logical size of the renderer
    //!
    //! \param left The left side of the renderer
    //! \param right The right side of the renderer
    //! \param bottom The bottom side of the renderer
    //! \param top The top side of the renderer
    void SetLogicalSize(int left, int right, int bottom, int top);

    //! \brief Render everything
    void Render();

    GLRenderer(int64_t spriteCount);
    ~GLRenderer();
private:
    SDL_Window *_window;
    SDL_GLContext _context;

    int _left, _right;
    int _bottom, _top;

    Matrix4 _proj;

    Mesh *_quad;

    SpriteShader *_spriteShader;

    SpriteRenderInfo *_sprites;
    int64_t _spriteCount;

    void Cleanup();
};
