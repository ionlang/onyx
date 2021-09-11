#pragma once

#include <ilc/misc/helpers.h>

namespace ilc::util {
    static constexpr bool isPlatformUnixLike =
        #if defined(OS_WINDOWS)
            false;
        #else
            true;
        #endif

    int execute(std::string command);
}
