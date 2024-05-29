#pragma once

#include <string>

#include <glad/glad.h>

#include <librsvg/rsvg.h>
#include <cairo/cairo.h>

#include <mutil/mutil.h>

// maximum allowed texture size
#define MAX_TEXTURE_SIZE 2048

// threshold for the collision mask
#define MASK_THRESHOLD 128

using namespace mutil;

class Loader;
class CostumeDef;

class Costume
{
public:
	//! \brief Returns the texture of the costume
	//!
	//! \return An OpenGL texture
	constexpr GLuint GetTexture() const { return _texture; }

	//! \brief Returns the width of the rendered texture
	//!
	//! \return The width of the texture, in pixels
	constexpr GLuint GetTextureWidth() const { return _texWidth; }

	//! \brief Returns the height of the rendered texture
	//!
	//! \return The height of the costume, in pixels
	constexpr GLuint GetTextureHeight() const { return _texHeight; }

	//! \brief Returns the logical center of the costume
	//!
	//! This does not have to be located within the costume itself, it
	//! is used as an anchor point for the sprite.
	//!
	//! \return The center of the costume, in units relative to the
	//! middle of the image
	constexpr const Vector2 &GetLogicalCenter() const { return _logicalCenter; }

	//! \brief Returns the logical size of the costume
	//!
	//! \return The size of the costume, in units
	constexpr const IntVector2 &GetLogicalSize() const { return _logicalSize; }

	//! \brief Checks if a point is inside the costume
	//!
	//! \param x The X coordinate of the point, in units
	//! \param y The Y coordinate of the point, in units
	//!
	//! \return Whether the point is inside the costume
	bool TestCollision(int32_t x, int32_t y) const;

	//! \brief Load a costume from an AST node
	//!
	//! \param loader The loader to use to load resources
	//! \param def The costume definition
	void Load(Loader *loader, CostumeDef *def);

	//! \brief Render the costume to a texture of a given size
	//!
	//! \param scale The scale of the costume
	void Render(double scale);

	Costume &operator=(const Costume &) = delete;
	Costume &operator=(Costume &&) = delete;

	Costume();
	Costume(const Costume &) = delete;
	Costume(Costume &&) = delete;
	~Costume();
private:
	GLuint _texture;
	GLuint _texWidth, _texHeight;
	uint8_t *_mask; // collision mask

	Vector2 _logicalCenter;
	IntVector2 _logicalSize;

	// svg specific
	RsvgHandle *_handle;
	int _svgWidth, _svgHeight;

	void Cleanup();

	//! \brief Renders SVG images to a texture
	//!
	//! If the costume is not an SVG, does nothing. If the SVG has
	//! already been rendered to a texture with a larger size, does
	//! nothing. Otherwise, renders the SVG to a texture, which can
	//! be retrieved with GetTexture().
	//!
	//! \param width The width of the texture, in pixels
	//! \param height The height of the texture, in pixels
	void RenderSVG(uint32_t width, uint32_t height);

	//! \brief Generates a collision mask for a texture
	//!
	//! \param pixels The pixel data of the texture, the alpha channel
	//! is used to generate the mask and must be the last byte of each
	//! pixel
	//! \param width The width of the texture, in pixels
	//! \param height The height of the texture, in pixels
	//!
	//! \return Whether the mask was generated successfully
	bool GenMaskForPixels(const uint8_t *pixels, uint32_t width, uint32_t height);
};
