#ifndef BACKEND_HPP
#define BACKEND_HPP

#ifdef USE_EASTL_BACKEND
#include "backends/eastl_backend.hpp"
#else
#include "backends/stl_backend.hpp"
#endif

#endif /* BACKEND_HPP */