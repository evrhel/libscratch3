#pragma once

#include "config.hpp"

#include <lysys/lysys.hpp>

#if defined(SCRATCH3_MULTITHREAD)
#define SCRATCH3_STORAGE LS_THREADLOCAL
#endif // SCRATCH3_MULTITHREAD
