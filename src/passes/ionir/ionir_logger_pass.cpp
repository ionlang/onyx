#include <iostream>
#include <ilc/passes/ionir/ionir_logger_pass.h>
#include <ilc/cli/log.h>
#include <ionir/const.h>

namespace ilc {
    IonIrLoggerPass::IonIrLoggerPass(
        std::shared_ptr<ionshared::PassContext> context
    ) :
        ionir::Pass(std::move(context)) {
        //
    }

    void IonIrLoggerPass::visit(std::shared_ptr<ionir::Construct> node) {
        ionir::ConstructKind constructKind = node->constructKind;
        std::optional<std::string> constructName = ionir::Const::getConstructKindName(constructKind);
        std::string defaultName = "Unknown (" + std::to_string((int)constructKind) + ")";
        std::string addressString = " [" + ionshared::util::getPointerAddressString(node.get()) + "]";

        std::cout << "Visiting: "
            << constructName.value_or(defaultName)
            << addressString
            << std::endl;

        ionir::Pass::visit(node);
    }
}
