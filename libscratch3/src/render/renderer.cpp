#include "renderer.hpp"

#include <lysys/lysys.hpp>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include "shader.hpp"

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
    char *vertexSource = ReadFile("../../libscratch3/shaders/sprite.vert", &vertexLength);

    size_t fragmentLength;
    char *fragmentSource = ReadFile("../../libscratch3/shaders/sprite.frag", &fragmentLength);

    SpriteShader *ss = new SpriteShader();
    ss->Load(vertexSource, vertexLength, fragmentSource, fragmentLength);

    delete[] fragmentSource;
    delete[] vertexSource;

    return ss;
}

void SpriteRenderInfo::Prepare(SpriteShader *ss)
{
    ss->SetModel(model);
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
    _layer(-1)
{
    shouldRender = false;
    model = Matrix4();
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

void GLRenderer::ScreenToStage(int x, int y, int64_t *xout, int64_t *yout) const
{
    int width, height;
    SDL_GetWindowSize(_window, &width, &height);

    *xout = (x - width / 2) * (_right - _left) / width;
    *yout = (height / 2 - y) * (_top - _bottom) / height;
}

void GLRenderer::StageToScreen(int64_t x, int64_t y, int *xout, int *yout) const
{
    int width, height;
	SDL_GetWindowSize(_window, &width, &height);

	*xout = x * width / (_right - _left) + width / 2;
	*yout = height / 2 - y * height / (_top - _bottom);
}

intptr_t GLRenderer::CreateSprite()
{
    for (int64_t i = 1; i < _spriteCount; i++)
    {
        SpriteRenderInfo *s = _sprites + i;
        if (s->_layer == -1)
        {            
            // find empty slot in render order
            for (int64_t j = 1; j < _spriteCount; j++)
            {
                if (_renderOrder[j] == -1)
                {
                    _renderOrder[j] = i;
                    s->_layer = j;
                    return i;
                }
            }

            // should never happen
            abort();
        }
    }

    return -1;
}

SpriteRenderInfo *GLRenderer::GetRenderInfo(intptr_t sprite)
{
    if (sprite > _spriteCount)
        return nullptr;
    return _sprites + sprite;
}

void GLRenderer::SetLayer(intptr_t sprite, int64_t layer)
{
    if (sprite <= SPRITE_STAGE || sprite >= _spriteCount || layer == 0)
        return;

    int64_t newLayer;
    if (layer < 0)
        newLayer = _spriteCount + layer + 1; // relative to back
    else
        newLayer = layer;
    
    if (newLayer < 1 || newLayer >= _spriteCount)
        return; // out of bounds

    SpriteRenderInfo *s = _sprites + sprite;
    if (s->_layer == newLayer)
        return; // already at target layer

    // iterators
    int64_t *end = _renderOrder + _spriteCount;
    int64_t *start = _renderOrder + s->_layer;
    int64_t *target = _renderOrder + newLayer;

    assert(target >= _renderOrder && target < _renderOrder + _spriteCount);

    // shift elements to make room for the sprite
    if (start < target)
    {
        for (int64_t *it = start; it < target; it++)
        {
            _sprites[*it]._layer--;
            *it = *(it + 1);
        }
    }
    else
    {
        for (int64_t *it = start; it > target; it--)
        {
            _sprites[*it]._layer++;
            *it = *(it - 1);
        }
    }

    // insert the sprite
    *target = sprite;
    s->_layer = newLayer;
}

void GLRenderer::MoveLayer(intptr_t sprite, int64_t direction)
{
    if (sprite <= SPRITE_STAGE || sprite >= _spriteCount)
        return;

    int64_t newLayer = _sprites[sprite]._layer + direction;

    if (newLayer < 1) newLayer = 1;
    else if (newLayer >= _spriteCount) newLayer = _spriteCount - 1;

    SetLayer(sprite, newLayer);
}

bool GLRenderer::TouchingColor(intptr_t sprite, const Vector3 &color)
{
    if (sprite < 1 || sprite >= _spriteCount)
        return false;

    // TODO: implement
    return true;
}

void GLRenderer::SetLogicalSize(int left, int right, int bottom, int top)
{
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;

    _proj = mutil::ortho(left, right, bottom, top, -1.0f, 1.0f);
}

void GLRenderer::BeginRender()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    int width, height;
    SDL_GL_GetDrawableSize(_window, &width, &height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_STENCIL_TEST);
    glDepthMask(GL_FALSE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, width, height);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLRenderer::Render()
{
    _spriteShader->Use();
    _spriteShader->SetProj(_proj);

    // draw stage sperately, pen is on top of stage, but below sprites
    _sprites[0].Prepare(_spriteShader);
    DrawQuad();

    // TODO: draw pen

    // draw sprites
    int64_t *end = _renderOrder + _spriteCount;
    for (int64_t *it = _renderOrder + 1; it < end; it++)
    {
        SpriteRenderInfo &s = _sprites[*it];
        if (s.shouldRender)
        {
            s.Prepare(_spriteShader);
            DrawQuad();
        }
    }
}

void GLRenderer::EndRender()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(_window);
}

void GLRenderer::Resize()
{
    int width, height;
    SDL_GL_GetDrawableSize(_window, &width, &height);

    double viewWidth = _right - _left;
    double viewHeight = _top - _bottom;
    _scale = std::max(width / viewWidth, height / viewHeight);
}

GLRenderer::GLRenderer(int64_t spriteCount) :
    _window(nullptr),
    _context(nullptr),
    _left(0), _right(0),
    _bottom(0), _top(0),
    _scale(0.0),
    _spriteShader(nullptr),
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

    if (SDL_GL_MakeCurrent(_window, _context) != 0)
    {
        Cleanup();
        return;
    }
    
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        Cleanup();
        return;
    }

    SDL_GL_SetSwapInterval(1);

    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(_window, _context);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    memset(&_quad, 0, sizeof(_quad));
    CreateQuad();

    _spriteShader = CreateSpriteShader();

    _spriteCount = spriteCount + 1; // +1 for stage
    _sprites = new SpriteRenderInfo[_spriteCount];
    _renderOrder = new int64_t[_spriteCount];

    // initialize render order
    for (int64_t i = 1; i < _spriteCount; i++)
        _renderOrder[i] = -1;

    // stage always at the bottom
    _renderOrder[0] = 0;
    _sprites[0]._layer = 0;

    SetLogicalSize(-VIEWPORT_WIDTH / 2, VIEWPORT_WIDTH / 2,
        -VIEWPORT_HEIGHT / 2, VIEWPORT_HEIGHT / 2);
    Resize();
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

    DestroyQuad();

    if (_context)
        SDL_GL_DeleteContext(_context), _context = nullptr;

    if (_window)
    {
        SDL_DestroyWindow(_window), _window = nullptr;
        SDL_Quit();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void GLRenderer::CreateQuad()
{
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

    glGenVertexArrays(1, &_quad.vao);
    glGenBuffers(1, &_quad.vbo);
    glGenBuffers(1, &_quad.ebo);

    glBindVertexArray(_quad.vao);

    glBindBuffer(GL_ARRAY_BUFFER, _quad.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // <vec2 position, vec2 texcoord>
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quad.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    _quad.indexCount = 6;
}

void GLRenderer::DrawQuad()
{
    glBindVertexArray(_quad.vao);
    glDrawElements(GL_TRIANGLES, _quad.indexCount, GL_UNSIGNED_INT, 0);
}

void GLRenderer::DestroyQuad()
{
    if (_quad.vao)
        glDeleteVertexArrays(1, &_quad.vao);

    if (_quad.vbo)
        glDeleteBuffers(1, &_quad.vbo);

    if (_quad.ebo)
        glDeleteBuffers(1, &_quad.ebo);

    memset(&_quad, 0, sizeof(_quad));
}