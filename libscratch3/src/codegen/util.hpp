#pragma once

#include <cstdint>

#include "../vm/memory.hpp"

// See BYTECODE.md
namespace bc
{
	// Types

	typedef int8_t int8;
	typedef uint8_t uint8;
	typedef int16_t int16;
	typedef uint16_t uint16;
	typedef int32_t int32;
	typedef uint32_t uint32;
	typedef int64_t int64;
	typedef uint64_t uint64;
	typedef float float32;
	typedef double float64;

	typedef uint8 byte;
	typedef uint8 _bool;

	template <typename T>
	using ptr = uint64;

	struct string
	{
		byte data[];
	};

	struct Script
	{
		uint64 offset;
	};

	struct Costume
	{
		ptr<string> name;
		ptr<string> format;
		uint32 bitmapResolution;
		uint32 reserved;
		float64 rotationCenterX;
		float64 rotationCenterY;
		uint64 dataSize;
		ptr<byte> data;
	};

	struct Sound
	{
		ptr<string> name;
		ptr<string> format;
		float64 rate;
		uint64 sampleCount;
		uint64 dataSize;
		ptr<byte> data;
	};

	struct Sprite
	{
		ptr<string> name;
		float64 x;
		float64 y;
		float64 direction;
		float64 size;
		int64 currentCostume;
		int64 layer;
		_bool visible;
		_bool isStage;
		_bool draggable;
		uint8 rotationStyle;
		Script initializer;
		uint64 numScripts;
		ptr<Script> scripts;
		uint64 numCostumes;
		ptr<Costume> costumes;
		uint64 numSounds;
		ptr<Sound> sounds;
	};

	struct VarId
	{
		uint8_t id[3]; // 3-byte integer (LE)

		constexpr VarId() :
			id{ 0, 0, 0 } { }

		constexpr VarId(uint32_t id) :
			id{ (uint8_t)(id & 0xff), (uint8_t)((id >> 8) & 0xff), (uint8_t)((id >> 16) & 0xff) } { }

		constexpr uint32_t ToInt() const
		{
			return id[0] | (id[1] << 8) | (id[2] << 16);
		}
	};

	// Structure

	struct SpriteTable
	{
		uint64 count;
		Sprite sprites[];
	};

	struct Header
	{
		uint32 magic;
		uint32 version;
		ptr<byte> text;
		uint64 text_size;
		ptr<SpriteTable> stable;
		uint64 stable_size;
		ptr<byte> data;
		uint64 data_size;
		ptr<byte> rdata;
		uint64 rdata_size;
		ptr<byte> debug;
		uint64 debug_size;
	};
}
