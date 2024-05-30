#pragma once

#include <vector>
#include <cstdint>

#include <SDL.h>
#include <glad/glad.h>
#include <mutil/mutil.h>

#define VIEWPORT_WIDTH 480
#define VIEWPORT_HEIGHT 360

// ID of the stage sprite
#define SPRITE_STAGE ((intptr_t)0)

using namespace mutil;

class GLRenderer;

class SpriteRenderer;
class SpriteShader;

class SpriteRenderInfo final
{
public:
    constexpr int64_t GetLayer() const { return _layer; }

    void Update(SpriteShader *ss);

    SpriteRenderInfo();
    ~SpriteRenderInfo();

    bool shouldRender;

    Matrix4 model;
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
    int64_t _layer;

    friend class GLRenderer;
};

class GLRenderer
{
public:
    constexpr SDL_Window *GetWindow() const { return _window; }

    constexpr bool HasError() const { return _window == nullptr; }
   
    constexpr SpriteShader *GetSpriteShader() const { return _spriteShader; }

    constexpr int GetLogicalLeft() const { return _left; }
    constexpr int GetLogicalRight() const { return _right; }
    constexpr int GetLogicalBottom() const { return _bottom; }
    constexpr int GetLogicalTop() const { return _top; }

    void ScreenToStage(int x, int y, int64_t *xout, int64_t *yout) const;

    //! \brief Create a new sprite
    //!
    //! \return A positive integer representing the sprite ID,
    //! or negative if the maximum number of sprites has been reached
    intptr_t CreateSprite();

    //! \brief Get the render info of a sprite
    //!
    //! \param sprite ID of the sprite
    SpriteRenderInfo *GetRenderInfo(intptr_t sprite);

    //! \brief Set the layer of a sprite
    //!
    //! \param sprite ID of the sprite to set the layer of
    //! \param layer The new layer of the sprite, use negative values
    //! to move the sprite relative to the back layer. Layers are 1-indexed.
    //! 0 is an invalid layer, as it is reserved for the stage.
    void SetLayer(intptr_t sprite, int64_t layer);

    void MoveLayer(intptr_t sprite, int64_t direction);

    //! \brief Test whether a sprite is touching a color
    //!
    //! \param sprite ID of the sprite to test
    //! \param color The color to test
    //!
    //! \return Whether the sprite is touching the color
    bool TouchingColor(intptr_t sprite, const Vector3 &color);

    //! \brief Set the logical size of the renderer
    //!
    //! \param left The left side of the renderer
    //! \param right The right side of the renderer
    //! \param bottom The bottom side of the renderer
    //! \param top The top side of the renderer
    void SetLogicalSize(int left, int right, int bottom, int top);

    void BeginRender();

    //! \brief Render everything
    void Render();

    void EndRender();

    //! \brief Create a new renderer
    //!
    //! \param spriteCount The maximum number of sprites to render,
    //! excluding the stage sprite
    GLRenderer(int64_t spriteCount);
    ~GLRenderer();
private:
    SDL_Window *_window;
    SDL_GLContext _context;

    int _left, _right;
    int _bottom, _top;
    
    Matrix4 _proj;

    struct
    {
        GLuint vao, vbo, ebo;
        GLuint indexCount;
    } _quad;

    SpriteShader *_spriteShader;

    SpriteRenderInfo *_sprites; // length _spriteCount
    int64_t *_renderOrder; // length _spriteCount, array of indices into _sprites
    int64_t _spriteCount; // max number of sprites, includes stage

    void Cleanup();

    void CreateQuad();
    void DrawQuad();
    void DestroyQuad();
};
