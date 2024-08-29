#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <mutil/mutil.h>

#include "script.hpp"
#include "memory.hpp"
#include "sound.hpp"
#include "costume.hpp"

#include "../codegen/util.hpp"
#include "../render/renderer.hpp"

#define UNALLOCATED_INSTANCE_ID 0
#define BASE_INSTANCE_ID 1

#define MAX_INSTANCES 256

using namespace mutil;

class VirtualMachine;
class AbstractSprite;

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

class AbstractSprite final
{
public:
    constexpr const String *GetName() const { return _name.u.string; }
    constexpr const char *GetNameString() const { return _name.u.string->str; }

    constexpr const bc::Sprite *GetInfo() const { return _info; }

    constexpr Costume *GetCostume(int64_t id) const
    {
        if (id < 1 || id > _nCostumes)
            return nullptr;
        return _costumes + id - 1;
    }

    int64_t FindCostume(const String *name) const;

    constexpr Costume *GetCostumes() const { return _costumes; }
    constexpr int64_t CostumeCount() const { return _nCostumes; }

    constexpr AbstractSound *GetSound(int64_t id) const
    {
        if (id < 1 || id > _nSounds)
            return nullptr;
        return _sounds + id - 1;
    }

    int64_t FindSound(const String *name) const;

    constexpr AbstractSound *GetSounds() const { return _sounds; }
    constexpr int64_t GetSoundCount() const { return _nSounds; }

    //! \brief Create a new instance of this sprite
    //!
    //! The instantiated sprite is added to the virtual machine's
    //! sprite list, so it is not the responsibility of the caller
    //! to manage the sprite's memory. Doing so results in undefined
    //! behavior.
    //! 
    //! \param tmpl The template sprite to use, or nullptr to create
    //! a new sprite from the the initial state. If not nullptr, the
    //! sprite must be an instance of this sprite.
    //! 
    //! \return The new sprite. Panics if the sprite could not be
    //! created.
    Sprite *Instantiate(Sprite *tmpl);

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
    //!
    //! \return Whether the sprite was successfully initialized
    bool Init(uint8_t *bytecode, size_t bytecodeSize, const bc::Sprite *info, bool stream);

    //! \brief Load the sprite data
    //!
    //! Loads any necessary data for the sprite, such as the costumes
    //! and sounds. Must be called from the render thread. The sprite
    //! will be tied to the given virtual machine.
    void Load();

    //! \brief Render debug information about the sprite
    void DebugUI() const;

    constexpr const size_t GetFieldCount() const { return _nFields; }

    constexpr const std::vector<Script *> &GetClickListeners() const { return _clickListeners; }
    constexpr const std::vector<bc::Script *> &GetCloneEntry() const { return _cloneEntry; }

    Sprite *Alloc();
    void Free(Sprite *sprite);

    constexpr size_t GetSpriteSize() const { return _spriteSize; }
    constexpr size_t GetInstanceCount() const { return _nInstances; }

    AbstractSprite();
    ~AbstractSprite();
private:
    Value _name; // sprite name, always a string
    const bc::Sprite *_info;

    Costume *_costumes;
    int64_t _nCostumes;
    StringMap<int64_t> _costumeNameMap; // costume name -> index

    AbstractSound *_sounds;
    int64_t _nSounds;
    StringMap<int64_t> _soundNameMap;  // sound name -> index

    size_t _nFields;

    std::vector<Script *> _clickListeners;
    std::vector<bc::Script *> _cloneEntry;

    ////////////////////////////////////////////////////////////////
    // Pool allocator for this sprite

    uint8_t *_pool; // Pool of sprites (size _spriteSize * MAX_INSTANCES)
    size_t _spriteSize; // Size of each sprite
    size_t _nInstances; // Number of instances

    void Cleanup();
};

struct Sprite final
{
public:
    static void *operator new(size_t) = delete;
    static void operator delete(void *) = delete;

    constexpr AbstractSprite *GetBase() const { return _base; }
    constexpr uint32_t GetInstanceId() const { return _instanceId; }
    constexpr bool IsDeleted() const { return _delete; }
    constexpr bool IsAllocated() const { return _instanceId != UNALLOCATED_INSTANCE_ID; }

    constexpr bool IsVisible() const { return _visible; }
    constexpr double GetX() const { return _x; }
    constexpr double GetY() const { return _y; }
    constexpr double GetSize() const { return _size; }
    constexpr double GetDirection() const { return _direction; }
    constexpr bool IsDraggable() const { return _draggable; }
    constexpr RotationStyle GetRotationStyle() const { return _rotationStyle; }
    constexpr int64_t GetCostumeIndex() const { return _costume; }
    constexpr Costume *GetCostume() const { return _base->GetCostume(_costume); }

    constexpr void SetVisible(bool visible) { _visible = visible; _transDirty = true; }

    constexpr void SetX(double x)
    {
        _x = x;
        if (_x < -240)
            _x = -240;
        else if (_x > 240)
            _x = 240;
        _transDirty = true;
    }

    constexpr void SetY(double y)
    {
        _y = y;
        if (_y < -180)
            _y = -180;
        else if (_y > 180)
            _y = 180;
        _transDirty = true;
    }

    constexpr void SetXY(double x, double y)
    {
        _x = x;
        if (_x < -240)
            _x = -240;
        else if (_x > 240)
            _x = 240;

        _y = y;
        if (_y < -180)
            _y = -180;
        else if (_y > 180)
            _y = 180;
      
        _transDirty = true;
    }

    constexpr void SetSize(double size) { _size = size; _transDirty = true; }
    constexpr void SetDirection(double direction) { _direction = direction; _transDirty = true; }
    constexpr void SetDraggable(bool draggable) { _draggable = draggable; }
    constexpr void SetRotationStyle(RotationStyle rotationStyle) { _rotationStyle = rotationStyle; }

    constexpr void SetCostume(const int64_t costume)
    {
        const int64_t newCostume = (costume - 1) % _base->CostumeCount() + 1;
        if (_costume != newCostume)
        {
            _costume = newCostume;
            _transDirty = true;
        }
    }

    constexpr void InvalidateTransform() { _transDirty = true; }

    constexpr DSPController &GetDSP() { return _dsp; }
    constexpr const DSPController &GetDSP() const { return _dsp; }

    constexpr GlideInfo &GetGlideInfo() { return _glide; }
    constexpr const GlideInfo &GetGlideInfo() const { return _glide; }

    constexpr const Value &GetMessage() const { return _message; }
    constexpr bool IsThinking() const { return _isThinking; }

    void SetMessage(const Value &message, bool think);

    void Update();

    bool TouchingPoint(const Vector2 &point);
    bool TouchingSprite(const Sprite *sprite);

    constexpr const Matrix4 &GetModel() const { return _model; }
    constexpr const Matrix4 &GetInvModel() const { return _invModel; }
    constexpr const AABB &GetBoundingBox() const { return _bbox; }

    constexpr GraphicEffectController &GetGraphicEffects() { return _gec; }
    constexpr const GraphicEffectController &GetGraphicEffects() const { return _gec; }
    constexpr GLuint GetTexture() const { return _texture; }

    constexpr Sprite *GetNext() const { return _next; }
    constexpr Sprite *GetPrev() const { return _prev; }

    Value &GetField(uint32_t id);

    constexpr const Value *GetFields() const { return _fields; }

    void DebugUI() const;

    //! \brief Clone this sprite
    //! 
    //! Creates a clone of this sprite and schedules the clone's
    //! scripts to run. The clone will be placed one layer below
    //! the original sprite.
    //! 
    //! \return The clone
    Sprite *Clone();

    //! \brief Destroy this sprite
    //! 
    //! Any scripts the sprite is running will be terminated. If
    //! the sprite is destroying itself (i.e. from within a script),
    //! this function does not return.
    //! 
    //! \param vm The virtual machine
    void Destroy();

    constexpr Voice *GetVoices() const
    {
        const size_t offset = offsetof(Sprite, _fields) + (_base->GetFieldCount() * sizeof(Value));
        return (Voice *)((uint8_t *)this + offset);
    }

    constexpr size_t GetVoiceCount() const
    {
        return _base->GetSoundCount();
    }

    constexpr Voice *GetVoice(const uint32_t id) const
    {
        if (id >= GetVoiceCount())
			return nullptr;
		return GetVoices() + id;
	}

    Sprite &operator=(const Sprite &) = delete;
    Sprite &operator=(Sprite &&) = delete;

    Sprite() = delete;
    Sprite(const Sprite &) = delete;
    Sprite(Sprite &&) = delete;
    ~Sprite() = delete;
private:
    AbstractSprite *_base;
    uint32_t _instanceId;
    bool _delete; // Whether the sprite is scheduled for deletion

    //////////////////////////////////////////////////////////////////////////
    // Sprite properties

    bool _visible;

    double _x, _y;
    double _size;
    double _direction;

    bool _draggable;

    RotationStyle _rotationStyle;

    int64_t _costume;

    bool _transDirty;
    
    //////////////////////////////////////////////////////////////////////////
    // Audio

    DSPController _dsp;

    //////////////////////////////////////////////////////////////////////////
    // Misc

    GlideInfo _glide;

    Value _message; // None = no message, otherwise the message
    bool _isThinking; // false = saying, true = thinking

    //////////////////////////////////////////////////////////////////////////
    // Rendering

    Matrix4 _model, _invModel;
    AABB _bbox;

    GraphicEffectController _gec;

    GLuint _texture;

    Sprite *_next, *_prev;

    //////////////////////////////////////////////////////////////////////////
    // Fields

    Value _fields[]; // Fields (size = _base->GetFieldCount())

    // ...

    // Voice _voices[]; // Voices (size = _base->GetSoundCount())


    friend class SpriteList;
    friend class AbstractSprite;
    friend class VirtualMachine;
};

