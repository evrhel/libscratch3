#pragma once

#include <string>

#include <glad/glad.h>

#include <librsvg/rsvg.h>
#include <cairo/cairo.h>

#include <mutil/mutil.h>

#include "preload.hpp"
#include "memory.hpp"

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
	constexpr const Value &GetNameValue() const { return _name; }
	constexpr const String *GetName() const { return _name.u.string; }
	constexpr const char *GetNameString() const { return _name.u.string->str; }

	constexpr const IntVector2 &GetCenter() const { return _center; }
	constexpr const IntVector2 &GetSize() const { return _size; }

	constexpr const Vector2 &GetLogicalCenter() const { return _logicalCenter; }
	constexpr const Vector2 &GetLogicalSize() const { return _logicalSize; }

	//! \brief Get the texture of the costume, at a given scale
	//! 
	//! \param scale The scale of the costume
	//! 
	//! \return The texture of the costume
	GLuint GetTexture(const Vector2 &scale);

	//! \brief Checks if a point is inside the costume
	//!
	//! \param x The X coordinate of the point, in units
	//! \param y The Y coordinate of the point, in units
	//!
	//! \return Whether the point is inside the costume
	bool TestCollision(int32_t x, int32_t y) const;

	void Init(const CostumeInfo *info);

	void Load();

	constexpr bool IsBitmap() const { return _handle == nullptr; }

	Costume &operator=(const Costume &) = delete;
	Costume &operator=(Costume &&) = delete;

	Costume();
	Costume(const Costume &) = delete;
	Costume(Costume &&) = delete;
	~Costume();
private:
	Value _name;

	GLuint *_textures; // texture ids for each LOD, bitmaps have only one LOD
	GLsizei _lodCount; // number of LODs

	// for base LOD
	GLuint _texWidth, _texHeight;

	IntVector2 _center;
	IntVector2 _size;

	Vector2 _logicalCenter;
	Vector2 _logicalSize;

	int32_t _bitmapResolution;

	// svg specific
	RsvgHandle *_handle;
	int _svgWidth, _svgHeight;

	// data
	std::string _dataFormat;
	uint8_t *_data;
	uint64_t _dataSize;

	void Cleanup();

	GLuint RenderLod(double scale);
};
