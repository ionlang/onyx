#pragma once

#include <filesystem>
#include <vector>
#include <llvm/ADT/Triple.h>
#include <llvm/IR/Module.h>
#include <ionshared/misc/helpers.h>
#include <ionlang/lexical/token.h>
#include <ionlang/construct/module.h>
#include <ilc/misc/helpers.h>

namespace ilc {
    class Driver {
    private:
        std::filesystem::path outputFilePath{};

        std::string input{};

        std::optional<ionlang::TokenStream> tokenStream{};

        std::vector<ionlang::Token> lex();

        ionshared::OptPtr<ionlang::Module> parse(
            std::vector<ionlang::Token> tokens,
            std::shared_ptr<DiagnosticVector> diagnostics
        );

        std::optional<std::vector<llvm::Module*>> lowerToLlvmIr(
            std::shared_ptr<ionlang::Module> module,
            std::shared_ptr<DiagnosticVector> diagnostics
        );

        bool writeObjectFile(llvm::Module* module);

        void tryThrow(std::exception exception);

    public:
        Driver() noexcept;

        bool link(std::vector<std::filesystem::path> objectFilePaths);

        /**
         * Proceed to lex, parse, lower, and emit to either LLVM
         * IR or object code. However, the linker will not be invoked
         * automatically. Returns true if successful, and false
         * otherwise.
         */
        bool process(
            std::filesystem::path outputFilePath,
            std::string input
        );
    };
}
