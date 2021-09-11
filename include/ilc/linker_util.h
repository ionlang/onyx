#pragma once

#include <string>
#include "linker_kind.h"

namespace ilc::linker_util {
    bool isLinkerKindUnixLike(LinkerKind linkerKind) noexcept {
        // NOTE: Custom linker kind will always default to Unix-style.
        return linkerKind != LinkerKind::MSVC;
    }

    std::string getLinkerKindPrefix(
        LinkerKind linkerKind,
        bool isShortOption
    ) noexcept {
        return linker_util::isLinkerKindUnixLike(linkerKind)
            ? (isShortOption ? "-" : "--")
            : "/";
    }
}
