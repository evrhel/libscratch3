#include "renderer.hpp"

#include <lysys/lysys.hpp>

#include "mesh.hpp"
#include "shader.hpp"

static Mesh *CreateQuad()
{
    // 0: position, 1: texcoord
    const Vector4 vertices[] = {
        Vector4(-0.5f, -0.5f, 0.0f, 0.0f),
        Vector4(0.5f, -0.5f, 1.0f, 0.0f),
        Vector4(0.5f, 0.5f, 1.0f, 1.0f),
        Vector4(-0.5f, 0.5f, 0.0f, 1.0f)
    };

    const uint32_t indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    Mesh *mesh = new Mesh();
    mesh->Load(vertices, 4, indices, 6);

    return mesh;
}

static char *ReadFile(const char *path, size_t *size)
{
    ls_handle fh;
    struct ls_stat st;

    fh = ls_open(path, LS_FILE_READ, LS_SHARE_READ, LS_OPEN_EXISTING);
    if (!fh)
        return nullptr;

    ls_fstat(fh, &st);
    
    char *buf = new char[st.size];
    *size = ls_read(fh, buf, st.size);

    ls_close(fh);

    return buf;
}

static SpriteShader *CreateSpriteShader()
{
    size_t vertexLength;
    char *vertexSource = ReadFile("shaders/sprite.vert", &vertexLength);

    size_t fragmentLength;
    char *fragmentSource = ReadFile("shaders/sprite.frag", &fragmentLength);

    SpriteShader *ss = new SpriteShader();
    ss->Load(vertexSource, vertexLength, fragmentSource, fragmentLength);

    delete[] fragmentSource;
    delete[] vertexSource;

    return ss;
}

void SpriteRenderInfo::Update(SpriteShader *ss)
{
    if (_dirty)
    {
        // recompute the model matrix

        _model = mutil::translate(Matrix4(), Vector3(_position, 0.0f));
        
        Quaternion q = mutil::rotateaxis(Vector3(0.0f, 0.0f, 1.0f), _rotation); 
        _model = _model * mutil::torotation(q);

        _dirty = false;
    }

    ss->SetModel(_model);
    ss->SetColorEffect(colorEffect);
    ss->SetBrightnessEffect(brightnessEffect);
    ss->SetFisheyeEffect(fisheyeEffect);
    ss->SetWhirlEffect(whirlEffect);
    ss->SetPixelateEffect(pixelateEffect);
    ss->SetMosaicEffect(mosaicEffect);
    ss->SetGhostEffect(ghostEffect);
    ss->SetTexture(texture);
    ss->SetColor(color);
}

SpriteRenderInfo::SpriteRenderInfo() :
    _rotation(0.0f),
    _dirty(true)
{
    colorEffect = 0.0f;
    brightnessEffect = 0.0f;
    fisheyeEffect = 0.0f;
    whirlEffect = 0.0f;
    pixelateEffect = 0.0f;
    mosaicEffect = 0.0f;
    ghostEffect = 0.0f;
    texture = 0;
    color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}

SpriteRenderInfo::~SpriteRenderInfo() { }

SpriteRenderInfo *GLRenderer::GetLayer(int64_t layer)
{
    if (layer == STAGE_LAYER)
        return _sprites;

    if (layer < 1 || layer > _spriteCount)
        return nullptr;

    return _sprites + layer;
}

void GLRenderer::MoveLayer(int64_t layer, int64_t distance)
{
    layer--; // layers are 1-indexed

    if (layer < 0 || layer >= _spriteCount)
        return;

    int64_t newLayer = layer + distance;
    if (newLayer < 0)
        newLayer = 0;
    else if (newLayer >= _spriteCount)
        newLayer = _spriteCount - 1;

    if (newLayer == layer)
        return;

    SpriteRenderInfo *sprites = _sprites + 1; // skip stage sprite

    SpriteRenderInfo tmp = sprites[layer];

    // shift layers
    if (newLayer < layer)
    {
        for (int64_t i = layer; i > newLayer; i--)
            sprites[i] = sprites[i - 1];
    }
    else
    {
        for (int64_t i = layer; i < newLayer; i++)
            sprites[i] = sprites[i + 1];
    }

    // insert
    sprites[newLayer] = tmp;
}

void GLRenderer::SetLogicalSize(int left, int right, int bottom, int top)
{
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;

    _proj = mutil::ortho(left, right, bottom, top, -1.0f, 1.0f);
}

void GLRenderer::Render()
{
    int width, height;
    SDL_GL_GetDrawableSize(_window, &width, &height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, width, height);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    _spriteShader->Use();
    _spriteShader->SetProj(_proj);

    // draw stage sperately
    _sprites[0].Update(_spriteShader);
    _quad->Render();

    // TODO: draw pen

    // draw sprites (+1 to skip stage sprite)
    SpriteRenderInfo *end = _sprites + _spriteCount + 1;
    for (SpriteRenderInfo *s = _sprites + 1; s < end; s++)
    {
        s->Update(_spriteShader);
        _quad->Render();
    }
}

GLRenderer::GLRenderer(int64_t spriteCount) :
    _window(nullptr),
    _context(nullptr),
    _left(0), _right(0),
    _bottom(0), _top(0),
    _quad(nullptr), _spriteShader(nullptr),
    _sprites(nullptr), _spriteCount(0)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    _window = SDL_CreateWindow("Scratch 3", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, VIEWPORT_WIDTH * 2, VIEWPORT_HEIGHT * 2,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!_window)
    {
        Cleanup();
        return;
    }

    _context = SDL_GL_CreateContext(_window);
    if (!_context)
    {
        Cleanup();
        return;
    }

    SDL_GL_MakeCurrent(_window, _context);
    
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        Cleanup();
        return;
    }

    _quad = CreateQuad();

    _spriteShader = CreateSpriteShader();

    _sprites = new SpriteRenderInfo[spriteCount + 1];
    _spriteCount = spriteCount;
}

GLRenderer::~GLRenderer()
{
    Cleanup();
}

void GLRenderer::Cleanup()
{
    if (_sprites)
        delete[] _sprites, _sprites = nullptr;

    if (_spriteShader)
        delete _spriteShader, _spriteShader = nullptr;

    if (_quad)
        delete _quad, _quad = nullptr;

    if (_context)
        SDL_GL_DeleteContext(_context), _context = nullptr;

    if (_window)
    {
        SDL_DestroyWindow(_window), _window = nullptr;
        SDL_Quit();
    }
}
