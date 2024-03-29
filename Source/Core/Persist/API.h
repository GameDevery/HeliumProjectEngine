#pragma once

#include "Platform/System.h"

#include "Foundation/Profile.h"

#if HELIUM_SHARED
# ifdef HELIUM_PERSIST_EXPORTS
#  define HELIUM_PERSIST_API HELIUM_API_EXPORT
# else
#  define HELIUM_PERSIST_API HELIUM_API_IMPORT
# endif
#else
# define HELIUM_PERSIST_API
#endif

#define HELIUM_PERSIST_PROFILE 0

#if HELIUM_PROFILE_INSTRUMENT_ALL || HELIUM_PERSIST_PROFILE
#define HELIUM_PERSIST_SCOPE_TIMER( ... ) HELIUM_PROFILE_SCOPE_TIMER( __VA_ARGS__ )
#else
#define HELIUM_PERSIST_SCOPE_TIMER( ... )
#endif