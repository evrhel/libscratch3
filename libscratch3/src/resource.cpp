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
	virtual size_t Size() const { return _size; }

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
	virtual ~ArchiveLoader() { zip_close(_archive); }
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

class FileResource : public Resource
{
public:
	virtual const uint8_t *Data() const override { return static_cast<const uint8_t *>(_data); }
	virtual size_t Size() const override { return _size; }

	FileResource(void *data, size_t size, ls_handle map, ls_handle file)
		: _data(data), _size(size), _map(map), _file(file) {}

	virtual ~FileResource()
	{
		if (_map) ls_munmap(_map, _data);
		else free(_data);

		if (_file)
			ls_close(_file);
	}

private:
	void *_data;
	size_t _size;
	ls_handle _map;
	ls_handle _file;
};

class DirectoryLoader : public Loader
{
public:
	DirectoryLoader(const char *dirname)
		: _dirname(dirname) {}
	virtual ~DirectoryLoader() {}
protected:
	virtual Resource *Load(const std::string &name) override
	{
		std::string filename = _dirname + "/" + name;

		ls_handle file = ls_open(filename.c_str(), LS_A_READ, LS_S_READ, LS_OPEN_EXISTING);
		if (!file)
			return nullptr;

		size_t pagesize = ls_page_size();

		struct ls_stat st;
		if (ls_fstat(file, &st) != 0)
		{
			ls_close(file);
			return nullptr;
		}

		void *data = nullptr;
		size_t size = st.size;
		ls_handle map = nullptr;

		if (size >= pagesize)
			data = ls_mmap(file, size, 0, LS_PROT_READ, &map);

		if (!data)
		{
			data = malloc(size);
			if (!data)
			{
				ls_close(file);
				return nullptr;
			}

			size = ls_read(file, data, size, nullptr);
			if (size == -1)
			{
				free(data);
				ls_close(file);
				return nullptr;
			}

			ls_close(file), file = nullptr;
		}

		return new FileResource(data, size, map, file);
	}
private:
	std::string _dirname;
};

Loader *CreateArchiveLoader(const char *filename)
{
	zip_t *archive = zip_open(filename, 0, nullptr);
	if (archive == nullptr)
		return nullptr;
	return new ArchiveLoader(archive);
}

Loader *CreateMemoryLoader(const char *data, size_t size)
{
	// TODO: Implement
	return nullptr;
}

Loader *CreateDirectoryLoader(const char *dirname)
{
	struct ls_stat st;
	if (ls_stat(dirname, &st) != 0)
		return nullptr;

	if (st.type != LS_FT_DIR)
		return nullptr; // not a directory

	return new DirectoryLoader(dirname);
}
