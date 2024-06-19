#pragma once

#include <string>
#include <unordered_map>

//! \brief An abstract resource.
class Resource
{
public:
    //! \brief Get a pointer to the resource's data.
    //!
    //! \return A pointer to the resource's data.
    virtual const uint8_t *Data() const = 0;

    //! \brief Get the number of bytes in the resource.
    //!
    //! \return The number of bytes in the resource.
    virtual size_t Size() const = 0;

    Resource() = default;
    virtual ~Resource() = default;
private:
};

//! \brief Abstract resource loader.
class Loader
{
public:
    //! \brief Locate a resource by name.
    //!
    //! \param name The name of the resource.
    //!
    //! \return A pointer to the resource, or nullptr if the resource could not be found.
    Resource *Find(const std::string &name);

    Loader();
    virtual ~Loader();
protected:
    virtual Resource *Load(const std::string &name) = 0;
private:
    std::unordered_map<std::string, Resource *> _cache;
};

//! \brief Create a loader for a compressed archive.
//!
//! \param data A pointer to the archive data.
//! \param size The size of the archive data.
//!
//! \return A pointer to the loader, or nullptr if the loader could not be created.
Loader *CreateArchiveLoader(const void *data, size_t size);
