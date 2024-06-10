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
	Segment_rdata,
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

struct ProgramHeader
{
	uint32_t magic;
	uint32_t version;
	uint32_t text; // Offset of text segment
	uint32_t rdata; // Offset of rdata segment
	uint32_t data; // Offset of data segment
	uint32_t bss; // Size of BSS segment
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
	Segment _rdata;
	Segment _data;
	size_t _bss; // Size of BSS segment

	std::unordered_map<std::string, DataReference> _strings;

	std::vector<std::pair<DataReference, DataReference>> _references; // (from, to)

	void WriteText(const void *data, size_t size);

	inline void WriteOpcode(uint8_t opcode) { WriteText(&opcode, 1); }

	template <typename T>
	inline void WriteText(const T &data) { WriteText(&data, sizeof(T)); }

	void PushString(const std::string &str);
	void PushValue(const Value &value);

	void WriteData(const void *data, size_t size);

	template <typename T>
	inline void WriteData(const T &data) { WriteData(&data, sizeof(T)); }

	void AllocRdata(size_t size);

	void WriteRdata(const void *data, size_t size);

	template <typename T>
	inline void WriteRdata(const T &data) { WriteRodata(&data, sizeof(T)); }

	void WriteBss(size_t size);

	void CreateReference(SegmentType src, SegmentType dst, uint64_t dstoff);

	friend class Compiler;
};

CompiledProgram *Compile(Program *p, Loader *loader);
