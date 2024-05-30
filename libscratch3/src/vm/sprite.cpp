#include "sprite.hpp"

#include "../render/renderer.hpp"
#include "../ast/ast.hpp"

// create AABB at the origin with the given size
static constexpr AABB &CenterAABB(AABB &self, const Vector2 &size)
{
    Vector2 half = size / 2.0f;
    self.lo = -half;
    self.hi = half;
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
    if (!_renderer)
        return;
    _renderer->SetLayer(_drawable, layer);
}

void Sprite::MoveLayer(int64_t amount)
{
    if (!_renderer)
        return;
    _renderer->MoveLayer(_drawable, amount);
}

bool Sprite::TouchingColor(int64_t color) const
{
    if (!_renderer)
        return true;
        
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
    return CheckPointAdv(point);
}

void Sprite::Update()
{
    if (_transDirty)
    {
        Costume *c = _costumes + _costume - 1;

        const Vector2 &cCenter = c->GetLogicalCenter();
        const Vector2 cSize = Vector2(c->GetLogicalSize());

        // setup matrices
        Matrix4 scale = mutil::scale(Matrix4(), Vector3(static_cast<float>(_size / 100)));
        Matrix4 transPos = mutil::translate(Matrix4(), Vector3(_x, _y, 0.0f));
        Quaternion q = mutil::rotateaxis(Vector3(0.0f, 0.0f, 1.0f), mutil::radians(_direction));
        Matrix4 transCenter = mutil::translate(Matrix4(), Vector3(-cCenter, 0.0f));

        // compute model matrix
        _model = transPos * mutil::torotation(q) * transCenter * scale;
        _invModel = mutil::inverse(_model);

        // compute bounding box
        ApplyTransformation(CenterAABB(_bbox, cSize), _model);

        if (_renderer)
        {
            // update sprite render info
            SpriteRenderInfo *s = _renderer->GetRenderInfo(_drawable);
            s->model = _model;
            s->colorEffect = clamp(mod(static_cast<float>(_colorEffect), 200.0f), 0.0f, 1.0f);
            s->brightnessEffect = clamp(static_cast<float>(_brightnessEffect / 100.0), 0.0f, 1.0f);
            s->fisheyeEffect = clamp(static_cast<float>(_fisheyeEffect / 100.0), 0.0f, 1.0f);
            s->whirlEffect = clamp(static_cast<float>(_whirlEffect / 100.0), 0.0f, 1.0f);
            s->pixelateEffect = clamp(static_cast<float>(_pixelateEffect / 100.0), 0.0f, 1.0f);
            s->mosaicEffect = clamp(static_cast<float>(_mosaicEffect / 100.0), 0.0f, 1.0f);
            s->ghostEffect = clamp(static_cast<float>(_ghostEffect / 100.0), 0.0f, 1.0f);
            s->texture = c->GetTexture();
            s->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        }

        _transDirty = false;
    }
}

void Sprite::InitGraphics(Loader *loader, GLRenderer *renderer)
{
    _renderer = renderer;

    _x = _node->x;
    _y = _node->y;
    _direction = _node->direction;
    _costume = _node->currentCostume;
    _size = _node->size;
    _volume = _node->volume;

    if (_node->costumes)
    {
        _nCostumes = _node->costumes->costumes.size();
        _costumes = new Costume[_nCostumes];

        size_t i = 0;
        for (AutoRelease<CostumeDef> &cd : _node->costumes->costumes)
        {
            Costume &c = _costumes[i++];
            c.Load(loader, *cd);
        }
    }
    
    SetLayer(_node->layer);
}

Sprite::Sprite(SpriteDef *def) :
    _name(def->name), _isStage(def->isStage),
    _node(def) { }

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
    if (_costumes)
    {
        delete[] _costumes, _costumes = nullptr;
        _nCostumes = 0;
    }

    _name.clear();
}
