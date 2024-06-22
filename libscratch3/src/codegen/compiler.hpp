#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

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

	std::unordered_map<std::string, std::vector<DataReference>> _managedStrings; // managed string pool (string -> references)
	std::unordered_map<std::string, std::vector<DataReference>> _plainStrings; // plain string pool (string -> references)

	std::vector<std::pair<uint64_t, std::string>> _importSymbols; // import procedure symbols (offset, name)
	std::unordered_map<std::string, uint64_t> _exportSymbols; // export procedure symbols (name -> offset)

	std::vector<std::pair<DataReference, DataReference>> _references; // (from, to)

	void Write(SegmentType seg, const void *data, size_t size);

	void *AllocText(size_t size);
	void WriteText(const void *data, size_t size);
	inline void WriteOpcode(uint8_t opcode) { WriteText(&opcode, 1); }
	template <typename T>
	inline void WriteText(const T &data) { WriteText(&data, sizeof(T)); }

	void WriteString(SegmentType seg, const std::string &str);
	void CreateString(void *dst, const std::string &str);
	void FlushStringPool();

	void WriteAbsoluteJump(uint8_t opcode, uint64_t off);
	void WriteRelativeJump(uint8_t opcode, int64_t off);

	void PushString(const std::string &str);
	void PushValue(const Value &value);

	void AlignText();

	void *AllocStable(size_t size);
	void WriteStable(const void *data, size_t size);
	template <typename T>
	void WriteStable(const T &data) { WriteStable(&data, sizeof(T)); }

	void *AllocData(size_t size);
	void WriteData(const void *data, size_t size);
	template <typename T>
	inline void WriteData(const T &data) { WriteData(&data, sizeof(T)); }

	void *AllocRdata(size_t size);
	void WriteRdata(const void *data, size_t size);
	template <typename T>
	inline void WriteRdata(const T &data) { WriteRdata(&data, sizeof(T)); }

	void *AllocDebug(size_t size);
	void WriteDebug(const void *data, size_t size);
	template <typename T>
	inline void WriteDebug(const T &data) { WriteDebug(&data, sizeof(T)); }

	size_t WriteReference(SegmentType from, SegmentType to, uint64_t off = -1);
	size_t CreateReference(void *dst, SegmentType seg, uint64_t off = -1);
	size_t SetReference(size_t refId, SegmentType seg, uint64_t newOff);

	void ResolvePointer(void *dst, SegmentType *seg, uint64_t *off);

	void Link();

	friend class Compiler;
};

CompiledProgram *CompileProgram(Program *p, Loader *loader, const Scratch3CompilerOptions *options);
