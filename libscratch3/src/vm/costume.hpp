#pragma once

#include <cairo/cairo.h>

#include "../resource.hpp"

class Costume
{
public:
	void LoadPNG(Loader *loader, const std::string &path, int resolution);

	void Render(cairo_t *cr, double x, double y, double direction, double xScale, double yScale);

	constexpr double GetWidth() const { return _width; }
	constexpr double GetHeight() const { return _height; }
	constexpr double GetResolution() const { return _resolution; }

	Costume &operator=(const Costume &) = delete;
	Costume &operator=(Costume &&) = delete;

	Costume();
	Costume(const Costume &) = delete;
	Costume(Costume &&) = delete;
	~Costume();
private:
	double _width, _height;
	double _resolution;

	cairo_surface_t *_bitmap;

	void RenderBitmap(cairo_t *cr, double x, double y, double direction, double xScale, double yScale);
	void RenderSVG(cairo_t *cr, double x, double y, double direction, double xScale, double yScale);
};
