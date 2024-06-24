#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <mutil/mutil.h>

#include "costume.hpp"
#include "sound.hpp"
#include "script.hpp"
#include "memory.hpp"

#include "../codegen/util.hpp"

#define BASE_SPRITE_ID 0

using namespace mutil;

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

class InstantiatedSprite;

class AbstractSprite
{
public:
    constexpr const String *GetName() const { return _name.u.string; }
    constexpr const char *GetNameString() const { return _name.u.string->str; }

    constexpr bool IsStage() const { return _isStage; }

    int64_t FindCostume(const String *name) const;
    constexpr int64_t CostumeCount() const { return _nCostumes; }

    Sound *FindSound(int64_t sound);
    Sound *FindSound(const String *name);

    constexpr Sound *GetSounds() const { return _sounds; }
    constexpr int64_t GetSoundCount() const { return _nSounds; }

    constexpr VirtualMachine *GetVM() const { return _vm; }

    //! \brief Initialize the sprite
    //!
    //! Sets up basic information about the sprite, such as the name
    //! and the initial position and size.
    //!
    //! \param bytecode The program bytecode.
    //! \param bytecodeSize The size of the bytecode.
    //! \param info The information about the sprite, as loaded from
    //! the bytecode
    //! \param stream Whether the sprite and its resources are streamed
    void Init(uint8_t *bytecode, size_t bytecodeSize, const bc::Sprite *info, bool stream);

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

    AbstractSprite();
    ~AbstractSprite();
private:
    Value _name; // sprite name, always a string
    bool _isStage = false;
        
    Costume *_costumes = nullptr;
    int64_t _nCostumes = 0;

    // costume name -> index
    std::unordered_map<const String *, int64_t, _StringHasher, _StringEqual> _costumeNameMap;

    Sound *_sounds = nullptr;
    int64_t _nSounds = 0;

    // sound name -> index
    std::unordered_map<const String *, int64_t, _StringHasher, _StringEqual> _soundNameMap;

    VirtualMachine *_vm = nullptr;

    // scripts started when the sprite is clicked
    std::vector<Script *> _clickListeners;

    // entry point for clones of this sprite
    std::vector<Script *> _cloneListeners;

    uint64_t _nextID = 0;

    void Cleanup();

    friend class InstantiatedSprite;
};

class Sprite
{
public:
    constexpr AbstractSprite *GetBase() const { return _base; }
    constexpr uint64_t GetID() const { return _id; }

    Sprite *Clone();
    void Destroy();

    Sprite();
    ~Sprite();
private:
    AbstractSprite *_base;
    uint64_t _id;

    //////////////////////////////////////////////////////////////////////////
    // Sprite properties

    double _x, _y;
    double _size;
    double _direction;

    int64_t _layer;

    bool _draggable;

    RotationStyle _rotationStyle;

    int64_t _costume;

    bool _transDirty;
    
    //////////////////////////////////////////////////////////////////////////
    // Graphics effects

    double _colorEffect;
    double _brightnessEffect;
    double _fisheyeEffect;
    double _whirlEffect;
    double _pixelateEffect;
    double _mosaicEffect;
    double _ghostEffect;
    bool _effectDirty;

    //////////////////////////////////////////////////////////////////////////
    // Audio effects

    DSPController _dsp;

    //////////////////////////////////////////////////////////////////////////
    // Misc

    GlideInfo _glide;

    Value _message;
    MessageState _messageState;

    //////////////////////////////////////////////////////////////////////////
    // Rendering

    Matrix4 _model, _invModel;
    AABB _bbox;

    Sprite *_prev, *_next; // layering
};
