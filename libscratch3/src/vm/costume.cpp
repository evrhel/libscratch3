#include "costume.hpp"

#include <cstdio>
#include <cmath>
#include <cassert>

#include <cairo/cairo.h>
#include <lysys/lysys.hpp>

#include "stb_image.h"

void Costume::Init(uint8_t *bytecode, uint64_t bytecodeSize, const bc::Costume *info, bool streamed)
{
	Cleanup();

	SetString(_name, (char *)(bytecode + info->name));
	_dataFormat = (char *)(bytecode + info->format);
	_bitmapResolution = info->bitmapResolution;
	_center.x = info->rotationCenterX;
	_center.y = info->rotationCenterY;
	_data = bytecode + info->data;
	_dataSize = info->dataSize;

	_streamed = streamed;
}

void Costume::Load()
{
	if (_dataFormat == "png" || _dataFormat == "jpg" || _dataFormat == "jpeg")
	{
		stbi_set_flip_vertically_on_load(true);

		// load image
		int width, height, channels;
		_bitmapData = stbi_load_from_memory(_data, _dataSize,
			&width, &height, &channels, 4);

		if (!_bitmapData)
		{
			printf("Costume::Load: Failed to load image %s\n", GetNameString());
			return;
		}

		_textures = (GLuint *)calloc(1, sizeof(GLuint));
		if (!_textures)
		{
			printf("Costume::Load: Failed to allocate memory for texture\n");
			return;
		}

		_lodCount = 1;

		_texWidth = width;
		_texHeight = height;

		_size = IntVector2(_texWidth, _texHeight);
		_logicalSize = Vector2(_size) / static_cast<float>(_bitmapResolution);
		_logicalCenter = Vector2(_center) / static_cast<float>(_bitmapResolution);

		if (!_streamed)
			Upload();
	}
	else if (_dataFormat == "svg")
	{
		_handle = rsvg_handle_new_from_data(_data, _dataSize, NULL);
		if (!_handle)
		{
			printf("Costume::Load: Failed to load SVG %s\n", GetNameString());
			return;
		}
		
		RsvgDimensionData dim;
		rsvg_handle_get_dimensions(_handle, &dim);

		_svgWidth = dim.width;
		_svgHeight = dim.height;

		_size = IntVector2(_svgWidth, _svgHeight);
		_logicalSize = Vector2(_size);
		_logicalCenter = Vector2(_center);
		
		if (!_streamed)
		{
			// preload some LODs (arbitrary)
			(void)GetTexture(Vector2(1));
			(void)GetTexture(Vector2(2));
		}
	}

#if LS_DEBUG
	printf("Costume::Load: Loaded %s\n", GetNameString());
#endif // LS_DEBUG
}

GLuint Costume::GetTexture(const Vector2 &scale)
{
	if (!_handle)
	{
		Upload();
		if (_uploadError)
			return 0;

		return _textures[0]; // not an SVG, always use first LOD
	}

	constexpr const float kIndexOffset = 8;

	float scaleMax = mutil::max(mutil::abs(scale.x), mutil::abs(scale.y));

	int lod = std::max(static_cast<int>(mutil::ceil(log2f(scaleMax)) + kIndexOffset), 0);
	int lodScale = static_cast<int>(powf(2, lod - kIndexOffset));

	if (lod >= _lodCount)
	{
		GLsizei newLodCount = lod + 1;

		GLuint *newTextures = (GLuint *)realloc(_textures, newLodCount * sizeof(GLuint));
		if (!newTextures)
		{
			printf("Costume::GetTexture: Failed to allocate memory for LOD textures\n");
			abort();
		}

		// zero out new textures
		for (GLsizei i = _lodCount; i < newLodCount; i++)
			newTextures[i] = 0;

		_textures = newTextures;
		_lodCount = newLodCount;
	}

	if (!_textures[lod]) // create texture if it doesn't exist
		_textures[lod] = RenderLod(lodScale);

	return _textures[lod];
}

bool Costume::TestCollision(int32_t x, int32_t y) const
{
	return true;
}

Costume::Costume() :
	_textures(nullptr), _lodCount(0),
	_streamed(false), _uploaded(false), _uploadError(false),
	_texWidth(0), _texHeight(0),
	_bitmapResolution(0),
	_data(nullptr), _dataSize(0)
{
	_handle = nullptr;
	_svgWidth = _svgHeight = 0;

	_bitmapData = nullptr;

	InitializeValue(_name);
}

Costume::~Costume()
{
	Cleanup();
}

void Costume::Upload()
{
	assert(_handle == nullptr);

	if (_uploaded)
		return;

	assert(_bitmapData != nullptr);

#if LS_DEBUG
	printf("Costume::Upload: Uploading %s\n", GetNameString());
#endif // LS_DEBUG

	// upload to gpu
	glGenTextures(1, _textures);
	glBindTexture(GL_TEXTURE_2D, _textures[0]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _texWidth, _texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, _bitmapData);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(_bitmapData), _bitmapData = nullptr;

	_uploaded = true;
}

void Costume::Cleanup()
{
	if (_textures)
	{
		for (GLsizei i = 0; i < _lodCount; i++)
		{
			if (_textures[i])
				glDeleteTextures(1, &_textures[i]);
		}

		free(_textures), _textures = nullptr;
		_lodCount = 0;
	}

	if (_handle)
		g_object_unref(_handle), _handle = nullptr;

	if (_bitmapData)
		stbi_image_free(_bitmapData), _bitmapData = nullptr;

	_texWidth = _texHeight = 0;
	_svgWidth = _svgHeight = 0;

	_streamed = false;
	_uploaded = false;
	_uploadError = false;

	ReleaseValue(_name);
}

GLuint Costume::RenderLod(double scale)
{
	if (!_handle)
		return 0; // not an SVG

#if LS_DEBUG
	printf("Costume::RenderLod: Rendering %s at scale %.2f\n", GetNameString(), scale);
#endif // LS_DEBUG

	uint32_t width = static_cast<uint32_t>(_svgWidth * scale);
	uint32_t height = static_cast<uint32_t>(_svgHeight * scale);

	// setup cairo surface
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
		return 0;

	// create cairo context
	cairo_t *cr = cairo_create(surface);
	if (cairo_status(cr) != CAIRO_STATUS_SUCCESS)
	{
		cairo_surface_destroy(surface);
		return 0;
	}

	double scaleX = static_cast<double>(width) / _svgWidth;
	double scaleY = static_cast<double>(height) / _svgHeight;
	cairo_scale(cr, scaleX, -scaleY);
	cairo_translate(cr, 0, -_svgHeight);

	// render to cairo surface
	if (!rsvg_handle_render_cairo(_handle, cr))
	{
		cairo_destroy(cr);
		cairo_surface_destroy(surface);
		return 0;
	}

	unsigned char *data = cairo_image_surface_get_data(surface);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// cleanup surface and context
	cairo_surface_destroy(surface), data = nullptr;
	cairo_destroy(cr), cr = nullptr;

	return texture;
}
