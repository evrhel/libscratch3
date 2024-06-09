#pragma once

#include <cstdint>
#include <vector>

#include "../ast/ast.hpp"
#include "../resource.hpp"
#include "../vm/memory.hpp"

#define EVENT_SYMBOL_PREFIX "event_"

using Segment = std::vector<uint8_t>;

enum SegmentType
{
	Segment_text,
	Segment_data,
	Segment_rodata,
	Segment_bss
};

struct DataReference
{
	SegmentType seg;
	uint64_t off;
};

struct SpriteProto
{
	double x;
	double y;
	double size;
	double direction;
	int64_t costume;
	int64_t layer;
	double rotationStyle;
};

class CompiledProgram final
{
public:
	CompiledProgram &operator=(const CompiledProgram &) = delete;
	CompiledProgram &operator=(CompiledProgram &&) = delete;

	CompiledProgram() = default;
	CompiledProgram(const CompiledProgram &) = delete;
	~CompiledProgram() = default;
private:
	Segment _text;
	Segment _data;
	Segment _rodata;
	Segment _bss;

	DataReference _stage = { Segment_text, -1 };
	std::unordered_map<std::string, DataReference> _sprites;

	std::vector<std::pair<DataReference, DataReference>> _references;

	void PushEventHandler(const std::string &event);
	void PushEventSender(const std::string &event);

	void WriteText(const void *data, size_t size);

	inline void WriteOpcode(uint8_t opcode) { WriteText(&opcode, 1); }

	template <typename T>
	inline void WriteText(const T &data) { WriteText(&data, sizeof(T)); }

	void PushString(const std::string &str);
	void PushValue(const Value &value);

	void WriteData(const void *data, size_t size);

	template <typename T>
	inline void WriteData(const T &data) { WriteData(&data, sizeof(T)); }

	void WriteRodata(const void *data, size_t size);

	template <typename T>
	inline void WriteRodata(const T &data) { WriteRodata(&data, sizeof(T)); }

	void WriteBss(size_t size);

	void Align(SegmentType segment);

	void CreateReference(SegmentType src, uint64_t srcOffset, SegmentType dst, uint64_t dstOffset);

	void EmitSprite(const std::string &name, bool isStage);

	friend class Compiler;
};

CompiledProgram *Compile(Program *p, Loader *loader);
