#include "sprite.hpp"

#include <imgui.h>

#include "vm.hpp"
#include "costume.hpp"
#include "sound.hpp"

#include "../render/renderer.hpp"
#include "../codegen/compiler.hpp"

// create AABB at the origin with extents [-0.5, 0.5]
static constexpr AABB &CenterAABB(AABB &self)
{
    self.lo = Vector2(-0.5f);
    self.hi = Vector2(0.5f);
    return self;
}

// create AABB from the given corners
static constexpr AABB &CornersToAABB(AABB &self, const Vector2 *corners)
{
    self.lo = Vector2(INFINITY);
    self.hi = Vector2(-INFINITY);

    for (int i = 0; i < 4; i++)
    {
        self.lo.x = mutil::min(self.lo.x, corners[i].x);
        self.lo.y = mutil::min(self.lo.y, corners[i].y);
        self.hi.x = mutil::max(self.hi.x, corners[i].x);
        self.hi.y = mutil::max(self.hi.y, corners[i].y);
    }

    return self;
}

// apply transformation to AABB
static inline AABB &ApplyTransformation(AABB &self, const Matrix4 &model)
{
    const Vector4 corners4[4] = {
        model * Vector4(self.lo.x, self.lo.y, 0.0f, 1.0f),
        model * Vector4(self.hi.x, self.lo.y, 0.0f, 1.0f),
        model * Vector4(self.hi.x, self.hi.y, 0.0f, 1.0f),
        model * Vector4(self.lo.x, self.hi.y, 0.0f, 1.0f)
    };

    const Vector2 corners[4] = {
        Vector2(corners4[0]) / corners4[0].w,
        Vector2(corners4[1]) / corners4[1].w,
        Vector2(corners4[2]) / corners4[2].w,
        Vector2(corners4[3]) / corners4[3].w
    };

    return CornersToAABB(self, corners);
}

// intersection of two AABBs
//
// no intersection if lo.x > hi.x or lo.y > hi.y
static constexpr AABB AABBIntersection(const AABB &a, const AABB &b)
{
    return AABB{
        Vector2(mutil::max(a.lo.x, b.lo.x), mutil::max(a.lo.y, b.lo.y)),
        Vector2(mutil::min(a.hi.x, b.hi.x), mutil::min(a.hi.y, b.hi.y))
    };
}

// check if AABB contains point
static constexpr bool AABBContains(const AABB &a, const Vector2 &p)
{
    return p.x >= a.lo.x && p.x <= a.hi.x &&
           p.y >= a.lo.y && p.y <= a.hi.y;
}

int64_t AbstractSprite::FindCostume(const String *name) const
{
    auto it = _costumeNameMap.find(name);
    if (it != _costumeNameMap.end())
        return it->second;
    return 0;
}

int64_t AbstractSprite::FindSound(const String *name) const
{
    auto it = _soundNameMap.find(name);
    if (it != _soundNameMap.end())
		return it->second;
    return 0;
}

Sprite *AbstractSprite::Instantiate(Sprite *tmpl)
{
    if (tmpl && tmpl->GetBase() != this)
        VM->Panic("Template sprite does not match base sprite");

    Sprite *inst = Alloc();

    if (tmpl)
    {
        // Copy properties from template
        inst->_visible = tmpl->_visible;
        inst->_x = tmpl->_x;
        inst->_y = tmpl->_y;
        inst->_size = tmpl->_size;
        inst->_direction = tmpl->_direction;
        inst->_draggable = tmpl->_draggable;
        inst->_rotationStyle = tmpl->_rotationStyle;
        inst->_costume = tmpl->_costume;
        inst->_dsp = tmpl->_dsp;
        inst->_gec = tmpl->_gec;

        // Copy fields from template
        for (uint32_t i = 0; i < _nFields; i++)
            Assign(inst->_fields[i], tmpl->_fields[i]);
    }
    else
    {
        // Copy properties from sprite info
        inst->_visible = _info->visible;
        inst->_x = _info->x;
        inst->_y = _info->y;
        inst->_size = _info->size;
        inst->_direction = _info->direction;
        inst->_draggable = _info->draggable;
        inst->_rotationStyle = (RotationStyle)_info->rotationStyle;
        inst->_costume = _info->currentCostume;
        // No initializers for GEC and DSP

        // TODO: read initial field values from sprite info
    }

    inst->InvalidateTransform();

    // Add to sprite list
    SpriteList *spriteList = VM->GetSpriteList();
    if (tmpl)
        spriteList->Insert(tmpl->GetPrev(), inst); // insert before template
    else
        spriteList->Add(inst);

    return inst;
}

bool AbstractSprite::Init(uint8_t *bytecode, size_t bytecodeSize, const bc::Sprite *info, bool stream)
{
    // Set basic properties
    SetString(_name, (char *)(bytecode + info->name));
    if (_name.type != ValueType_String)
        return false;
    _info = info;
    
    // Allocate field
    _nFields = info->numFields;

    // Allocate costumes
    _nCostumes = info->numCostumes;
    _costumes = new Costume[_nCostumes];
        
    // Initialize costumes
    bc::Costume *costumes = (bc::Costume *)(bytecode + info->costumes);
    for (int64_t i = 0; i < _nCostumes; i++)
    {
        if (!_costumes[i].Init(bytecode, bytecodeSize, &costumes[i], stream))
        {
            Cleanup();
            return false;
        }

        _costumeNameMap[_costumes[i].GetName()] = i + 1;
    }

    // Allocate sounds
    _nSounds = info->numSounds;
    _sounds = new AbstractSound[_nSounds];

    // Initialize sounds
    bc::Sound *sounds = (bc::Sound *)(bytecode + info->sounds);
    for (int64_t i = 0; i < _nSounds; i++)
    {
        if (!_sounds[i].Init(bytecode, bytecodeSize, &sounds[i], stream))
        {
            Cleanup();
            return false;
        }

		_soundNameMap[_sounds[i].GetName()] = i;
	}

    _spriteSize = offsetof(Sprite, _fields);
    _spriteSize += _nFields * sizeof(Value);
    _spriteSize += _nSounds * sizeof(Voice);

    _nInstances = 0;

    // Allocate sprite pool
    _pool = (uint8_t *)malloc(_spriteSize * MAX_INSTANCES);
    if (!_pool)
    {
        Cleanup();
        return false;
    }

    // Causes all Value instances to be zero-initialized
    // effectively calling InitializeValue on all fields,
    // so we don't have to do it manually when allocating
    memset(_pool, 0, _spriteSize * MAX_INSTANCES);

    return true;
}

void AbstractSprite::Load()
{    
    uint8_t *bytecode = VM->GetBytecode();

    // find all listeners
    for (const SCRIPT_ALLOC_INFO &ai : VM->GetScriptStubs())
    {
        if (ai.sprite->GetBase() != this)
            continue;

        Opcode entry = (Opcode)*(bytecode + ai.info->offset);
        if (entry == Op_onclick)
            _clickListeners.push_back(VM->AllocScript(ai));
        else if (entry == Op_onclone)
            _cloneEntry.push_back(ai.info);
    }

    for (int64_t i = 0; i < _nCostumes; i++)
        _costumes[i].Load();

    for (int64_t i = 0; i < _nSounds; i++)
        _sounds[i].Load();
}

void AbstractSprite::DebugUI() const
{
    ImGui::SeparatorText("Costumes");
    constexpr float kImageHeight = 64;
    for (int64_t id = 1; id <= _nCostumes; id++)
    {
        Costume *c = _costumes + id - 1;
        const IntVector2 &size = c->GetSize();

        ImGui::Text("[%d]: '%s' (%s), origin: (%.2f, %.2f), size: %dx%d",
            (int)id,
            c->GetNameString(),
            c->IsBitmap() ? "bitmap" : "vector",
            (double)c->GetCenter().x,
            (double)c->GetCenter().y,
            (int)c->GetSize().x,
            (int)c->GetSize().y);

        float aspect = (float)size.x / size.y;
        float imageWidth = kImageHeight * aspect;

        float scale = kImageHeight / size.y;

        void *tex = (void *)(intptr_t)c->GetTexture(Vector2(scale));
        if (tex)
        {
            ImGui::Image(tex, ImVec2(imageWidth, kImageHeight), ImVec2(0, 1), ImVec2(1, 0));
            if (ImGui::IsItemHovered())
            {
                if (ImGui::BeginTooltip())
                {
                    constexpr float height = kImageHeight * 2;
                    float width = imageWidth * 2;
                    scale = height / size.y;

                    tex = (void *)(intptr_t)c->GetTexture(Vector2(scale));

                    ImGui::Image(tex, ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0));
                    ImGui::EndTooltip();
                }
            }
        }
        else
            ImGui::Text("(unloaded)");
    }

    ImGui::SeparatorText("Sounds");
    for (int64_t i = 0; i < _nSounds; i++)
    {
        AbstractSound &s = _sounds[i];

        ImGui::Text("[%lld]: '%s', rate: %d, length: %.2f",
			i+1, s.GetName()->str, s.GetSampleRate(),
            s.GetDuration());
    }
}

Sprite *AbstractSprite::Alloc()
{
    if (_nInstances >= MAX_INSTANCES)
        VM->Panic("Too many instances");

    uint8_t *end = _pool + (_spriteSize * MAX_INSTANCES);
    for (uint8_t *p = _pool; p < end; p += _spriteSize)
    {
        Sprite *s = (Sprite *)p;
        if (!s->IsAllocated())
        {
            s->_base = this;
            s->_instanceId = ((p - _pool) / _spriteSize) + 1;
            s->_delete = false;

            s->_dsp.ClearEffects();
            s->_gec.ClearEffects();

            s->_next = nullptr;
            s->_prev = nullptr;

            Voice *voices = s->GetVoices();
            for (size_t i = 0; i < _nSounds; i++)
                voices[i].Init(_sounds + i, &s->_dsp);

            _nInstances++;

            return s;
        }
    }

    abort(); // unreachable
}

void AbstractSprite::Free(Sprite *sprite)
{
    assert(sprite->_base == this);
    assert(sprite->IsAllocated());

    // Release all resources

    Voice *voices = sprite->GetVoices();
    for (size_t i = 0; i < _nSounds; i++)
        voices[i].Release();

    for (uint32_t i = 0; i < _nFields; i++)
        ReleaseValue(sprite->_fields[i]);
    ReleaseValue(sprite->_message);

    sprite->_prev = nullptr;
    sprite->_next = nullptr;

    sprite->_delete = false;
    sprite->_instanceId = UNALLOCATED_INSTANCE_ID;
    sprite->_base = nullptr;

    _nInstances--;
}

AbstractSprite::AbstractSprite() :
    _info(nullptr),
    _costumes(nullptr), _nCostumes(0),
    _sounds(nullptr), _nSounds(0),
    _nFields(0),
    _pool(nullptr), _spriteSize(0), _nInstances(0)
{
    InitializeValue(_name);
}

AbstractSprite::~AbstractSprite()
{
    Cleanup();
}

void AbstractSprite::Cleanup()
{
    if (_sounds)
        delete[] _sounds, _sounds = nullptr;

    if (_costumes)
        delete[] _costumes, _costumes = nullptr;

    if (_pool)
    {
        uint8_t *end = _pool + (_spriteSize * MAX_INSTANCES);
        for (uint8_t *p = _pool; p < end; p += _spriteSize)
        {
            Sprite *s = (Sprite *)p;
            if (s->IsAllocated())
                Free(s);
        }

        free(_pool), _pool = nullptr;
    }
}

void Sprite::SetMessage(const Value &message, bool think)
{
    Assign(_message, message);
    _isThinking = think;
}

void Sprite::Update()
{
    if (VM->GetTime() < _glide.end)
    {
        // linear interpolation
        double t = (VM->GetTime() - _glide.start) / (_glide.end - _glide.start);
        double x = _glide.x0 + t * (_glide.x1 - _glide.x0);
        double y = _glide.y0 + t * (_glide.y1 - _glide.y0);
        SetXY(x, y);
    }

    if (_transDirty)
    {
        Costume *c = _base->GetCostume(_costume);

        const Vector2 &logicalCenter = c->GetLogicalCenter();
        const Vector2 &logicalSize = c->GetLogicalSize();

        Vector2 center = logicalSize / 2.0f;
        Vector2 centerOffset = logicalCenter - center;

        float unifScale = static_cast<float>(_size / 100);
        Vector2 size = logicalSize * unifScale;

        // Determine actual rotation
        float rotation;
        if (_rotationStyle == RotationStyle_DontRotate)
            rotation = 0.0f;
        else if (_rotationStyle == RotationStyle_LeftRight)
        {
            if (_direction < 0)
                rotation = MUTIL_PI;
            else
                rotation = 0.0f;
        }
        else
            rotation = radians(static_cast<float>(_direction - 90.0));

        // Setup transformation matrices
        Matrix4 mScale = scale(Matrix4(), Vector3(size, 1.0f));
        Matrix4 mTransPos = translate(Matrix4(), Vector3(_x, _y, 0.0f));
        Quaternion q = rotateaxis(Vector3(0.0f, 0.0f, 1.0f), rotation);
        Matrix4 mTransCenter = translate(Matrix4(), Vector3(-centerOffset.x, centerOffset.y, 0.0f));

        // Compute model matrix
        _model = mTransPos * torotation(q) * mTransCenter * mScale;

        // Update bounding box
        ApplyTransformation(CenterAABB(_bbox), _model);

        if (_visible)
        {
            // GetTexture may trigger a texture load, only want to do this
            // if the sprite is visible

            GLRenderer *render = VM->GetRenderer();

            Vector2 fbSize(render->GetWidth(), render->GetHeight());
            const Vector2 &viewportSize = render->GetLogicalSize();

            Vector2 texScale = unifScale * fbSize / viewportSize;
            _texture = c->GetTexture(texScale);
        }

        _transDirty = false;
    }
}

// check against costume collision mask
static inline bool CheckPointExp(const Sprite *s, const Vector2 &point)
{
    Costume *c = s->GetCostume();

    // normalize point to costume size
    const AABB &bbox = s->GetBoundingBox();
    Vector2 N = (point - bbox.lo) / (bbox.hi - bbox.lo);
    N.y = 1.0f - N.y; // flip Y axis

    // scale to costume size and check collision
    const IntVector2 &size = c->GetSize();
    return c->CheckCollision(N.x * size.x, N.y * size.y);
}

bool Sprite::TouchingPoint(const Vector2 &point) const
{
    if (!_visible)
        return false;

    if (_gec.GetGhostEffect() >= 100)
		return false; // invisible

    // fast check
    if (!AABBContains(_bbox, point))
        return false;

    return CheckPointExp(this, point);
}

bool Sprite::TouchingSprite(const Sprite *sprite) const
{
    if (!_visible || !sprite->_visible)
        return false;

    if (_gec.GetGhostEffect() >= 100 || sprite->_gec.GetGhostEffect() >= 100)
        return false; // invisible

    const AABB I = AABBIntersection(_bbox, sprite->_bbox);
    if (I.lo.x > I.hi.x || I.lo.y > I.hi.y)
		return false; // no intersection

    const Vector2 size = I.hi - I.lo;
    if (size.x < 1 || size.y < 1)
		return false; // too small

    // iterate over the intersection area
    Vector2 P;
    for (P.y = I.lo.y; P.y < I.hi.y; P.y++)
    {
        for (P.x = I.lo.x; P.x < I.hi.x; P.x++)
        {            
            // check if point is inside both sprites
            if (CheckPointExp(this, P) && CheckPointExp(sprite, P))
                return true;
        }
    }

    return false;
}

Value &Sprite::GetField(uint32_t id)
{
    assert(IsAllocated() && !_delete);

    if (id >= _base->GetFieldCount())
        VM->Panic("Invalid field ID");
    return _fields[id];
}

void Sprite::DebugUI() const
{
    ImGui::SeparatorText("Transform");
    ImGui::LabelText("Position", "%.0f, %.0f", _x, _y);
    ImGui::LabelText("Direction", "%.0f", _direction);
    ImGui::LabelText("Size", "%.0f%%", _size);
    ImGui::LabelText("Bounding Box", "(%.0f, %.0f) (%.0f, %.0f), size: %.0fx%.0f",
        (double)_bbox.lo.x, (double)_bbox.lo.y, (double)_bbox.hi.x, (double)_bbox.hi.y,
        (double)(_bbox.hi.x - _bbox.lo.x), (double)(_bbox.hi.y - _bbox.lo.y));

    ImGui::SeparatorText("Graphics");
    ImGui::LabelText("Visible", "%s", _visible ? "true" : "false");
    ImGui::LabelText("Costume", "%lld/%lld (%s)", _costume,
        _base->CostumeCount(), _base->GetCostume(_costume)->GetNameString());
    ImGui::LabelText("Color", "%.0f (%.2f)", _gec.GetColorEffect(), (double)_gec.GetColorFactor());
    ImGui::LabelText("Brightness", "%.0f (%.2f)", _gec.GetBrightnessEffect(), (double)_gec.GetBrightnessFactor());
    ImGui::LabelText("Fisheye", "%.0f (%.2f)", _gec.GetFisheyeEffect(), (double)_gec.GetFisheyeFactor());
    ImGui::LabelText("Whirl", "%.0f (%.2f)", _gec.GetWhirlEffect(), (double)_gec.GetWhirlFactor());
    ImGui::LabelText("Pixelate", "%.0f (%.2f)", _gec.GetPixelateEffect(), (double)_gec.GetPixelateFactor());
    ImGui::LabelText("Mosaic", "%.0f (%.2f)", _gec.GetMosaicEffect(), (double)_gec.GetMosaicFactor());
    ImGui::LabelText("Ghost", "%.0f (%.2f)", _gec.GetGhostEffect(), (double)_gec.GetGhostFactor());

    ImGui::SeparatorText("Sound");
    ImGui::LabelText("Volume", "%.0f%%", _dsp.GetVolume());
    ImGui::LabelText("Pitch", "%.0f (%+.0f semitones, ratio %.2f)", _dsp.GetPitch(), _dsp.GetPitch() / 10, _dsp.GetResampleRatio());
    ImGui::LabelText("Pan", "%.0f", _dsp.GetPan());

    ImGui::SeparatorText("Voices");
    Voice *voices = GetVoices();
    for (int64_t i = 0; i < _base->GetSoundCount(); i++)
    {
        Voice &v = voices[i];
        AbstractSound *as = v.GetSound();

        const char *name = as->GetName()->str;
        if (v.IsPlaying())
            ImGui::Text("[%lld]: '%s' (not playing)", i+1, name);
        else
        {
            unsigned long pos = v.GetStreamPos();
            unsigned long size = as->GetFrameCount();
            int rate = as->GetSampleRate();

            double duration = (double)size / rate;
            double location = duration * pos / size;

            int dMin = static_cast<int>(duration / 60);
            int dSec = duration - dMin * 60;
            int lMin = static_cast<int>(location / 60);
            int lSec = location - lMin * 60;

            ImGui::Text("[%lld]: '%s' %d:%02d/%d:%02d",
                i+1, name, lMin, lSec, dMin, dSec);
        }
    }
}

Sprite *Sprite::Clone()
{
    assert(IsAllocated() && !_delete);

    Sprite *clone = _base->Instantiate(this);

    SCRIPT_ALLOC_INFO ai;
    ai.sprite = clone;

    for (bc::Script *ce : _base->GetCloneEntry())
    {
        ai.info = ce;

        Script *script = VM->AllocScript(ai);
        VM->RestartScript(script);
    }

    VM->Reschedule();

    return clone;
}

void Sprite::Destroy()
{
    assert(IsAllocated() && !_delete);

    _delete = true;

    Script *current = VM->GetCurrentScript();
    if (current && current->sprite == this)
        VM->TerminateScript(current);
}
