#pragma once

#include <vector>
#include <optional>
#include <string>
#include <filesystem>
#include "linker_kind.h"

namespace ilc {
    class LinkerInvoker {
    private:
        [[nodiscard]] std::string prepareGccArguments();

        [[nodiscard]] std::optional<std::string> prepareArguments(
            LinkerKind linkerKind
        );

    public:
        const std::vector<std::filesystem::path> objectFilePaths{};

        explicit LinkerInvoker(
            std::vector<std::filesystem::path> objectFilePaths
        ) noexcept;

        std::optional<int> invoke(
            LinkerKind linkerKind
        );
    };
}
