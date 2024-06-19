#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <mutil/mutil.h>

#include "costume.hpp"
#include "sound.hpp"
#include "script.hpp"
#include "preload.hpp"
#include "memory.hpp"

using namespace mutil;

class SpriteDef;
class Loader;
class GLRenderer;
class VirtualMachine;

struct AABB
{
    Vector2 lo, hi;
};

struct GlideInfo
{
	double x0 = 0.0, y0 = 0.0; // Source glide position
	double x1 = 0.0, y1 = 0.0; // Target glide position
	double start = -1.0, end = 0.0; // Start and end times
};

#define MESSAGE_STATE_NONE 0
#define MESSAGE_STATE_SAY 1
#define MESSAGE_STATE_THINK 2

class Sprite
{
public:
    constexpr const String *GetName() const { return _name.u.string; }
    constexpr const char *GetNameString() const { return _name.u.string->str; }

    constexpr bool IsStage() const { return _isStage; }
    
    constexpr const String *GetMessage() const { return _message.u.string; }
    constexpr int GetMessageState() const { return _messageState; }
    void SetMessage(const Value &message, int state);

    constexpr GlideInfo *GetGlide() { return &_glide; }

    constexpr bool IsShown() const { return _shown; }
    constexpr void SetShown(bool shown) { _shown = shown; }

    constexpr double GetX() const { return _x; }
    constexpr void SetX(double x) { _x = x, _transDirty = true; }
    constexpr double GetY() const { return _y; }
    constexpr void SetY(double y) { _y = y, _transDirty = true; }
    constexpr void SetXY(double x, double y) { _x = x, _y = y, _transDirty = true; }

    constexpr void InvalidateTransform() { _transDirty = true; }

    void SetLayer(int64_t layer);
    void MoveLayer(int64_t amount);

    constexpr double GetSize() const { return _size; }
    constexpr void SetSize(double size) { _size = size, _transDirty = true; }

    constexpr double GetDirection() const { return _direction; }
    inline void SetDirection(double direction)
    {
        // direction is in the range of [-180, 180]
        _direction = fmod(direction + 180.0, 360.0) - 180.0;
        _transDirty = true;
    }

    constexpr RotationStyle GetRotationStyle() const { return _rotationStyle; }
    constexpr void SetRotationStyle(RotationStyle rotationStyle) { _rotationStyle = rotationStyle, _transDirty = true; }

    constexpr int64_t GetCostume() const { return _costume; }
    constexpr const Value &GetCostumeName() const { return _costumes[_costume - 1].GetNameValue(); }
    constexpr void SetCostume(int64_t costume)
    {
        if (!_nCostumes)
            return; // no costumes, prevent division by zero

        // wrap around
        costume = (costume - 1) % _nCostumes + 1;

        if (_costume != costume)
        {
            _costume = costume;
            _transDirty = true;
        }
    }

    void SetCostume(const String *name);

    Sound *FindSound(int64_t sound);
    Sound *FindSound(const String *name);
    constexpr Sound *GetSounds() const { return _sounds; }
    constexpr int64_t GetSoundCount() const { return _nSounds; }

    constexpr int64_t CostumeCount() const { return _nCostumes; }

    constexpr intptr_t GetDrawable() const { return _drawable; }

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

    constexpr DSPController *GetDSP() { return &_dsp; }

    bool TouchingColor(int64_t color) const;
    bool TouchingSprite(const Sprite *sprite) const;
    bool TouchingPoint(const Vector2 &point) const;

    // call from render thread
    void Update();

    // return next sprite data
    void Init(const SpriteInfo *info);

    // call from render thread
    void Load(VirtualMachine *vm);

    void DebugUI() const;

    constexpr const std::vector<Script *> &GetClickListeners() const { return _clickListeners; }

    constexpr const AABB &GetBoundingBox() const { return _bbox; }

    Sprite();
    ~Sprite();
private:
    uint8_t *_bytecode = nullptr;
    size_t _bytecodeSize = 0;

    uint8_t *_spriteData = nullptr, *_spriteEnd = nullptr;

    Value _name;
    bool _isStage = false;

    bool _shown = true;

    double _x = 0.0, _y = 0.0;
    double _size = 100.0;
    double _direction = 90.0;

    int64_t _layer = 0;

    bool _draggable = false;

    RotationStyle _rotationStyle = RotationStyle_AllAround;

    GlideInfo _glide;

	Value _message;
	int _messageState = MESSAGE_STATE_NONE;
    
    int64_t _costume = 1;
    Costume *_costumes = nullptr;
    int64_t _nCostumes = 0;

    std::unordered_map<const String *, int64_t, _StringHasher, _StringEqual> _costumeNameMap;

    Sound *_sounds = nullptr;
    int64_t _nSounds = 0;

    std::unordered_map<const String *, int64_t, _StringHasher, _StringEqual> _soundNameMap;

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

    DSPController _dsp;

    Matrix4 _model, _invModel;
    AABB _bbox;

    VirtualMachine *_vm = nullptr;

    std::vector<Script *> _clickListeners;

    bool CheckSpriteAdv(const Sprite *sprite) const;
    bool CheckPointAdv(const Vector2 &point) const;

    // call from render thread
    void Cleanup();
};
