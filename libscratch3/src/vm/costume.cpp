#include "costume.hpp"

#include <cstdio>

struct Reader
{
	Resource *res;
	unsigned int pos;
};

static cairo_status_t cairoReader(void *closure, unsigned char *data, unsigned int length)
{
	Reader *reader = (Reader *)closure;

	unsigned int avail = reader->res->Size() - reader->pos;
	if (length > avail)
		return CAIRO_STATUS_READ_ERROR;

	memcpy(data, reader->res->Data() + reader->pos, length);
	reader->pos += length;

	return CAIRO_STATUS_SUCCESS;
}

void Costume::LoadPNG(Loader *loader, const std::string &path, int resolution)
{
	if (_bitmap)
	{
		printf("Costume already loaded\n");
		return;
	}

	Resource *res = loader->Find(path);
	if (!res)
	{
		printf("Resource not found: %s\n", path.c_str());
		return;
	}

	Reader reader;
	reader.res = res;
	reader.pos = 0;

	_bitmap = cairo_image_surface_create_from_png_stream(cairoReader, &reader);
	if (cairo_surface_status(_bitmap) != CAIRO_STATUS_SUCCESS)
	{
		cairo_surface_destroy(_bitmap);
		printf("Failed to load PNG: %s\n", path.c_str());
		return;
	}

	int width = cairo_image_surface_get_width(_bitmap);
	int height = cairo_image_surface_get_height(_bitmap);

	_width = width / resolution;
	_height = height / resolution;
	_resolution = resolution;
	
	printf("Loaded PNG: %s\n", path.c_str());
}

void Costume::Render(cairo_t *cr, double x, double y, double direction, double xScale, double yScale)
{
	if (_bitmap)
		RenderBitmap(cr, x, y, direction, xScale, yScale);
	else
		RenderSVG(cr, x, y, direction, xScale, yScale);
}

Costume::Costume() :
	_width(0), _height(0),
	_resolution(1),
	_bitmap(nullptr)
{
}

Costume::~Costume()
{
	if (_bitmap)
		cairo_surface_destroy(_bitmap);
}

void Costume::RenderBitmap(cairo_t *cr, double x, double y, double direction, double xScale, double yScale)
{
	cairo_translate(cr, x, y);
	cairo_scale(cr, xScale, yScale);

	cairo_set_source_surface(cr, _bitmap, 0, 0);
	cairo_paint(cr);

	cairo_identity_matrix(cr);
}

void Costume::RenderSVG(cairo_t *cr, double x, double y, double direction, double xScale, double yScale)
{
	// TODO: implement
}
