#include "preload.hpp"

#include "../codegen/compiler.hpp"

static uint8_t *ParseCostume(uint8_t *bytecode, size_t bytecodeSize, uint8_t *loc, CostumeInfo *info)
{
	uint8_t *ptr = loc;

	info->name = (char *)(bytecode + *(uint64_t *)ptr);
	ptr += sizeof(uint64_t);

	info->dataFormat = (char *)(bytecode + *(uint64_t *)ptr);
	ptr += sizeof(uint64_t);

	info->bitmapResolution = *(int32_t *)ptr;
	ptr += sizeof(int32_t);

	info->rotationCenterX = *(double *)ptr;
	ptr += sizeof(double);

	info->rotationCenterY = *(double *)ptr;
	ptr += sizeof(double);

	info->data = bytecode + *(uint64_t *)ptr;
	ptr += sizeof(uint64_t);

	info->dataSize = *(uint64_t *)ptr;
	ptr += sizeof(uint64_t);

	return ptr;
}

static uint8_t *ParseSound(uint8_t *bytecode, size_t bytecodeSize, uint8_t *loc, SoundInfo *info)
{
	uint8_t *ptr = loc;

	info->name = (char *)(bytecode + *(uint64_t *)ptr);
	ptr += sizeof(uint64_t);

	info->dataFormat = (char *)(bytecode + *(uint64_t *)ptr);
	ptr += sizeof(uint64_t);

	info->rate = *(double *)ptr;
	ptr += sizeof(double);

	info->sampleCount = *(uint32_t *)ptr;
	ptr += sizeof(uint32_t);

	info->data = bytecode + *(uint64_t *)ptr;
	ptr += sizeof(uint64_t);

	info->dataSize = *(uint64_t *)ptr;
	ptr += sizeof(uint64_t);

	return ptr;
}

static uint8_t *ParseSprite(uint8_t *bytecode, size_t bytecodeSize, uint8_t *loc, SpriteInfo *info)
{
	uint8_t *ptr = loc;

	info->name = (char *)(bytecode + *(uint64_t *)ptr);
	ptr += sizeof(uint64_t);

	info->x = *(double *)ptr;
	ptr += sizeof(double);

	info->y = *(double *)ptr;
	ptr += sizeof(double);

	info->size = *(double *)ptr;
	ptr += sizeof(double);

	info->direction = *(double *)ptr;
	ptr += sizeof(double);

	info->currentCostume = *(int64_t *)ptr;
	ptr += sizeof(int64_t);

	info->layer = *(int64_t *)ptr;
	ptr += sizeof(int64_t);

	info->visible = *(uint8_t *)ptr;
	ptr += sizeof(uint8_t);

	info->isStage = *(uint8_t *)ptr;
	ptr += sizeof(uint8_t);

	info->draggable = *(uint8_t *)ptr;
	ptr += sizeof(uint8_t);

	info->rotationStyle = (RotationStyle)*(uint8_t *)ptr;
	ptr += sizeof(uint8_t);

	info->init.loc = bytecode + *(uint64_t *)ptr;
	ptr += sizeof(uint64_t);

	info->scripts.resize(*(uint64_t *)ptr);
	ptr += sizeof(uint64_t);

	for (size_t i = 0; i < info->scripts.size(); i++)
	{
		ScriptInfo &script = info->scripts.at(i);
		script.loc = bytecode + *(uint64_t *)ptr;
		ptr += sizeof(uint64_t);
	}

	info->costumes.resize(*(uint64_t *)ptr);
	ptr += sizeof(uint64_t);

	for (size_t i = 0; i < info->costumes.size(); i++)
	{
		CostumeInfo &costume = info->costumes.at(i);
		ptr = ParseCostume(bytecode, bytecodeSize, ptr, &costume);
	}

	info->sounds.resize(*(uint64_t *)ptr);
	ptr += sizeof(uint64_t);

	for (size_t i = 0; i < info->sounds.size(); i++)
	{
		SoundInfo &sound = info->sounds.at(i);
		ptr = ParseSound(bytecode, bytecodeSize, ptr, &sound);
	}

	return ptr;
}

void ParseSprites(uint8_t *bytecode, size_t bytecodeSize, ParsedSprites *sprites)
{
	ProgramHeader *header = (ProgramHeader *)bytecode;
	uint8_t *ptr = bytecode + header->stable;

	uint64_t count = *(uint64_t *)ptr;
	ptr += sizeof(uint64_t);

	sprites->resize(count);
	for (size_t i = 0; i < sprites->size(); i++)
	{
		SpriteInfo &info = sprites->at(i);
		ptr = ParseSprite(bytecode, bytecodeSize, ptr, &info);
	}
}
