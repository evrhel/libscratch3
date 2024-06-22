#pragma once

#include <string>
#include <cstdint>

#include <glad/glad.h>
#include <librsvg/rsvg.h>
#include <mutil/mutil.h>

#include "memory.hpp"
#include "../codegen/util.hpp"

// threshold for the collision mask's alpha channel
#define MASK_THRESHOLD 128

using namespace mutil;

//! \brief A costume (skin) for a sprite
class Costume
{
public:
	constexpr const Value &GetNameValue() const { return _name; }
	constexpr const String *GetName() const { return _name.u.string; }
	constexpr const char *GetNameString() const { return _name.u.string->str; }

	//! \brief Get the center of the costume
	//!
	//! \return The center of the costume, in pixels
	constexpr const IntVector2 &GetCenter() const { return _center; }

	//! \brief Get the size of the costume
	//!
	//! \return The size of the costume, in pixels
	constexpr const IntVector2 &GetSize() const { return _size; }

	//! \brief Get the logical center of the costume
	//!
	//! \return The logical center of the costume, in units
	constexpr const Vector2 &GetLogicalCenter() const { return _logicalCenter; }

	//! \brief Get the logical size of the costume
	//!
	//! \return The logical size of the costume, in units
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

	//! \brief Initialize the costume
	//!
	//! Sets up basic information about the costume, such as the name and the
	//! center and size of the costume.
	//!
	//! \param bytecode The program bytecode
	//! \param bytecodeSize The size of the bytecode
	//! \param info The costume information
	void Init(uint8_t *bytecode, uint64_t bytecodeSize, const bc::Costume *info);

	//! \brief Load the costumes
	//!
	//! Loads any necessary data for the costume, such as the texture or SVG data.
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

	IntVector2 _center; // center of the costume (pixels)
	IntVector2 _size; // size of the costume (pixels)

	Vector2 _logicalCenter; // center of the costume (units)
	Vector2 _logicalSize; // size of the costume (units)

	int32_t _bitmapResolution; // pixels per unit

	// svg specific
	RsvgHandle *_handle;
	int _svgWidth, _svgHeight;

	// data
	std::string _dataFormat; // "svg", "png", etc.
	uint8_t *_data;
	uint64_t _dataSize;

	void Cleanup();

	GLuint RenderLod(double scale);
};
