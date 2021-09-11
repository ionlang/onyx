#include <ilc/processing/linker_finder.h>

namespace ilc {
    std::string LinkerFinder::findGcc() {
        return "gcc";
    }

    std::optional<std::string> LinkerFinder::find(LinkerKind linkerKind) {
        // TODO: Verify that the path or command exists.

        switch (linkerKind) {
            case LinkerKind::GCC: {
                return LinkerFinder::findGcc();
            }

            default: {
                return std::nullopt;
            }
        }
    }
}
