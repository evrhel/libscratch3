#include "sprite.hpp"

#include "../render/renderer.hpp"

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

bool Sprite::TouchingColor(GLRenderer *renderer, int64_t color) const
{
    if (!renderer)
        return true;

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

void Sprite::Validate()
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

        _transDirty = false;
    }
}

void Sprite::PrepareRender(GLRenderer *renderer)
{

}

bool Sprite::Load(Loader *loader, SpriteDef *def)
{
    return false;
}

Sprite::Sprite() {}

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
