#include "renderer.hpp"

#include <lysys/lysys.hpp>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>

#include "shader.hpp"
#include "../vm/sprite.hpp"
#include "../vm/vm.hpp"

namespace // shaders
{
#include <shaders/sprite.vert.h>
#include <shaders/sprite.frag.h>
}

static SpriteShader *CreateSpriteShader()
{
    SpriteShader *ss = new SpriteShader();
    bool r = ss->Load(
        sprite_vert_source,
        sizeof(sprite_vert_source) - 1,
        sprite_frag_source,
        sizeof(sprite_frag_source) - 1);

    if (!r)
    {
        delete ss;
        return nullptr;
    }

    return ss;
}

//! \brief Prepare the sprite shader for rendering
//! 
//! \param sprite The sprite to render
//! \param ss The sprite shader
//! 
//! \return true if the sprite should be rendered
static inline bool PrepareSprite(const Sprite *sprite, SpriteShader *ss)
{
    if (!sprite->IsVisible())
        return false;

    GLuint tex = sprite->GetTexture();
    if (!tex)
        return false;

    const GraphicEffectController &gec = sprite->GetGraphicEffects();

    ss->SetModel(sprite->GetModel());
    ss->SetColorEffect(gec.GetColorFactor());
    ss->SetBrightnessEffect(gec.GetBrightnessFactor());
    ss->SetFisheyeEffect(gec.GetFisheyeFactor());
    ss->SetWhirlEffect(gec.GetWhirlFactor());
    ss->SetPixelateEffect(gec.GetPixelateFactor());
    ss->SetMosaicEffect(gec.GetMosaicFactor());
    ss->SetGhostEffect(gec.GetGhostFactor());
    ss->SetTexture(tex);
    ss->SetColor(Vector4(1.0f));

    return true;
}

static bool CreateQuad(GLuint *vao, GLuint *vbo, GLuint *ebo)
{
    static const Vector4 vertices[] = {
        Vector4(-0.5f, -0.5f, 0.0f, 0.0f),
        Vector4(0.5f, -0.5f, 1.0f, 0.0f),
        Vector4(0.5f, 0.5f, 1.0f, 1.0f),
        Vector4(-0.5f, 0.5f, 0.0f, 1.0f)
    };

    static const uint8_t indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    glGenBuffers(1, ebo);

    glBindVertexArray(*vao);

    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // <vec2 position, vec2 texcoord>
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    if (!CheckGLError())
    {
		glDeleteVertexArrays(1, vao);
		glDeleteBuffers(1, vbo);
		glDeleteBuffers(1, ebo);
		return false;
    }

    return true;
}

static void DetectResolution(const Scratch3VMOptions &options, int *const width, int *const height)
{
    if (options.width <= 0 && options.height > 0)
    {
        *height = options.height;
        *width = options.height * VIEWPORT_WIDTH / VIEWPORT_HEIGHT;
    }
    else if (options.width > 0 && options.height <= 0)
    {
        *width = options.width;
        *height = options.width * VIEWPORT_HEIGHT / VIEWPORT_WIDTH;
    }
    else if (options.width > 0 && options.height > 0)
    {
        *width = options.width;
        *height = options.height;
    }
    else
    {
        SDL_DisplayMode displayMode;
        if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0)
        {
            if (options.fullscreen)
            {
                *width = displayMode.w;
                *height = displayMode.h;
            }
            else
            {
                *height = displayMode.h * 2 / 3;
                *width = *height * VIEWPORT_WIDTH / VIEWPORT_HEIGHT;
            }
        }
        else
        {
            *width = VIEWPORT_WIDTH;
            *height = VIEWPORT_HEIGHT;
        }
    }
}

static Uint32 DetectWindowFlags(const Scratch3VMOptions &options)
{
    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;

    if (options.fullscreen)
    {
        if (options.borderless)
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        else
            flags |= SDL_WINDOW_FULLSCREEN;
    }
    else if (options.borderless)
        flags |= SDL_WINDOW_BORDERLESS;

#if LS_DARWIN
    flags |= SDL_WINDOW_ALLOW_HIGHDPI; // Retina display
#endif // LS_DARWIN

    return flags;
}

static SDL_Window *CreateWindow(const Scratch3VMOptions &options)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Determine window size
    int width, height;
    DetectResolution(options, &width, &height);

    // Determine window flags
    Uint32 flags = DetectWindowFlags(options);

    // Create window
    SDL_Window *window = SDL_CreateWindow("Scratch 3", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, width, height, flags);
    if (!window)
    {
#if LS_DEBUG
        printf("CreateWindow: SDL_CreateWindow failed: %s\n", SDL_GetError());
#endif // LS_DEBUG
        return nullptr;
    }

    return window;
}

static SDL_GLContext InitializeOpenGL(SDL_Window *window)
{
    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (!gl)
    {
#if LS_DEBUG
        printf("InitializeOpenGL: SDL_GL_CreateContext failed: %s\n", SDL_GetError());
#endif // LS_DEBUG
        return nullptr;
    }

    if (SDL_GL_MakeCurrent(window, gl) != 0)
    {
#if LS_DEBUG
        printf("InitializeOpenGL: SDL_GL_MakeCurrent failed: %s\n", SDL_GetError());
#endif // LS_DEBUG

        SDL_GL_DeleteContext(gl);
        return nullptr;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
#if LS_DEBUG
        printf("InitializeOpenGL: gladLoadGLLoader failed\n");
#endif // LS_DEBUG

        SDL_GL_DeleteContext(gl);
        return nullptr;
    }

    SDL_GL_SetSwapInterval(1);

    return gl;
}

GLRenderer *GLRenderer::Create(const SpriteList *sprites, const Scratch3VMOptions &options)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return nullptr;

    // Create window
    SDL_Window *window = CreateWindow(options);  
    if (!window)
    {
        SDL_Quit();
        return nullptr;
    }

    // Initialize OpenGL
    SDL_GLContext gl = InitializeOpenGL(window);
    if (!gl)
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return nullptr;
    }

    // Load shaders
    SpriteShader *ss = CreateSpriteShader();
    
    // Create quad for rendering sprites
    GLuint vao, vbo, ebo;
    if (!CreateQuad(&vao, &vbo, &ebo))
    {
#if LS_DEBUG
        printf("GLRenderer::Create: Failed to create quad\n");
#endif // LS_DEBUG

        delete ss;
        SDL_GL_DeleteContext(gl);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return nullptr;
    }

    // Initialize ImGui
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, gl);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    GLRenderer *r = new GLRenderer();

    r->_window = window;
    r->_gl = gl;
    r->_options = options;

    r->_spriteShader = ss;

    r->_quadVao = vao;
    r->_quadVbo = vbo;
    r->_quadEbo = ebo;

    r->_sprites = sprites;

    // Set viewport
    r->SetLogicalSize(-VIEWPORT_WIDTH / 2, VIEWPORT_WIDTH / 2,
        -VIEWPORT_HEIGHT / 2, VIEWPORT_HEIGHT / 2);
    r->Resize();

    SDL_ShowWindow(window);

#if LS_DEBUG
    printf("GLRenderer::Create: OpenGL version: %s\n", glGetString(GL_VERSION));
#endif // LS_DEBUG

    return r;
}

bool GLRenderer::TouchingColor(Sprite *sprite, const Vector3 &color, const Vector3 *mask)
{
    assert(sprite != nullptr);

    if (!_queryFbo)
        return false;

    if (!sprite->IsVisible())
        return false;

    sprite->Update();

    const AABB &bbox = sprite->GetBoundingBox();
    if (bbox.hi.x - bbox.lo.x <= 0 || bbox.hi.y - bbox.lo.y <= 0)
		return false;

    const IntVector4 bounds(bbox.lo.x, bbox.hi.x, bbox.lo.y, bbox.hi.y);

    if (!TouchingColorBegin(sprite, color, mask, bounds))
        return false;

    return TouchingColorEnd(color, bounds);
}

void GLRenderer::SetLogicalSize(int left, int right, int bottom, int top)
{
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;

    _proj = mutil::ortho(left, right, bottom, top, -1.0f, 1.0f);

    _logicalSize = Vector2(right - left, top - bottom);
}

void GLRenderer::BeginRender()
{
    _frame++;
    _lastTime = _time;
    _time = ls_time64() - _startTime;
    _deltaTime = _time - _lastTime;
    _fps = 1.0 / _deltaTime;
    _objectsDrawn = 0;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_STENCIL_TEST);
    glDepthMask(GL_FALSE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(_viewport.x, _viewport.y, _viewport.width, _viewport.height);

    if (_options.freeAspectRatio)
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    else
    {
        int width, height;
        SDL_GL_GetDrawableSize(_window, &width, &height);

        // clear whole screen
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // clear viewport
        glEnable(GL_SCISSOR_TEST);
        glScissor(_viewport.x, _viewport.y, _viewport.width, _viewport.height);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glScissor(0, 0, width, height);
        glDisable(GL_SCISSOR_TEST);
    }
}

void GLRenderer::Render()
{
    _spriteShader->Use();
    _spriteShader->SetProj(_proj);

    _spriteShader->SetUseColorMask(false);

    glBindVertexArray(_quadVao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadEbo);

    for (Sprite *s = _sprites->Head(); s; s = s->GetNext())
    {
        if (!PrepareSprite(s, _spriteShader))
            continue; // not visible

        glDrawElements(GL_TRIANGLES, QUAD_INDEX_COUNT, GL_UNSIGNED_BYTE, 0);

        _objectsDrawn++;
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
    SDL_GL_GetDrawableSize(_window, &_width, &_height);
    _viewport.Resize(_options.freeAspectRatio, _width, _height);

    if (_queryRbo)
    {
       glDeleteRenderbuffers(1, &_queryRbo);
		_queryRbo = 0;
    }

    if (_queryFbo)
    {
        glDeleteFramebuffers(1, &_queryFbo);
        _queryFbo = 0;
    }

    glGenFramebuffers(1, &_queryFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _queryFbo);

    GLuint colorBuffer;
    glGenTextures(1, &colorBuffer);

    glBindTexture(GL_TEXTURE_2D, colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _viewport.width, _viewport.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);

    glGenRenderbuffers(1, &_queryRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, _queryRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _viewport.width, _viewport.height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _queryRbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("GLRenderer::Resize: Framebuffer is not complete\n");

        glDeleteRenderbuffers(1, &_queryRbo);
        glDeleteFramebuffers(1, &_queryFbo);

        _queryRbo = 0;
        _queryFbo = 0;
    }
}

GLRenderer::GLRenderer() :
    _window(nullptr), _gl(nullptr),
    _options({}),
    _left(0), _right(0), _bottom(0), _top(0),
    _viewport({}),
    _width(0), _height(0),
    _queryFbo(0), _queryRbo(0),
    _frame(0),
    _startTime(0), _lastTime(0), _time(0), _deltaTime(0), _fps(-1),
    _objectsDrawn(0),
    _quadVao(0), _quadVbo(0), _quadEbo(0),
    _spriteShader(nullptr),
    _sprites(nullptr)
{
    _startTime = ls_time64();
}

GLRenderer::~GLRenderer()
{
    if (_queryRbo)
        glDeleteRenderbuffers(1, &_queryRbo);

    if (_queryFbo)
        glDeleteFramebuffers(1, &_queryFbo);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    delete _spriteShader;

    glDeleteVertexArrays(1, &_quadVao);
    glDeleteBuffers(1, &_quadVbo);
    glDeleteBuffers(1, &_quadEbo);

    SDL_GL_DeleteContext(_gl);
    SDL_DestroyWindow(_window);

    SDL_Quit();
}

bool GLRenderer::TouchingColorBegin(Sprite *sprite, const Vector3 &color, const Vector3 *mask, const IntVector4 &bounds)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _queryFbo);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    
    const IntVector2 size(bounds.y - bounds.x, bounds.w - bounds.z);

    glViewport(0, 0, size.x, size.y);
    Matrix4 proj = mutil::ortho(bounds.x, bounds.y, bounds.z, bounds.w);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Draw sprite to stencil buffer

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glColorMask(0, 0, 0, 0);

    glBindVertexArray(_quadVao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quadEbo);

    _spriteShader->Use();
    _spriteShader->SetProj(proj);

    if (mask)
    {
        _spriteShader->SetUseColorMask(true);
        _spriteShader->SetColorMask(*mask);
    }

    PrepareSprite(sprite, _spriteShader);
    glDrawElements(GL_TRIANGLES, QUAD_INDEX_COUNT, GL_UNSIGNED_BYTE, 0);

    // Draw rest of sprites to color buffer

    if (mask)
        _spriteShader->SetUseColorMask(false);

    glStencilFunc(GL_EQUAL, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glColorMask(1, 1, 1, 1);

    for (Sprite *s = _sprites->Head(); s; s = s->GetNext())
    {
        if (s == sprite)
            continue;

        if (!PrepareSprite(s, _spriteShader))
            continue; // not visible

        glDrawElements(GL_TRIANGLES, QUAD_INDEX_COUNT, GL_UNSIGNED_BYTE, 0);

        _objectsDrawn++;
    }

    glDisable(GL_STENCIL_TEST);

    return CheckGLError();
}

bool GLRenderer::TouchingColorEnd(const Vector3 &color, const IntVector4 &bounds)
{
    const IntVector2 size(bounds.y - bounds.x, bounds.w - bounds.z);
    const int32_t bufferSize = size.x * size.y * 4;

    uint8_t *buffer = new uint8_t[bufferSize];
    uint8_t *const end = buffer + bufferSize;

    IntVector3 icolor(color * 255.0f);

    glReadPixels(0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    if (!CheckGLError())
    {
        delete[] buffer;
        return false;
    }

    for (uint8_t *pixel = buffer; pixel < end; pixel += 4)
    {
        if (pixel[3] == 0)
            continue; // transparent

        if (pixel[0] == icolor.x && pixel[1] == icolor.y && pixel[2] == icolor.z)
        {
			delete[] buffer;
			return true;
		}
    }

    delete[] buffer;
    return false;
}

bool CheckGLError()
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        printf("OpenGL error: 0x%x\n", err);
        return false;
    }

    return true;
}
