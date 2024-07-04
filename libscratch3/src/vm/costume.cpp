#include "costume.hpp"

#include <cstdio>
#include <cmath>
#include <cassert>

#include <cairo/cairo.h>
#include <lysys/lysys.hpp>

#include "stb_image.h"

bool Costume::Init(uint8_t *bytecode, uint64_t bytecodeSize, const bc::Costume *info, bool streamed)
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

	return true;
}

void Costume::Load()
{
	if (_dataFormat == "png" || _dataFormat == "jpg" || _dataFormat == "jpeg")
	{
		stbi_set_flip_vertically_on_load(true);

		// load image
		int width, height, channels;
		_bitmapData = stbi_load_from_memory(_data, _dataSize,
			&width, &height, &channels, 0);
		if (!_bitmapData)
		{
			printf("Costume::Load: Failed to load image %s\n", GetNameString());
			return;
		}

		if (channels != 3 && channels != 4)
		{
			printf("Costume::Load: Invalid number of channels %d\n", channels);
			stbi_image_free(_bitmapData), _bitmapData = nullptr;
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
		_nComponents = channels;

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

		_nComponents = 4; // always RGBA
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

bool Costume::CheckCollision(int32_t x, int32_t y)
{
	if (x < 0 || y < 0 || x >= _size.x || y >= _size.y)
		return false;

	if (_nComponents != 4)
		return true; // no alpha channel, assume all pixels are collidable

	if (!GenerateCollisionMask())
		return false;

	return _collisionMask[y * _size.x + x];
}

Costume::Costume() :
	_textures(nullptr), _lodCount(0),
	_streamed(false), _uploaded(false), _uploadError(false),
	_texWidth(0), _texHeight(0),
	_bitmapResolution(0),
	_data(nullptr), _dataSize(0),
	_nComponents(0),
	_collisionMask(nullptr)
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

	GLenum format, internalFormat;
	switch (_nComponents)
	{
	case 3:
		format = GL_RGB;
		internalFormat = GL_RGB8;
		break;
	case 4:
		format = GL_RGBA;
		internalFormat = GL_RGBA8;
		break;
	default:
		printf("Costume::Upload: Invalid number of components %d\n", _nComponents);
		_uploadError = true;
		return;
	}

	// upload to gpu
	glGenTextures(1, _textures);
	glBindTexture(GL_TEXTURE_2D, _textures[0]);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _texWidth, _texHeight, 0, format, GL_UNSIGNED_BYTE, _bitmapData);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (_collisionMask)
		stbi_image_free(_bitmapData), _bitmapData = nullptr; // no longer needed

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

	if (_collisionMask)
		free(_collisionMask), _collisionMask = nullptr;

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
	{
		printf("Costume::RenderLod: Failed to create cairo surface\n");
		return 0;
	}

	// create cairo context
	cairo_t *cr = cairo_create(surface);
	if (cairo_status(cr) != CAIRO_STATUS_SUCCESS)
	{
		printf("Costume::RenderLod: Failed to create cairo context\n");
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
		printf("Costume::RenderLod: Failed to render SVG %s\n", GetNameString());
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
	cairo_surface_destroy(surface);
	cairo_destroy(cr);

	return texture;
}

bool Costume::GenerateCollisionMask()
{
	if (_collisionMask)
		return true; // already generated

	if (_nComponents != 4)
	{
		printf("Costume::GenerateCollisionMask: Invalid number of components %d\n", _nComponents);
		return false;
	}

	size_t size = (size_t)_size.x * (size_t)_size.y;

	if (_handle)
	{
		// setup cairo surface
		cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, _svgWidth, _svgHeight);
		if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
		{
			printf("Costume::GenerateSVGCollisionMask: Failed to create cairo surface\n");
			return false;
		}

		// create cairo context
		cairo_t *cr = cairo_create(surface);
		if (cairo_status(cr) != CAIRO_STATUS_SUCCESS)
		{
			printf("Costume::GenerateSVGCollisionMask: Failed to create cairo context\n");
			cairo_surface_destroy(surface);
			return false;
		}

		// render to cairo surface
		if (!rsvg_handle_render_cairo(_handle, cr))
		{
			printf("Costume::GenerateSVGCollisionMask: Failed to render SVG %s\n", GetNameString());
			cairo_destroy(cr);
			cairo_surface_destroy(surface);
			return false;
		}

		unsigned char *data = cairo_image_surface_get_data(surface);

		_collisionMask = (uint8_t *)malloc(size);
		if (!_collisionMask)
		{
			printf("Costume::GenerateSVGCollisionMask: Failed to allocate memory for collision mask\n");
			cairo_surface_destroy(surface);
			cairo_destroy(cr);
			return false;
		}

		// generate collision mask
		for (size_t i = 0; i < size; i++)
		{
			unsigned char *pixel = data + i * 4;
			_collisionMask[i] = pixel[3] >= MASK_THRESHOLD;
		}

		// cleanup surface and context
		cairo_surface_destroy(surface);
		cairo_destroy(cr);
	}
	else
	{
		if (!_bitmapData)
		{
			printf("Costume::GenerateCollisionMask: No bitmap data\n");
			return false;
		}

		_collisionMask = (uint8_t *)malloc(size);
		if (!_collisionMask)
		{
			printf("Costume::GenerateCollisionMask: Failed to allocate memory for collision mask\n");
			return false;
		}

		// generate collision mask
		for (size_t y = 0; y < _size.y; y++)
		{
			size_t srcOff = y * _size.x;
			size_t dstOff = (_size.y - y - 1) * _size.x; // flip y
			for (size_t x = 0; x < _size.x; x++)
			{
				unsigned char *pixel = _bitmapData + (srcOff + x) * 4;
				_collisionMask[dstOff + x] = pixel[3] >= MASK_THRESHOLD;
			}
		}

		if (_uploaded)
			stbi_image_free(_bitmapData), _bitmapData = nullptr; // no longer needed
	}

	return true;
}
