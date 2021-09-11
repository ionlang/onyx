#pragma once

#include <vector>
#include <ionshared/helpers.h>
#include <ionlang/lexical/token.h>
#include <ionlang/construct/namespace.h>
#include <ilc/helpers.h>

namespace ilc {
    class JitDriver {
    private:
        std::string input{};

        std::optional<ionlang::TokenStream> tokenStream{};

        std::vector<ionlang::Token> lex();

        ionshared::OptPtr<ionlang::Namespace> parse(
            std::vector<ionlang::Token> tokens,
            std::shared_ptr<DiagnosticVector> diagnostics
        );

        void codegen(
            std::shared_ptr<ionlang::Namespace> ast,
            std::shared_ptr<DiagnosticVector> diagnostics
        );

        void tryThrow(std::exception exception);

    public:
        JitDriver() noexcept;

        void run(std::string input);
    };
}
