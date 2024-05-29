#include "costume.hpp"

#include <cstdio>

#include "../resource.hpp"
#include "../ast/ast.hpp"

#include "stb_image.h"

void Costume::Load(Loader *loader, CostumeDef *def)
{
	Resource *rsrc = loader->Find(def->md5ext);
	if (!rsrc)
	{
		printf("Costume::Load: Unknown resource %s\n", def->md5ext.c_str());
		return;
	}

	if (def->dataFormat == "png" || def->dataFormat == "jpg" || def->dataFormat == "jpeg")
	{
		stbi_set_flip_vertically_on_load(true);

		// load image
		int width, height, channels;
		unsigned char *data = stbi_load_from_memory(rsrc->Data(), rsrc->Size(),
			&width, &height, &channels, 4);

		if (!data)
		{
			printf("Costume::Load: Failed to load image %s\n", def->md5ext.c_str());
			return;
		}

		// check for invalid size
		if (width > MAX_TEXTURE_SIZE || height > MAX_TEXTURE_SIZE)
		{
			printf("Costume::Load: Image %s is too large (%dx%d)\n", def->md5ext.c_str(), width, height);
			return;
		}

		// generate mask
		if (!GenMaskForPixels(data, width, height))
		{
			printf("Costume::Load: Failed to generate mask for %s\n", def->md5ext.c_str());
			stbi_image_free(data);
			return;
		}

		// upload to gpu
		glGenTextures(1, &_texture);
		glBindTexture(GL_TEXTURE_2D, _texture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		_texWidth = width;
		_texHeight = height;

		_logicalSize = IntVector2(_texWidth / def->bitmapResolution);
	}
	else if (def->dataFormat == "svg")
	{
		_handle = rsvg_handle_new_from_data(rsrc->Data(), rsrc->Size(), NULL);
		if (!_handle)
		{
			printf("Costume::Load: Failed to load SVG %s\n", def->md5ext.c_str());
			return;
		}
		
		RsvgDimensionData dim;
		rsvg_handle_get_dimensions(_handle, &dim);

		// clamp to max texture size
		_svgWidth = std::min(dim.width, MAX_TEXTURE_SIZE);
		_svgHeight = std::min(dim.height, MAX_TEXTURE_SIZE);

		_logicalSize = IntVector2(_svgWidth, _svgHeight);

		// pre-render SVG
		RenderSVG(_svgWidth, _svgHeight);
	}
}

bool Costume::TestCollision(int32_t x, int32_t y) const
{
	// convert to pixel coordinates
	GLuint px = static_cast<GLuint>(x * _texWidth / _logicalSize.x);
	GLuint py = static_cast<GLuint>(y * _texHeight / _logicalSize.y);

	if (px >= _texWidth || py >= _texHeight)
		return false;

	return _mask[py * _texWidth + px];
}

void Costume::Render(double scale)
{
	if (!_handle)
		return; // ignore non-SVGs

	uint32_t width = static_cast<uint32_t>(_svgWidth * scale);
	uint32_t height = static_cast<uint32_t>(_svgHeight * scale);

	RenderSVG(width, height);
}

// https://graphics.stanford.edu/%7Eseander/bithacks.html#RoundUpPowerOf2
static constexpr uint32_t RoundUp2(uint32_t x)
{
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;

	return x;
}
Costume::Costume() :
	_texture(0),
	_texWidth(0), _texHeight(0)
{
	_handle = nullptr;
	_svgWidth = _svgHeight = 0;
}

Costume::~Costume()
{
	Cleanup();
}

void Costume::Cleanup()
{
	if (_texture)
		glDeleteTextures(1, &_texture), _texture = 0;

	if (_handle)
		g_object_unref(_handle), _handle = nullptr;

	if (_mask)
		free(_mask), _mask = nullptr;

	_texWidth = _texHeight = 0;
	_svgWidth = _svgHeight = 0;
}

void Costume::RenderSVG(uint32_t width, uint32_t height)
{
	if (!_handle)
		return; // not an SVG

	// check for invalid size
	constexpr uint32_t signBit = 0x80000000;
	if ((width & signBit) || (height & signBit))
		return; // invalid size
	
	// round up to nearest power of 2
	width = RoundUp2(width);
	height = RoundUp2(height);

	width = std::min<uint32_t>(width, MAX_TEXTURE_SIZE);
	height = std::min<uint32_t>(height, MAX_TEXTURE_SIZE);

	if (width < _svgWidth && height < _svgHeight)
		return; // already rendered a larger texture

	printf("Costume::Render: Rendering SVG at %dx%d\n", width, height);

	// setup cairo surface
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
	{
		printf("Costume::Render: Failed to create surface\n");
		return;
	}

	// create cairo context
	cairo_t *cr = cairo_create(surface);
	if (cairo_status(cr) != CAIRO_STATUS_SUCCESS)
	{
		printf("Costume::Render: Failed to create context\n");
		cairo_surface_destroy(surface);
		return;
	}

	// render to cairo surface
	if (!rsvg_handle_render_cairo(_handle, cr))
	{
		printf("Costume::Render: Failed to render SVG\n");
		cairo_destroy(cr);
		cairo_surface_destroy(surface);
		return;
	}

	unsigned char *data = cairo_image_surface_get_data(surface);

	if (!GenMaskForPixels(data, width, height))
	{
		printf("Costume::Render: Failed to generate mask\n");
		cairo_surface_destroy(surface), data = nullptr;
		cairo_destroy(cr), cr = nullptr;
		return;
	}

	// create texture
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// we use mipmaps on SVGs to reduce aliasing when shrinking as we do not ever
	// re-render at a lower resolution
	glGenerateMipmap(GL_TEXTURE_2D);

	// update properties
	if (_texture)
		glDeleteTextures(1, &_texture);
	_texture = texture;

	_texWidth = width;
	_texHeight = height;

	// cleanup surface and context
	cairo_surface_destroy(surface), data = nullptr;
	cairo_destroy(cr), cr = nullptr;
}

bool Costume::GenMaskForPixels(const uint8_t *pixels, uint32_t width, uint32_t height)
{
	// allocate mask
	uint8_t *mask = static_cast<uint8_t *>(malloc(width * height));
	if (!mask)
	{
		printf("Costume::GenMaskForPixels: Failed to allocate mask\n");
		return false;
	}

	// generate mask
	uint32_t count = width * height;
	for (uint32_t i = 0; i < count; i++)
		mask[i] = pixels[i * 4 + 3] > MASK_THRESHOLD;

	// update properties
	if (_mask)
		free(_mask);
	_mask = mask;

	return true;
}
