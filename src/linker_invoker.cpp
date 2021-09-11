#include <ilc/cli/log.h>
#include <ilc/processing/linker_invoker.h>
#include <ilc/processing/linker_argument_builder.h>
#include <ilc/processing/linker_finder.h>
#include <ilc/misc/util.h>

namespace ilc {
    LinkerInvoker::LinkerInvoker(
        std::vector<std::filesystem::path> objectFilePaths
    ) noexcept :
        objectFilePaths(objectFilePaths) {
        //
    }

    std::string LinkerInvoker::prepareGccArguments() {
        LinkerArgumentBuilder linkerArgumentBuilder{LinkerKind::GCC};

        // NOTE: The 'o' option specifies the output file name.
        linkerArgumentBuilder.addOption(
            "o",

            std::filesystem::path(cli::options.outputDirectoryPath)
                .append(cli::options.outputExecutablePath)
                .string(),

            true
        );

        for (const auto objectFilePath : this->objectFilePaths) {
            linkerArgumentBuilder.addPath(objectFilePath.string());
        }

        return linkerArgumentBuilder.finish();
    }

    std::optional<std::string> LinkerInvoker::prepareArguments(LinkerKind linkerKind) {
        switch (linkerKind) {
            case LinkerKind::GCC: {
                return this->prepareGccArguments();
            }

            default: {
                return std::nullopt;
            }
        }
    }

    std::optional<int> LinkerInvoker::invoke(
        LinkerKind linkerKind
    ) {
        std::optional<std::string> linkerExecutablePath =
            LinkerFinder::find(linkerKind);

        if (!linkerExecutablePath.has_value()) {
            log::error("The requested linker could not be found, or is unsupported");

            return std::nullopt;
        }

        std::optional<std::string> linkerArguments =
            this->prepareArguments(linkerKind);

        if (!linkerArguments.has_value()) {
            log::error("Could not prepare arguments for an unknown or unsupported linker");

            return std::nullopt;
        }

        std::string linkerExecutionCommand = *linkerExecutablePath + *linkerArguments;

        log::verbose("Invoking '" + linkerExecutionCommand + "'");

        // NOTE: The linker arguments string begins with a space.
        int executionExitCode = util::execute(linkerExecutionCommand);

        if (executionExitCode != 0) {
            log::error("Linking failed with code: " + std::to_string(executionExitCode));
        }

        return executionExitCode;
    }
}
