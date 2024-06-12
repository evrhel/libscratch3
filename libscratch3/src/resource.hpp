#pragma once

#include <string>
#include <unordered_map>

class Resource
{
public:
    virtual const uint8_t *Data() const = 0;
    virtual size_t Size() const = 0;

    Resource() = default;
    virtual ~Resource() = default;
private:
};

class Loader
{
public:
    Resource *Find(const std::string &name);

    Loader();
    virtual ~Loader();
protected:
    virtual Resource *Load(const std::string &name) = 0;
private:
    std::unordered_map<std::string, Resource *> _cache;
};

Loader *CreateArchiveLoader(const void *data, size_t size);
Loader *CreateBytecodeLoader(const void *data, size_t size);
