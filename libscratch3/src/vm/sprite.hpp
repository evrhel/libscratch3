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

//! \brief Axis-aligned bounding box
struct AABB
{
    Vector2 lo, hi;
};

//! \brief Stores information about how a sprite should glide
struct GlideInfo
{
	double x0 = 0.0, y0 = 0.0; // Source glide position
	double x1 = 0.0, y1 = 0.0; // Target glide position
	double start = -1.0, end = 0.0; // Start and end times
};

enum MessageState
{
    MessageState_None, // nothing
    MessageState_Say, // speech bubble
    MessageState_Think // thought bubble
};

class Sprite
{
public:
    constexpr const String *GetName() const { return _name.u.string; }
    constexpr const char *GetNameString() const { return _name.u.string->str; }

    constexpr bool IsStage() const { return _isStage; }
    
    constexpr const String *GetMessage() const { return _message.u.string; }
    constexpr int GetMessageState() const { return _messageState; }

    //! \brief Set the message to display
    //!
    //! \param message The message to display, None to clear the message
    //! \param state The type of message to display
    void SetMessage(const Value &message, MessageState state);

    constexpr GlideInfo *GetGlide() { return &_glide; }

    //! \brief Show or hide the sprite
    //!
    //! \param shown Whether to show the sprite
    constexpr void SetShown(bool shown) { _shown = shown; }
    constexpr bool IsShown() const { return _shown; }

    constexpr void SetX(double x) { _x = x, _transDirty = true; }
    constexpr void SetY(double y) { _y = y, _transDirty = true; }
    constexpr void SetXY(double x, double y) { _x = x, _y = y, _transDirty = true; }

    constexpr double GetX() const { return _x; }
    constexpr double GetY() const { return _y; }

    //! \brief Invalidate the transformation matrix
    //!
    //! This is automatically called when the sprite's position or
    //! size is changed but may be called manually if necessary.
    constexpr void InvalidateTransform() { _transDirty = true; }

    //! \brief Set the Sprite's render layer
    //!
    //! \param layer The layer to set, higher layers are rendered on top
    //! of lower layers
    void SetLayer(int64_t layer);

    //! \brief Move the Sprite's render layer
    //!
    //! \param amount The amount to move the layer by, positive means
    //! towards the front, negative means towards the back
    void MoveLayer(int64_t amount);

    //! \brief Set the size of the sprite
    //!
    //! \param size The size of the sprite, in percent
    constexpr void SetSize(double size) { _size = size, _transDirty = true; }
    constexpr double GetSize() const { return _size; }

    //! \brief Set the direction of the sprite
    //!
    //! The direction is stored in degrees, with 90 degrees being the
    //! default direction (right). The direction is clamped to the
    //! range of [-180, 180].
    //!
    //! \param direction The direction of the sprite, in degrees
    inline void SetDirection(double direction)
    {
        // direction is in the range of [-180, 180]
        _direction = fmod(direction + 180.0, 360.0) - 180.0;
        _transDirty = true;
    }

    constexpr double GetDirection() const { return _direction; }

    constexpr void SetRotationStyle(RotationStyle rotationStyle) { _rotationStyle = rotationStyle, _transDirty = true; }
    constexpr RotationStyle GetRotationStyle() const { return _rotationStyle; }

    constexpr int64_t GetCostume() const { return _costume; }
    constexpr const Value &GetCostumeName() const { return _costumes[_costume - 1].GetNameValue(); }

    //! \brief Set the costume of the sprite by index
    //!
    //! \param costume The index of the costume to set (1-based)
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

    //! \brief Set the costume of the sprite by name
    //!
    //! Does nothing if the costume does not exist.
    //!
    //! \param name The name of the costume to set
    void SetCostume(const String *name);

    //! \brief Find a costume by index
    //!
    //! \param costume The index of the costume to find (0-based)
    //!
    //! \return The costume, or nullptr if the costume does not exist
    Sound *FindSound(int64_t sound);

    //! \brief Find a costume by name
    //!
    //! \param name The name of the costume to find
    //!
    //! \return The costume, or nullptr if the costume does not exist
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

    //! \brief Get the DSP controller for the sprite
    //!
    //! \return The DSP controller
    constexpr DSPController *GetDSP() { return &_dsp; }

    bool TouchingColor(int64_t color) const;
    bool TouchingSprite(const Sprite *sprite) const;
    bool TouchingPoint(const Vector2 &point) const;

    //! \brief Update the sprite
    //!
    //! Updates the sprite's transformation matrix if necessary and
    //! communicates with the renderer to update any graphical effects.
    void Update();

    //! \brief Initialize the sprite
    //!
    //! Sets up basic information about the sprite, such as the name
    //! and the initial position and size.
    //!
    //! \param info The information about the sprite, as loaded from
    //! the bytecode
    void Init(const SpriteInfo *info);

    //! \brief Load the sprite data
    //!
    //! Loads any necessary data for the sprite, such as the costumes
    //! and sounds. Must be called from the render thread. The sprite
    //! will be tied to the given virtual machine.
    //!
    //! \param vm The virtual machine to use for loading
    void Load(VirtualMachine *vm);

    //! \brief Render debug information about the sprite
    void DebugUI() const;

    constexpr const std::vector<Script *> &GetClickListeners() const { return _clickListeners; }

    //! \brief Get the sprite's bounding box
    //!
    //! \return The sprite's bounding box
    constexpr const AABB &GetBoundingBox() const { return _bbox; }

    Sprite();
    ~Sprite();
private:
    uint8_t *_bytecode = nullptr; // the bytecode for the project
    size_t _bytecodeSize = 0; // the size of the bytecode

    // this sprite's data in the bytecode
    uint8_t *_spriteData = nullptr, *_spriteEnd = nullptr;

    Value _name; // sprite name, always a string
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
    MessageState _messageState = MessageState_None;
    
    int64_t _costume = 1;
    Costume *_costumes = nullptr;
    int64_t _nCostumes = 0;

    // costume name -> index
    std::unordered_map<const String *, int64_t, _StringHasher, _StringEqual> _costumeNameMap;

    Sound *_sounds = nullptr;
    int64_t _nSounds = 0;

    // sound name -> index
    std::unordered_map<const String *, int64_t, _StringHasher, _StringEqual> _soundNameMap;

    bool _transDirty = true;
    intptr_t _drawable = -1; // drawable sprite

    // graphical effects
    double _colorEffect = 0.0;
    double _brightnessEffect = 0.0;
    double _fisheyeEffect = 0.0;
    double _whirlEffect = 0.0;
    double _pixelateEffect = 0.0;
    double _mosaicEffect = 0.0;
    double _ghostEffect = 0.0;
    bool _effectDirty = true;

    DSPController _dsp; // sound effect controller

    Matrix4 _model, _invModel; // transformation matrices
    AABB _bbox; // bounding box

    VirtualMachine *_vm = nullptr;

    // scripts started when the sprite is clicked
    std::vector<Script *> _clickListeners;

    //! \brief Check if this sprite is touching another sprite
    //!
    //! This performs an advanced check that takes into account the
    //! precise shapes of the sprites and is very expensive. Should
    //! only be called after a preliminary bounding box check.
    //!
    //! \param sprite The sprite to check
    //!
    //! \return Whether the sprites are touching
    bool CheckSpriteAdv(const Sprite *sprite) const;

    //! \brief Check if this sprite is touching a point
    //!
    //! This performs an advanced check that takes into account the
    //! precise shape of the sprite and is very expensive. Should
    //! only be called after a preliminary bounding box check.
    //!
    //! \param point The point to check
    //!
    //! \return Whether the sprite is touching the point
    bool CheckPointAdv(const Vector2 &point) const;

    // call from render thread
    void Cleanup();
};
