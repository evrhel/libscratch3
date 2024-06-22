#pragma once
/*
#include <cstdint>
#include <vector>

#include "../ast/astdef.hpp"

struct CostumeInfo
{
	char *name;
	char *dataFormat;
	int32_t bitmapResolution;
	double rotationCenterX;
	double rotationCenterY;
	uint8_t *data;
	uint64_t dataSize;
};

struct SoundInfo
{
	char *name;
	char *dataFormat;
	double rate;
	unsigned int sampleCount;
	uint8_t *data;
	uint64_t dataSize;
};

struct ScriptInfo
{
	uint8_t *loc;
};

struct SpriteInfo
{
	char *name;
	double x;
	double y;
	double size;
	double direction;
	int64_t currentCostume;
	int64_t layer;

	bool visible;
	bool isStage;
	bool draggable;
	RotationStyle rotationStyle;

	ScriptInfo init;
	std::vector<ScriptInfo> scripts;

	std::vector<CostumeInfo> costumes;
	std::vector<SoundInfo> sounds;
};

using ParsedSprites = std::vector<SpriteInfo>;

//! \brief Parse the bytecode to extract the sprites.
//!
//! \param bytecode The bytecode to parse.
//! \param bytecodeSize The size of the bytecode.
//! \param sprites The parsed sprites.
void ParseSprites(uint8_t *bytecode, size_t bytecodeSize, ParsedSprites *sprites);*/
