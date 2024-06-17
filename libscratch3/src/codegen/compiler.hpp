#pragma once

#include <cstdint>
#include <vector>

#include <scratch3/scratch3.h>

#include "../ast/ast.hpp"
#include "../resource.hpp"
#include "../vm/memory.hpp"

#define EVENT_SYMBOL_PREFIX "event_"

// "CSB3" in ASCII
#define PROGRAM_MAGIC 0x33425343

#define PROGRAM_VERSION 1

using Segment = std::vector<uint8_t>;

enum SegmentType
{
	Segment_text,
	Segment_stable, // Sprite table
	Segment_data,
	Segment_rdata,
	Segment_debug // Debug information
};

struct DataReference
{
	SegmentType seg;
	uint64_t off;
};

struct ProgramHeader
{
	uint32_t magic;
	uint32_t version;

	uint32_t text; // Offset of text segment
	uint32_t text_size; // Size of text segment

	uint32_t stable; // Offset of sprite table
	uint32_t stable_size; // Size of sprite table

	uint32_t data; // Offset of data segment
	uint32_t data_size; // Size of data segment

	uint32_t rdata; // Offset of rdata segment
	uint32_t rdata_size; // Size of rdata segment

	uint32_t debug; // Offset of debug segment
	uint32_t debug_size; // Size of debug segment
};

enum
{
	DebugEntryType_variable,
	DebugEntryType_broadcast,
	DebugEntryType_proc
};

struct DebugEntry
{
	uint32_t type;

	uint8_t _padding[4];

	union
	{
		struct
		{
			uint64_t id;
			uint64_t name;
			uint64_t sprite;
		} variable;

		struct
		{
			uint64_t id;
			uint64_t name;
		} broadcast;

		struct
		{
			uint64_t id;
			uint64_t name;
		} proc;
	} u;
};

class CompiledProgram final
{
public:
	uint8_t *Export(size_t *outSize) const;

	CompiledProgram &operator=(const CompiledProgram &) = delete;
	CompiledProgram &operator=(CompiledProgram &&) = delete;

	CompiledProgram() = default;
	CompiledProgram(const CompiledProgram &) = delete;
	~CompiledProgram() = default;
private:
	Segment _stable;
	Segment _text;
	Segment _rdata;
	Segment _data;
	Segment _debug;

	std::unordered_map<std::string, DataReference> _managedStrings;
	std::unordered_map<std::string, DataReference> _plainStrings;

	std::vector<std::pair<DataReference, DataReference>> _references; // (from, to)

	void Write(SegmentType seg, const void *data, size_t size);

	void WriteText(const void *data, size_t size);

	inline void WriteOpcode(uint8_t opcode) { WriteText(&opcode, 1); }

	template <typename T>
	inline void WriteText(const T &data) { WriteText(&data, sizeof(T)); }

	void WriteString(SegmentType seg, const std::string &str);

	void WriteAbsoluteJump(uint8_t opcode, uint64_t off);
	void WriteRelativeJump(uint8_t opcode, int64_t off);

	void PushString(const std::string &str);
	void PushValue(const Value &value);

	void AlignText();

	void WriteStable(const void *data, size_t size);

	template <typename T>
	void WriteStable(const T &data) { WriteStable(&data, sizeof(T)); }

	void AllocData(size_t size);

	void WriteData(const void *data, size_t size);

	template <typename T>
	inline void WriteData(const T &data) { WriteData(&data, sizeof(T)); }

	void AllocRdata(size_t size);

	void WriteRdata(const void *data, size_t size);

	template <typename T>
	inline void WriteRdata(const T &data) { WriteRdata(&data, sizeof(T)); }

	void WriteDebug(const void *data, size_t size);

	template <typename T>
	inline void WriteDebug(const T &data) { WriteDebug(&data, sizeof(T)); }

	void WriteReference(SegmentType seg, const DataReference &dst);
	void WriteReference(SegmentType seg, SegmentType dst, uint64_t dstoff);

	friend class Compiler;
};

CompiledProgram *CompileProgram(Program *p, Loader *loader, const Scratch3CompilerOptions *options);
