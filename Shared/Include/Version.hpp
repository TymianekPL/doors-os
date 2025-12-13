#pragma once

#include <cstddef>

#define STRING(x) #x			// NOLINT(cppcoreguidelines-macro-usage)
#define EXPAND_STRING(e) STRING(e) // NOLINT(cppcoreguidelines-macro-usage)

#define DOORS_VERSION_MAJOR 0	   // NOLINT(cppcoreguidelines-macro-to-enum,modernize-macro-to-enum) NOLINT(cppcoreguidelines-macro-usage)
#define DOORS_VERSION_MINOR 0	   // NOLINT(cppcoreguidelines-macro-to-enum,modernize-macro-to-enum) NOLINT(cppcoreguidelines-macro-usage)
#define DOORS_VERSION_REVISION 1 // NOLINT(cppcoreguidelines-macro-to-enum,modernize-macro-to-enum) NOLINT(cppcoreguidelines-macro-usage)
#define DOORS_VERSION_STRING                                                                                                               \
	EXPAND_STRING(DOORS_VERSION_MAJOR)                                                                                                    \
	"." EXPAND_STRING(DOORS_VERSION_MINOR) "." EXPAND_STRING(DOORS_VERSION_REVISION) // NOLINT(cppcoreguidelines-macro-usage)

namespace Doors
{
	constexpr int VersionMajor = DOORS_VERSION_MAJOR;
	constexpr int VersionMinor = DOORS_VERSION_MINOR;
	constexpr int VersionRevision = DOORS_VERSION_REVISION;
} // namespace Doors
