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

void Sprite::SetLayer(int64_t layer)
{
    _vm->GetRenderer()->SetLayer(_drawable, layer);
}

void Sprite::MoveLayer(int64_t amount)
{
    _vm->GetRenderer()->MoveLayer(_drawable, amount);
}

void Sprite::SetCostume(const std::string &name)
{
    auto it = _costumeNames.find(name);
    if (it != _costumeNames.end())
        SetCostume(it->second);
}

bool Sprite::TouchingColor(int64_t color) const
{        
    // TODO: implement
    return true;
}

bool Sprite::TouchingSprite(const Sprite *sprite) const
{
    if (!_shown || !sprite->_shown)
        return false;

    // fast AABB check
    if (!AABBIntersect(_bbox, sprite->_bbox))
        return false;
    return CheckSpriteAdv(sprite);
}

bool Sprite::TouchingPoint(const Vector2 &point) const
{
    if (!_shown)
        return false;

    // fast AABB check
    if (!AABBContains(_bbox, point))
        return false;

    return true;
    //return CheckPointAdv(point);
}

void Sprite::Update()
{
    GLRenderer *render = _vm->GetRenderer();
    SpriteRenderInfo *s = render->GetRenderInfo(_drawable);
    s->shouldRender = _shown;

    double uniformScale = _size / 100;
    Costume *c = _costumes + _costume - 1;

    if (_vm->GetTime() < _glide.end)
    {
        // linear interpolation
        double t = (_vm->GetTime() - _glide.start) / (_glide.end - _glide.start);
        double x = _glide.x0 + t * (_glide.x1 - _glide.x0);
        double y = _glide.y0 + t * (_glide.y1 - _glide.y0);
        SetXY(x, y);
    }

    if (_transDirty)
    {
        const Vector2 &cCenter = c->GetLogicalCenter();
        const Vector2 cSize = Vector2(c->GetLogicalSize());

        Vector2 size = cSize * static_cast<float>(uniformScale);

        float rotation;
        if (_rotationStyle == RotationStyle_DontRotate)
            rotation = 0.0f;
        else if (_rotationStyle == RotationStyle_LeftRight)
        {
            if (_direction < 0)
                rotation = 180.0f;
			else
				rotation = 0.0f;
        }
		else
			rotation = static_cast<float>(_direction - 90.0);
        rotation = mutil::radians(rotation);

        // setup matrices

        Matrix4 scale = mutil::scale(Matrix4(), Vector3(size, 1.0f));
        Matrix4 transPos = mutil::translate(Matrix4(), Vector3(_x, _y, 0.0f));
        Quaternion q = mutil::rotateaxis(Vector3(0.0f, 0.0f, 1.0f), rotation);
        Matrix4 transCenter = mutil::translate(Matrix4(), Vector3(-cCenter, 0.0f));

        // compute model matrix
        _model = transPos * mutil::torotation(q) * transCenter * scale;
        _invModel = mutil::inverse(_model);

        // compute bounding box
        ApplyTransformation(CenterAABB(_bbox), _model);

        // update sprite render info
        s->model = _model;
        s->texture = c->GetTexture();

        _transDirty = false;
    }

    if (_effectDirty)
    {
        s->colorEffect = mutil::mod(static_cast<float>(_colorEffect / 200), 1.0f);
        s->brightnessEffect = clamp(static_cast<float>(_brightnessEffect / 100), -1.0f, 1.0f);
        s->fisheyeEffect = mutil::max(0.0f, static_cast<float>(_fisheyeEffect + 100) / 100.0f);
        s->whirlEffect = -_whirlEffect * MUTIL_PI / 180.0f;
        s->pixelateEffect = mutil::abs(static_cast<float>(_pixelateEffect)) / 10;
        s->mosaicEffect = mutil::clamp(mutil::round(mutil::abs(static_cast<float>(_mosaicEffect) + 10) / 10), 1.0f, 512.0f);
        s->ghostEffect = clamp(static_cast<float>(_ghostEffect / 100), 0.0f, 1.0f);
        s->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        _effectDirty = false;
    }

    c->Render(uniformScale, render->GetScale());
}

void Sprite::Init(const SpriteInfo *info)
{
    _name = info->name;
    _x = info->x;
    _y = info->y;
    _size = info->size;
    _direction = info->direction;
    _costume = info->currentCostume;
    _layer = info->layer;
    _shown = info->visible;
    _isStage = info->isStage;
    _draggable = info->draggable;
    _rotationStyle = info->rotationStyle;

    _nCostumes = info->costumes.size();
    _costumes = new Costume[_nCostumes];
    for (int64_t i = 0; i < _nCostumes; i++)
    {
        _costumes[i].Init(&info->costumes[i]);
		_costumeNames[_costumes[i].GetName()] = i + 1;
    }
}

void Sprite::Load(VirtualMachine *vm)
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

    SpriteRenderInfo *ri = render->GetRenderInfo(_drawable);
    ri->userData = this;

    // initial update
    Update();
}

void Sprite::DebugUI() const
{
    GLRenderer *render = _vm->GetRenderer();

    ImGui::SeparatorText("Transform");
    ImGui::LabelText("Position", "%.0f, %.0f", _x, _y);
    ImGui::LabelText("Direction", "%.0f", _direction);
    ImGui::LabelText("Size", "%.0f%%", _size);
    ImGui::LabelText("Draw Order", "%d", render->GetRenderInfo(_drawable)->GetLayer());
    ImGui::LabelText("Bounding Box", "(%.0f, %.0f) (%.0f, %.0f), size: %.0fx%.0f",
        		(double)_bbox.lo.x, (double)_bbox.lo.y, (double)_bbox.hi.x, (double)_bbox.hi.y,
                (double)(_bbox.hi.x - _bbox.lo.x), (double)(_bbox.hi.y - _bbox.lo.y));

    ImGui::SeparatorText("Graphics");
    ImGui::LabelText("Visible", "%s", _shown ? "true" : "false");
    ImGui::LabelText("Costume", "%d/%d (%s)", (int)_costume, (int)_nCostumes, _costumes[_costume - 1].GetName().c_str());
    ImGui::LabelText("Color", "%.0f", _colorEffect);
    ImGui::LabelText("Brightness", "%.0f", _brightnessEffect);
    ImGui::LabelText("Fisheye", "%.0f", _fisheyeEffect);
    ImGui::LabelText("Whirl", "%.0f", _whirlEffect);
    ImGui::LabelText("Pixelate", "%.0f", _pixelateEffect);
    ImGui::LabelText("Mosaic", "%.0f", _mosaicEffect);
    ImGui::LabelText("Ghost", "%.0f", _ghostEffect);

    ImGui::SeparatorText("Sound");
    ImGui::LabelText("Volume", "%.0f%%", _volume);

    ImGui::SeparatorText("Costumes");
    constexpr float kImageHeight = 64;
    for (int64_t id = 1; id <= _nCostumes; id++)
    {
        Costume *c = _costumes + id - 1;
        ImGui::Text("[%d]: '%s' (%s), origin: (%.2f, %.2f), size: %dx%d, rendered: %dx%d",
            (int)id,
            c->GetName().c_str(),
            c->IsBitmap() ? "bitmap" : "vector",
            (double)c->GetLogicalCenter().x,
            (double)c->GetLogicalCenter().y,
            (int)c->GetLogicalSize().x,
            (int)c->GetLogicalSize().y,
            (int)c->GetTextureWidth(),
            (int)c->GetTextureHeight());

        float aspect = (float)c->GetTextureWidth() / c->GetTextureHeight();
        float imageWidth = kImageHeight * aspect;

        void *tex = (void *)(intptr_t)c->GetTexture();
        if (tex)
        {
            ImGui::Image((void *)(intptr_t)c->GetTexture(), ImVec2(imageWidth, kImageHeight), ImVec2(0, 1), ImVec2(1, 0));
            if (ImGui::IsItemHovered())
            {
                if (ImGui::BeginTooltip())
                {
                    ImGui::Image((void *)(intptr_t)c->GetTexture(), ImVec2(c->GetLogicalSize().x, c->GetLogicalSize().y), ImVec2(0, 1), ImVec2(1, 0));
                    ImGui::EndTooltip();
                }
            }
        }
        else
            ImGui::Text("(unloaded)");
    }
}

Sprite::Sprite() { }

Sprite::~Sprite()
{
    Cleanup();
}

bool Sprite::CheckSpriteAdv(const Sprite *sprite) const
{
    Costume *sc = sprite->_costumes + sprite->_costume - 1;
    
    // brute force check
    const IntVector2 &size = sc->GetLogicalSize();
    for (int32_t x = 0; x < size.x; x++)
    {
        for (int32_t y = 0; y < size.y; y++)
        {
            // check if a collision is possible
            if (!sc->TestCollision(x, y))
                continue;

            // convert sprite costume pixel location to world space
            Vector4 world4 = sprite->_model * Vector4(x, y, 0.0f, 1.0f);
            Vector2 world = Vector2(world4) / world4.w;

            // check if the point is inside this sprite
            if (CheckPointAdv(world))
                return true;
        }
    }

    return false;
}

bool Sprite::CheckPointAdv(const Vector2 &point) const
{
    // convert to local space
    Vector4 pos4 = _invModel * Vector4(point, 0.0f, 1.0f);
    Vector2 pos = Vector2(pos4) / pos4.w;

    Costume *c = _costumes + _costume - 1;
    return c->TestCollision(static_cast<int32_t>(pos.x), static_cast<int32_t>(pos.y));
}

void Sprite::Cleanup()
{
    _clickListeners.clear();

    if (_costumes)
    {
        delete[] _costumes, _costumes = nullptr;
        _nCostumes = 0;
    }

    _drawable = -1;

    _vm = nullptr;

    _messageState = MESSAGE_STATE_NONE;
    _message.clear();

    _name.clear();
}
