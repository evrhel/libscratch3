#include "resource.hpp"

#include <zip.h>
#include <lysys/lysys.hpp>

Resource *Loader::Find(const std::string &name)
{
	auto it = _cache.find(name);
	if (it != _cache.end())
		return it->second;

	Resource *resource = Load(name);
	if (resource)
		_cache[name] = resource;

	return resource;
}

Loader::Loader() {}

Loader::~Loader()
{
	for (auto &p : _cache)
		delete p.second;
}

class ArchiveResource : public Resource
{
public:
	virtual const uint8_t *Data() const override { return static_cast<const uint8_t *>(_data); }
	virtual size_t Size() const override { return _size; }

	ArchiveResource(void *data, size_t size) : _data(data), _size(size) {}
	virtual ~ArchiveResource() { free(_data); }
private:
	void *_data;
	size_t _size;
};

class ArchiveLoader : public Loader
{
public:
	ArchiveLoader(zip_t *archive) : _archive(archive) {}

	virtual ~ArchiveLoader()
	{
		zip_close(_archive);
	}
protected:
	virtual Resource *Load(const std::string &name) override
	{
		const char *namestr = name.c_str();

		zip_stat_t st;
		if (zip_stat(_archive, namestr, 0, &st) != 0)
			return nullptr;

		zip_file_t *file = zip_fopen(_archive, namestr, 0);
		if (file == nullptr)
			return nullptr;

		size_t size = st.size;
		void *data = malloc(size);
		if (data == nullptr)
		{
			zip_fclose(file);
			return nullptr;
		}

		size = zip_fread(file, data, size);
		if (size == -1)
		{
			free(data);
			zip_fclose(file);
			return nullptr;
		}

		zip_fclose(file);

		return new ArchiveResource(data, size);
	}
private:
	zip_t *_archive;
};

Loader *CreateArchiveLoader(const void *data, size_t size)
{
	zip_source_t *source = zip_source_buffer_create(data, size, 0, nullptr);
	if (source == nullptr)
		return nullptr;

	zip_t *archive = zip_open_from_source(source, 0, nullptr);
	if (archive == nullptr)
	{
		zip_source_free(source);
		return nullptr;
	}

	// source is managed by the archive
	return new ArchiveLoader(archive);
}
