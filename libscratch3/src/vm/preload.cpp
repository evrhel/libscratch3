/*#include "preload.hpp"

#include "../codegen/util.hpp"

static uint8_t *ParseCostume(uint8_t *bytecode, size_t bytecodeSize, uint8_t *loc, CostumeInfo *info)
{
	bc::Costume *costume = (bc::Costume *)loc;

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

	return loc + sizeof(bc::Costume);
}

static uint8_t *ParseSound(uint8_t *bytecode, size_t bytecodeSize, uint8_t *loc, SoundInfo *info)
{
	bc::Sound *sound = (bc::Sound *)loc;

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

	return loc + sizeof(bc::Sound);
}

static uint8_t *ParseSprite(uint8_t *bytecode, size_t bytecodeSize, uint8_t *loc, SpriteInfo *info)
{
	bc::Sprite *sprite = (bc::Sprite *)loc;

	info->name = (char *)(bytecode + sprite->name);
	info->x = sprite->x;
	info->y = sprite->y;
	info->size = sprite->size;
	info->direction = *(double *)ptr;
	info->currentCostume = *(int64_t *)ptr;
	info->layer = *(int64_t *)ptr;
	info->visible = *(uint8_t *)ptr;
	info->isStage = *(uint8_t *)ptr;
	info->draggable = *(uint8_t *)ptr;
	info->rotationStyle = (RotationStyle)*(uint8_t *)ptr;
	info->init.loc = bytecode + *(uint64_t *)ptr;

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

	return loc + sizeof(bc::Sprite);
}

void ParseSprites(uint8_t *bytecode, size_t bytecodeSize, ParsedSprites *sprites)
{
	bc::Header *header = (bc::Header *)bytecode;
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
*/