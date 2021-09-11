#pragma once

#include <optional>
#include <string>
#include "linker_kind.h"

namespace ilc {
    class LinkerFinder {
    private:
        static std::string findGcc();

    public:
        static std::optional<std::string> find(LinkerKind linkerKind);
    };
}
