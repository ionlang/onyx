#include <ilc/passes/ionir/ionir_directive_processor_pass.h>
#include <ilc/file_system.h>

namespace ilc {
    IonIrDirectiveProcessorPass::IonIrDirectiveProcessorPass(
        std::shared_ptr<ionshared::PassContext> context,
        ionshared::OptPtr<std::stringstream> includeOutputStream
    ) noexcept :
        ionir::Pass(std::move(context)),
        includeOutputStream(includeOutputStream) {
        //
    }

    // TODO
    // void IonIrDirectiveProcessorPass::visitDirective(ionir::Directive node) {
    //     std::string directiveName = node.first;

    //     if (node.second.has_value()) {
    //         // TODO: Hard-coded string(s).
    //         if (directiveName == "include") {
    //             /**
    //              * Attempt to read the file's contents. Will return std::nullopt
    //              * if the operation fails.
    //              */
    //             std::optional<std::string> fileContents =
    //                 FileSystem::readFileContents(*node.second);

    //             // The file content's could not be read. Report an error.
    //             if (!fileContents.has_value()) {
    //                 throw std::runtime_error("Could not read file contents of provided include path. Path may not exist or be inaccessible.");
    //             }

    //             // Send the file contents through the provided output stream if applicable.
    //             if (ionshared::util::hasValue(this->includeOutputStream)) {
    //                 **this->includeOutputStream << *fileContents;
    //             }
    //         }
    //         else if (directiveName == "define") {
    //             // TODO: Implement.
    //         }
    //     }
    // }
}
