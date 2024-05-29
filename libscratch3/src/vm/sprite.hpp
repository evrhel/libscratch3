#pragma once

#include <string>
#include <vector>

#include <mutil/mutil.h>

#include "costume.hpp"

using namespace mutil;

class SpriteDef;
class Loader;
class GLRenderer;

struct AABB
{
    Vector2 lo, hi;
};

class Sprite
{
public:
    constexpr const std::string &GetName() const { return _name; }

    constexpr bool IsShown() const { return _shown; }
    constexpr void SetShown(bool shown) { _shown = shown; }

    constexpr double GetX() const { return _x; }
    constexpr double GetY() const { return _y; }
    constexpr void SetXY(double x, double y) { _x = x, _y = y, _transDirty = true; }

    constexpr double GetSize() const { return _size; }
    constexpr void SetSize(double size) { _size = size, _transDirty = true; }

    constexpr double GetDirection() const { return _direction; }
    constexpr void SetDirection(double direction) { _direction = direction, _transDirty = true; }

    constexpr int64_t GetCostume() const { return _costume; }
    constexpr void SetCostume(int64_t costume)
    {
        // clamp costume number
        if (costume < 1)
            costume = 1;
        else if (costume > _nCostumes)
            costume = _nCostumes;

        if (_costume != costume)
        {
            _costume = costume;
            _transDirty = true;
        }
    }

    constexpr double GetColorEffect() const { return _colorEffect; }
    constexpr void SetColorEffect(double colorEffect) { _colorEffect = colorEffect, _effectDirty = true; }

    constexpr double GetBrightnessEffect() const { return _brightnessEffect; }
    constexpr void SetBrightnessEffect(double brightnessEffect) { _brightnessEffect = brightnessEffect, _effectDirty = true; }

    constexpr double GetFisheyeEffect() const { return _fisheyeEffect; }
    constexpr void SetFisheyeEffect(double fisheyeEffect) { _fisheyeEffect = fisheyeEffect, _effectDirty = true; }

    constexpr double GetWhirlEffect() const { return _whirlEffect; }
    constexpr void SetWhirlEffect(double whirlEffect) { _whirlEffect = whirlEffect, _effectDirty = true; }
    
    constexpr double GetPixelateEffect() const { return _pixelateEffect; }
    constexpr void SetPixelateEffect(double pixelateEffect) { _pixelateEffect = pixelateEffect, _effectDirty = true; }

    constexpr double GetMosaicEffect() const { return _mosaicEffect; }
    constexpr void SetMosaicEffect(double mosaicEffect) { _mosaicEffect = mosaicEffect, _effectDirty = true; }

    constexpr double GetGhostEffect() const { return _ghostEffect; }
    constexpr void SetGhostEffect(double ghostEffect) { _ghostEffect = ghostEffect, _effectDirty = true; }

    bool TouchingColor(GLRenderer *renderer, int64_t color) const;

    bool TouchingSprite(const Sprite *sprite) const;

    bool TouchingPoint(const Vector2 &point) const;

    void Validate();

    void PrepareRender(GLRenderer *renderer);

    bool Load(Loader *loader, SpriteDef *def);

    Sprite();
    ~Sprite();
private:
    std::string _name;

    bool _shown = true;

    double _x = 0.0, _y = 0.0;
    double _size = 100.0;
    double _direction = 90.0;
    
    int64_t _costume = 1;
    Costume *_costumes = nullptr;
    int64_t _nCostumes = 0;

    bool _transDirty = true;
    intptr_t _drawable = -1; // drawable sprite

    double _colorEffect = 0.0;
    double _brightnessEffect = 0.0;
    double _fisheyeEffect = 0.0;
    double _whirlEffect = 0.0;
    double _pixelateEffect = 0.0;
    double _mosaicEffect = 0.0;
    double _ghostEffect = 0.0;
    bool _effectDirty = true;

    Matrix4 _model, _invModel;
    AABB _bbox;

    bool CheckSpriteAdv(const Sprite *sprite) const;
    bool CheckPointAdv(const Vector2 &point) const;

    void Cleanup();
};
