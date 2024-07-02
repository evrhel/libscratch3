#pragma once

#include <vector>
#include <cstdint>

#include <SDL.h>
#include <glad/glad.h>
#include <mutil/mutil.h>

#include <scratch3/scratch3.h>

#define VIEWPORT_WIDTH 480
#define VIEWPORT_HEIGHT 360

// ID of the stage sprite
#define SPRITE_STAGE ((intptr_t)0)

#define QUAD_INDEX_COUNT 6

using namespace mutil;

class GLRenderer;

class SpriteRenderer;
class SpriteShader;

class Sprite;
class SpriteList;

struct GraphicEffectController final
{
public:
    constexpr double GetColorEffect() const { return _colorEffect; }
    constexpr double GetBrightnessEffect() const { return _brightnessEffect; }
    constexpr double GetFisheyeEffect() const { return _fisheyeEffect; }
    constexpr double GetWhirlEffect() const { return _whirlEffect; }
    constexpr double GetPixelateEffect() const { return _pixelateEffect; }
    constexpr double GetMosaicEffect() const { return _mosaicEffect; }
    constexpr double GetGhostEffect() const { return _ghostEffect; }

    constexpr float GetColorFactor() const { return _colorFactor; }
    constexpr float GetBrightnessFactor() const { return _brightnessFactor; }
    constexpr float GetFisheyeFactor() const { return _fisheyeFactor; }
    constexpr float GetWhirlFactor() const { return _whirlFactor; }
    constexpr float GetPixelateFactor() const { return _pixelateFactor; }
    constexpr float GetMosaicFactor() const { return _mosaicFactor; }
    constexpr float GetGhostFactor() const { return _ghostFactor; }

    inline void AddColorEffect(const double amount) { SetColorEffect(_colorEffect + amount); }
    inline void SetColorEffect(const double effect)
    {
        _colorEffect = effect;
        _colorFactor = fract(static_cast<float>(effect) / 200);
    }

    inline void AddBrightnessEffect(const double amount) { SetBrightnessEffect(_brightnessEffect + amount); }
    inline void SetBrightnessEffect(const double effect)
    {
        _brightnessEffect = effect;
        if (_brightnessEffect < -100.0) _brightnessEffect = -100.0;
        else if (_brightnessEffect > 100.0) _brightnessEffect = 100.0;
        _brightnessFactor = static_cast<float>(_brightnessEffect) / 100.0f + 1.0f;
    }

    inline void AddFisheyeEffect(const double amount) { SetFisheyeEffect(_fisheyeEffect + amount); }
    inline void SetFisheyeEffect(const double effect)
    {
        _fisheyeEffect = effect;
        _fisheyeFactor = (static_cast<float>(_fisheyeEffect) + 100.0f) / 100.0f;
	}

    inline void AddWhirlEffect(const double amount) { SetWhirlEffect(_whirlEffect + amount); }
    inline void SetWhirlEffect(const double effect)
    {
        _whirlEffect = effect;
        _whirlFactor = radians(-static_cast<float>(_whirlEffect));
    }

    inline void AddPixelateEffect(const double amount) { SetPixelateEffect(_pixelateEffect + amount); }
    inline void SetPixelateEffect(const double effect)
    {
        _pixelateEffect = effect;
        if (_pixelateEffect < 0) _pixelateEffect = 0;
        _pixelateFactor = static_cast<float>(_pixelateEffect) / 10;
    }

    inline void AddMosaicEffect(const double amount) { SetMosaicEffect(_mosaicEffect + amount); }
    inline void SetMosaicEffect(const double effect)
    {
        _mosaicEffect = effect;
        if (_mosaicEffect < 0.0) _mosaicEffect = 0.0;
        _mosaicFactor = clamp(mutil::round((static_cast<float>(_mosaicEffect) + 10.0f) / 10.0f), 1.0f, 512.0f);
	}

    inline void AddGhostEffect(const double amount) { SetGhostEffect(_ghostEffect + amount); }
    inline void SetGhostEffect(const double effect)
    {
        _ghostEffect = effect;
        if (_ghostEffect < 0) _ghostEffect = 0;
        else if (_ghostEffect > 100) _ghostEffect = 100;
        _ghostFactor = static_cast<float>(_ghostEffect) / 100.0f;
	}

    constexpr void ClearEffects()
    {
        _colorEffect = 0, _colorFactor = 0;
        _brightnessEffect = 0, _brightnessFactor = 1;
        _fisheyeEffect = 0, _fisheyeFactor = 1;
        _whirlEffect = 0, _whirlFactor = 0;
        _pixelateEffect = 0, _pixelateFactor = 1;
        _mosaicEffect = 0, _mosaicFactor = 1;
        _ghostEffect = 0, _ghostFactor = 0;
    }
private:
    double _colorEffect;
    float _colorFactor;
    
    double _brightnessEffect;
    float _brightnessFactor;

    double _fisheyeEffect;
    float _fisheyeFactor;

    double _whirlEffect;
    float _whirlFactor;

    double _pixelateEffect;
    float _pixelateFactor;

    double _mosaicEffect;
    float _mosaicFactor;

    double _ghostEffect;
    float _ghostFactor;
};

struct GLViewport
{
    int x, y;
    int width, height;

    constexpr void Resize(const bool freeAspect, const int fbWidth, const int fbHeight)
    {
        if (freeAspect)
        {
            width = fbWidth;
            height = fbHeight;
            x = 0;
            y = 0;
        }
        else
        {
            if (fbWidth * VIEWPORT_HEIGHT > fbHeight * VIEWPORT_WIDTH)
            {
                width = fbHeight * VIEWPORT_WIDTH / VIEWPORT_HEIGHT;
                height = fbHeight;
                x = (fbWidth - width) / 2;
                y = 0;
            }
            else
            {
                width = fbWidth;
                height = fbWidth * VIEWPORT_HEIGHT / VIEWPORT_WIDTH;
                x = 0;
                y = (fbHeight - height) / 2;
            }
        }
    }
};

class GLRenderer final
{
public:
    //! \brief Create a new renderer
    //!
    //! If both width and height are <= 0, the renderer will use a
    //! size dependent on the monitor's resolution. Either one of
    //! width or height are <= 0, the renderer will choose the other
    //! based on the aspect ratio of the viewport. Otherwise, the
    //! renderer will use the specified width and height.
    //! 
    //! \param sprites The list of sprites to render
    //! \param options The options to create the renderer with
    //! 
    //! \return The renderer, or nullptr if an error occurred
    static GLRenderer *Create(const SpriteList *sprites, const Scratch3VMOptions &options);

    constexpr SDL_Window *GetWindow() const { return _window; }
       
    constexpr SpriteShader *GetSpriteShader() const { return _spriteShader; }

    constexpr int GetLogicalLeft() const { return _left; }
    constexpr int GetLogicalRight() const { return _right; }
    constexpr int GetLogicalBottom() const { return _bottom; }
    constexpr int GetLogicalTop() const { return _top; }
    constexpr const Vector2 &GetLogicalSize() const { return _logicalSize; }

    constexpr void ScreenToStage(int x, int y, int64_t *xout, int64_t *yout) const
    {
        // TODO: this will not work on retina displays
        x -= _viewport.x;
        y -= _viewport.y;

        *xout = (x - _viewport.width / 2) * (_right - _left) / _viewport.width;
        *yout = (_viewport.height / 2 - y) * (_top - _bottom) / _viewport.height;
    }

    constexpr void StageToScreen(int64_t x, int64_t y, int *xout, int *yout) const
    {
        // TODO: this will not work on retina displays
        *xout = x * _viewport.width / (_right - _left) + _viewport.width / 2 + _viewport.x;
        *yout = _viewport.height / 2 - y * _viewport.height / (_top - _bottom) + _viewport.y;
    }

    constexpr int GetWidth() const { return _width; }
    constexpr int GetHeight() const { return _height; }

    //! \brief Test whether a sprite is touching a color
    //!
    //! \param sprite ID of the sprite to test
    //! \param color The color to test
    //!
    //! \return Whether the sprite is touching the color
    bool TouchingColor(Sprite *sprite, const Vector3 &color);

    //! \brief Set the logical size of the renderer
    //!
    //! \param left The left side of the renderer
    //! \param right The right side of the renderer
    //! \param bottom The bottom side of the renderer
    //! \param top The top side of the renderer
    void SetLogicalSize(int left, int right, int bottom, int top);

    void BeginRender();
    void Render();
    void EndRender();

    //! \brief Call on window resize
    void Resize();

    constexpr int GetFrame() const { return _frame; }
    constexpr double GetTime() const { return _time; }
    constexpr double GetDeltaTime() const { return _deltaTime; }
    constexpr double GetFramerate() const { return _fps; }
    constexpr int GetObjectsDrawn() const { return _objectsDrawn; }

    GLRenderer &operator=(const GLRenderer &) = delete;
    GLRenderer &operator=(GLRenderer &&) = delete;

    GLRenderer(const GLRenderer &) = delete;
    GLRenderer(GLRenderer &&) = delete;
    ~GLRenderer();
private:
    GLRenderer();

    SDL_Window *_window;
    SDL_GLContext _gl;

    Scratch3VMOptions _options;

    int _left, _right;
    int _bottom, _top;
    Vector2 _logicalSize;

    GLViewport _viewport;

    // Window framebuffer size
    int _width, _height;
        
    Matrix4 _proj;

    int _frame;
    double _startTime;
    double _lastTime;
    double _time;
    double _deltaTime;
    double _fps;

    int _objectsDrawn;

    SpriteShader *_spriteShader;

    GLuint _quadVao, _quadVbo, _quadEbo;

    const SpriteList *_sprites;
};

bool CheckGLError();
