#include "sprite.hpp"

#include <imgui.h>

#include "../render/renderer.hpp"
#include "../codegen/compiler.hpp"

#include "vm.hpp"

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

// check if two AABBs intersect
static constexpr bool AABBIntersect(const AABB &a, const AABB &b)
{
    return a.lo.x <= b.hi.x && a.hi.x >= b.lo.x &&
           a.lo.y <= b.hi.y && a.hi.y >= b.lo.y;
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

Sound *AbstractSprite::FindSound(int64_t sound)
{
    if (sound < 0 || sound >= _nSounds)
		return nullptr;
	return _sounds + sound;
}

Sound *AbstractSprite::FindSound(const String *name)
{
    auto it = _soundNameMap.find(name);
    if (it != _soundNameMap.end())
		return FindSound(it->second);
    return nullptr;
}

void AbstractSprite::Init(uint8_t *bytecode, size_t bytecodeSize, const bc::Sprite *info, bool stream)
{
    // Set basic properties
    SetString(_name, (char *)(bytecode + info->name));
    _layer = info->layer;
    _isStage = info->isStage;
    _draggable = info->draggable;

    // Allocate costumes
    _nCostumes = info->numCostumes;
    _costumes = new Costume[_nCostumes];
        
    // Initialize costumes
    bc::Costume *costumes = (bc::Costume *)(bytecode + info->costumes);
    for (int64_t i = 0; i < _nCostumes; i++)
    {
        _costumes[i].Init(bytecode, bytecodeSize, &costumes[i], stream);
        _costumeNameMap[_costumes[i].GetName()] = i + 1;
    }

    // Allocate sounds
    _nSounds = info->numSounds;
    _sounds = new Sound[_nSounds];

    // Initialize sounds
    bc::Sound *sounds = (bc::Sound *)(bytecode + info->sounds);
    for (int64_t i = 0; i < _nSounds; i++)
    {
		_sounds[i].Init(bytecode, bytecodeSize, &sounds[i], stream, &_dsp);
		_soundNameMap[_sounds[i].GetName()] = i;
	}
}

void AbstractSprite::Load(VirtualMachine *vm)
{
    if (_vm)
        return; // already loaded

    _vm = vm;
    
    // find all click listeners
    for (const Script &script : vm->GetScripts())
    {
        if (script.sprite != this)
            continue;

        if (*script.entry == Op_onclick)
			_clickListeners.push_back((Script *)&script);
    }

    Loader *loader = vm->GetLoader();
    GLRenderer *render = vm->GetRenderer();

    if (!_isStage)
    {
        _drawable = render->CreateSprite();
        SetLayer(_layer);
    }
    else
        _drawable = SPRITE_STAGE;

    for (int64_t i = 0; i < _nCostumes; i++)
        _costumes[i].Load();

    for (int64_t i = 0; i < _nSounds; i++)
        _sounds[i].Load();

    SpriteRenderInfo *ri = render->GetRenderInfo(_drawable);
    ri->userData = this;

    // initial update
    Update();
}

void AbstractSprite::DebugUI() const
{
    GLRenderer *render = _vm->GetRenderer();

    ImGui::SeparatorText("Transform");
    ImGui::LabelText("Position", "%.0f, %.0f", _x, _y);
    ImGui::LabelText("Direction", "%.0f", _direction);
    ImGui::LabelText("Size", "%.0f%%", _size);
    ImGui::LabelText("Draw Order", "%lld", render->GetRenderInfo(_drawable)->GetLayer());
    ImGui::LabelText("Bounding Box", "(%.0f, %.0f) (%.0f, %.0f), size: %.0fx%.0f",
        		(double)_bbox.lo.x, (double)_bbox.lo.y, (double)_bbox.hi.x, (double)_bbox.hi.y,
                (double)(_bbox.hi.x - _bbox.lo.x), (double)(_bbox.hi.y - _bbox.lo.y));

    ImGui::SeparatorText("Graphics");
    ImGui::LabelText("Visible", "%s", _shown ? "true" : "false");
    ImGui::LabelText("Costume", "%d/%d (%s)", (int)_costume, (int)_nCostumes, _costumes[_costume - 1].GetNameString());
    ImGui::LabelText("Color", "%.0f", _colorEffect);
    ImGui::LabelText("Brightness", "%.0f", _brightnessEffect);
    ImGui::LabelText("Fisheye", "%.0f", _fisheyeEffect);
    ImGui::LabelText("Whirl", "%.0f", _whirlEffect);
    ImGui::LabelText("Pixelate", "%.0f", _pixelateEffect);
    ImGui::LabelText("Mosaic", "%.0f", _mosaicEffect);
    ImGui::LabelText("Ghost", "%.0f", _ghostEffect);

    ImGui::SeparatorText("Sound");
    ImGui::LabelText("Volume", "%.0f%%", _dsp.GetVolume());
    ImGui::LabelText("Pitch", "%.0f (%+.0f semitones, ratio %.2f)", _dsp.GetPitch(), _dsp.GetPitch() / 10, _dsp.GetResampleRatio());
    ImGui::LabelText("Pan", "%.0f", _dsp.GetPan());

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
        Sound &s = _sounds[i];

        ImGui::Text("[%d]: '%s', rate: %d, length: %.2f, playing: %s",
			(int)i+1,
			s.GetNameString(),
            s.GetSampleRate(),
            s.GetDuration(),
            s.IsPlaying() ? "true" : "false");
    }
}

AbstractSprite::AbstractSprite()
{
    InitializeValue(_name);
}

AbstractSprite::~AbstractSprite()
{
    Cleanup();
}

void AbstractSprite::Cleanup()
{
    _clickListeners.clear();

    if (_sounds)
    {
        delete[] _sounds, _sounds = nullptr;
        _nSounds = 0;
    }

    if (_costumes)
    {
        delete[] _costumes, _costumes = nullptr;
        _nCostumes = 0;
    }

    _drawable = -1;

    _vm = nullptr;

    _messageState = MessageState_None;
    ReleaseValue(_message);

    ReleaseValue(_name);
}
