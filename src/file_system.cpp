#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <ilc/misc/file_system.h>

namespace ilc {
    bool FileSystem::doesPathExist(const std::string& name) {
        struct stat buffer{};

        return stat(name.c_str(), &buffer) == 0;
    }

    std::optional<std::string> FileSystem::readFileContents(std::string path) {
        // The provided path does not exist, do not continue.
        if (!FileSystem::doesPathExist(path)) {
            return std::nullopt;
        }

        std::ifstream stream{path};
        std::stringstream buffer{};

        buffer << stream.rdbuf();

        // Consume the buffer as a string and return it.
        return buffer.str();
    }
}
