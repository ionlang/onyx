#pragma once

#define ILC_DIAGNOSTIC_PRINTER_DEFAULT_GRACE 2

#include <string>
#include <vector>
#include <ionshared/diagnostics/diagnostic.h>
#include <ionlang/lexical/token.h>
#include <ilc/misc/helpers.h>

namespace ilc {
    struct CodeBlockLine {
        std::string text{};

        std::vector<ionlang::Token> tokens{};

        std::optional<uint32_t> lineNumber{std::nullopt};

        bool colors{false};

        std::optional<ionshared::Span> underline{std::nullopt};
    };

    typedef std::vector<CodeBlockLine> CodeBlock;

    /**
     * A print result. The first item on the pair represents the resulting
     * string, and the second the amount of error-like diagnostics encountered.
     */
    typedef std::pair<std::optional<std::string>, uint32_t> DiagnosticStackTraceResult;

    struct DiagnosticPrinterOpts {
        const std::string input;

        ionlang::TokenStream tokenStream;

        // TODO: Turning off colors by default to debug output, since CLion console doesn't support colors.
        const bool colors = false;
    };

    class DiagnosticPrinter {
    private:
        static std::string makeGutter(std::optional<uint32_t> lineNumber);

        static std::string makeCodeBlockLine(CodeBlockLine codeBlockLine);

        static std::optional<std::string> makeCodeBlock(
            std::vector<CodeBlockLine> codeBlock,
            bool colors = true
        );

        [[nodiscard]] static std::string makeDiagnosticKindText(
            ionshared::DiagnosticKind kind,
            std::string text
        );

        [[nodiscard]] static std::string resolveInputText(
            const std::string& input,
            std::vector<ionlang::Token> lineBuffer
        );

        [[nodiscard]] static std::string createTraceHeader(
            ionshared::Diagnostic diagnostic
        ) noexcept;

        std::optional<CodeBlock> createCodeBlockNear(
            const uint32_t lineNumber,
            ionshared::Span column,
            const uint32_t grace = ILC_DIAGNOSTIC_PRINTER_DEFAULT_GRACE
        );

        std::optional<CodeBlock> createCodeBlockNear(
            const ionlang::Token& token,
            uint32_t grace = ILC_DIAGNOSTIC_PRINTER_DEFAULT_GRACE
        );

        std::optional<CodeBlock> createCodeBlockNear(
            const ionshared::SourceLocation& sourceLocation,
            uint32_t grace = ILC_DIAGNOSTIC_PRINTER_DEFAULT_GRACE
        );

        std::optional<CodeBlock> createCodeBlockNear(
            const ionshared::Diagnostic& diagnostic,
            uint32_t grace = ILC_DIAGNOSTIC_PRINTER_DEFAULT_GRACE
        );

        std::string createTraceBody(
            ionshared::Diagnostic diagnostic,
            bool isPrime
        );

    public:
        const DiagnosticPrinterOpts opts;

        explicit DiagnosticPrinter(DiagnosticPrinterOpts opts);

        DiagnosticStackTraceResult createDiagnosticStackTrace(
            std::shared_ptr<DiagnosticVector> diagnostics
        );

        bool printDiagnosticStackTrace(
            std::shared_ptr<DiagnosticVector> diagnostics
        );
    };
}
