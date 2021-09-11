#include <ilc/misc/util.h>

namespace ilc::util {
    int execute(std::string command) {
        // TODO: Support for getting output and cross-platform safety.

        return std::system(command.c_str());
    }
}
