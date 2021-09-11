#include <sstream>
#include <ionlang/const/const_name.h>
#include <ionlang/misc/util.h>
#include <ilc/cli/log.h>
#include <ilc/diagnostics/code_highlight.h>
#include <ilc/diagnostics/diagnostic_printer.h>

#define ILC_DIAGNOSTICS_TAB "   "

namespace ilc {
    std::string DiagnosticPrinter::makeGutter(std::optional<uint32_t> lineNumber) {
        /**
         * NOTE: If the separator is modified, and its length changes,
         * the gutter separator length constant defined on top of
         * this file must be updated as well.
         */
        return (lineNumber.has_value() ? std::to_string(*lineNumber) : " ") + " | ";
    }

    std::string DiagnosticPrinter::makeCodeBlockLine(CodeBlockLine codeBlockLine) {
        std::stringstream stringStream;
        std::string gutterText = DiagnosticPrinter::makeGutter(codeBlockLine.lineNumber);

        stringStream << ILC_DIAGNOSTICS_TAB
            + gutterText
            + codeBlockLine.text
            + "\n";

        if (codeBlockLine.underline.has_value()) {
            // TODO: Investigate offset & its edge cases.
            // TODO: Width must be the LARGEST/MAX length of line.
            /**
             * Start the new line with the same indentation (a single tab),
             * and additionally add a width to account for the gutter text.
             * An offset of 1 is added to the length.
             */
            stringStream << ILC_DIAGNOSTICS_TAB << std::setw(gutterText.length() + 1);

            // Fill in the underline with denoting character(s).
            for (uint32_t i = 0; i < codeBlockLine.underline.value().getEndPosition(); i++) {
                stringStream << "^";
            }

            stringStream << "\n";
        }

        return stringStream.str();
    }

    std::optional<std::string> DiagnosticPrinter::makeCodeBlock(
        std::vector<CodeBlockLine> codeBlock,
        const bool colors
    ) {
        if (codeBlock.empty()) {
            return std::nullopt;
        }

        std::stringstream result;

        // TODO: Is mutating the line here a good idea? Will it change the original? It should only mutate to return here.
        for (auto& line : codeBlock) {
            if (colors) {
                // Entire code should be highlighted gray by default.
                // TODO: Actually use gray color.
                // TODO: This messes up the ranges below, because since the color codes are inserted, the ranges are messed up?
//                line.text = ConsoleColor::white(line.text);

                // Apply syntax highlighting to the line's applicable token(s).
                for (const auto& token : line.tokens) {
                    /**
                     * Note that value coating is not bound to occur; the value
                     * may remain the same. It depends on the token's kind.
                     */
                    const std::string highlightedText = CodeHighlight::coat(token);

                    /**
                     * Split the line's text into two halves, excluding the token's
                     * value, in order to insert coated text.
                     */
                    const std::string firstHalf = line.text.substr(0, token.startPosition);
                    uint32_t secondHalfEndPosition = token.getEndPosition() - 1;
                    std::string secondHalf = line.text.substr(secondHalfEndPosition);

                    /**
                     * If the end position of the token - 1 is 0, this means that
                     * the line's text is only 1 character long. Make the second
                     * half an empty string to avoid taking a substring starting
                     * from position 0 (which means that the whole string will be
                     * included).
                     */
                    if (secondHalfEndPosition == 0) {
                        secondHalf = "";
                    }

                    // Replace the line's text with the same, possibly highlighted text.
                    line.text = firstHalf + highlightedText + secondHalf;
                }
            }

            result << DiagnosticPrinter::makeCodeBlockLine(line);
        }

        return result.str();
    }

    std::string DiagnosticPrinter::makeDiagnosticKindText(
        ionshared::DiagnosticKind kind,
        std::string text
    ) {
        log::LogLevel logLevel;

        switch (kind) {
            // TODO: Using fallback (not directly mapped).
            case ionshared::DiagnosticKind::Fatal: {
                //
            }

            // TODO: Using fallback (not directly mapped).
            case ionshared::DiagnosticKind::InternalError: {
                //
            }

            case ionshared::DiagnosticKind::Error: {
                logLevel = log::LogLevel::Error;

                break;
            }

            case ionshared::DiagnosticKind::Info: {
                logLevel = log::LogLevel::Info;

                break;
            }

            case ionshared::DiagnosticKind::Warning: {
                logLevel = log::LogLevel::Warning;

                break;
            }

            default: {
                throw std::runtime_error("Unknown diagnostic kind");
            }
        }

        // NOTE: The result is returned with a newline character.
        return log::makeLogTemplate(logLevel, (ColorKind)logLevel, text).str();
    }

    std::string DiagnosticPrinter::resolveInputText(
        const std::string& input,
        std::vector<ionlang::Token> lineBuffer
    ) {
        if (lineBuffer.empty() || input.empty()) {
            throw std::invalid_argument("Both input and line buffer arguments must contain value(s)");
        }

        return lineBuffer.size() > 1
            ? input.substr(lineBuffer[0].startPosition, lineBuffer[lineBuffer.size() - 1].getEndPosition())
            : lineBuffer[0].value;
    }

    std::string DiagnosticPrinter::createTraceHeader(
        ionshared::Diagnostic diagnostic
    ) noexcept {
        std::stringstream traceHeader;

        // NOTE: The result is returned with a newline character.
        traceHeader << DiagnosticPrinter::makeDiagnosticKindText(
            diagnostic.kind,
            diagnostic.message
        );

        if (diagnostic.sourceLocation.has_value()) {
            ionshared::SourceLocation sourceLocation = diagnostic.sourceLocation.value();

            // TODO: File path.
            traceHeader << ILC_DIAGNOSTICS_TAB
                << "@ "
                << ionlang::const_name::unknown
                << ":"
                << sourceLocation.lines.startPosition
                << "-"
                << sourceLocation.lines.getEndPosition()
                << ":"
                << sourceLocation.column.startPosition
                << "-"
                << sourceLocation.column.getEndPosition()
                << "\n";
        }

        return traceHeader.str();
    }

    DiagnosticPrinter::DiagnosticPrinter(DiagnosticPrinterOpts opts) :
        opts(opts) {
        //
    }

    std::optional<CodeBlock> DiagnosticPrinter::createCodeBlockNear(
        const uint32_t lineNumber,
        ionshared::Span column,
        const uint32_t grace
    ) {
        CodeBlock codeBlock{};
        ionlang::TokenStream tokenStream = this->opts.tokenStream;

        tokenStream.begin();

        // Compute start & end line for the code block.
        const uint32_t start = grace >= lineNumber ? 0 : lineNumber - grace;
        const uint32_t end = lineNumber + grace;

        uint32_t lineNumberCounter = 0;
        std::optional<ionlang::Token> tokenBuffer = std::nullopt;
        std::vector<ionlang::Token> lineBuffer{};
        bool prime = true;
        bool met = false;

        auto buildCodeBlockLine = [&](bool underline) -> CodeBlockLine {
            CodeBlockLine codeBlockLine = CodeBlockLine{
                DiagnosticPrinter::resolveInputText(this->opts.input, lineBuffer),
                lineBuffer,
                lineNumber,
                this->opts.colors
            };

            if (underline) {
                codeBlockLine.underline = column;
            }

            return codeBlockLine;
        };

        while (lineNumberCounter != start) {
            if (!tokenStream.hasNext()) {
                // Could not reach starting point.
                return std::nullopt;
            }

            tokenBuffer = tokenStream.next();

            if (tokenBuffer->lineNumber != lineNumberCounter) {
                lineNumberCounter = tokenBuffer->lineNumber;
            }
        }

        while (lineNumberCounter != end) {
            met = lineNumberCounter >= lineNumber;

            if (!prime) {
                tokenBuffer = tokenStream.next();
            }
            else {
                tokenBuffer = tokenStream.get();
                prime = false;
            }

            lineBuffer.push_back(*tokenBuffer);

            uint32_t nextLineNumber = tokenStream.peek()->lineNumber;

            if (nextLineNumber != lineNumberCounter) {
                codeBlock.push_back(
                    /**
                     * If the current line number is the provided problematic line
                     * number, instruct the code block's line to underline the
                     * problematic column span.
                     */
                    buildCodeBlockLine(lineNumberCounter == lineNumber)
                );

                lineBuffer.clear();
                lineNumberCounter = nextLineNumber;

                continue;
            }

            bool streamHasNext = tokenStream.hasNext();

            // Could not reach end point.
            if (!streamHasNext && !met) {
                return std::nullopt;
            }
            // Return requirements have been met. Do not continue.
            else if (!streamHasNext) {
                // At this point, the code block line will always be underlined.
                codeBlock.push_back(buildCodeBlockLine(true));

                return codeBlock;
            }
        }

        return codeBlock;
    }

    std::optional<CodeBlock> DiagnosticPrinter::createCodeBlockNear(
        const ionlang::Token& token,
        uint32_t grace
    ) {
        return this->createCodeBlockNear(
            token.lineNumber,

            ionshared::Span{
                token.startPosition,
                token.getEndPosition() - token.startPosition
            },

            grace
        );
    }

    std::optional<CodeBlock> DiagnosticPrinter::createCodeBlockNear(
        const ionshared::SourceLocation& sourceLocation,
        uint32_t grace
    ) {
        // TODO: Use start AND end line positions.
        return this->createCodeBlockNear(
            sourceLocation.lines.startPosition,
            sourceLocation.column,
            grace
        );
    }

    std::optional<CodeBlock> DiagnosticPrinter::createCodeBlockNear(
        const ionshared::Diagnostic& diagnostic,
        uint32_t grace
    ) {
        if (!diagnostic.sourceLocation.has_value()) {
            return std::nullopt;
        }

        return this->createCodeBlockNear(*diagnostic.sourceLocation, grace);
    }

    std::string DiagnosticPrinter::createTraceBody(
        ionshared::Diagnostic diagnostic,
        bool isPrime
    ) {
        std::stringstream traceBody{};

        if (!isPrime) {
            traceBody << ILC_DIAGNOSTICS_TAB << "at ";
        }
        else {
            std::optional<CodeBlock> codeBlock =
                this->createCodeBlockNear(diagnostic);

            if (!codeBlock.has_value()) {
                throw std::runtime_error("Unexpected code block to be null");
            }

            std::optional<std::string> codeBlockString = DiagnosticPrinter::makeCodeBlock(
                *codeBlock,
                this->opts.colors
            );

            if (!codeBlockString.has_value()) {
                throw std::runtime_error("Unexpected code block string to be null");
            }

            traceBody << *codeBlockString;
        }

        return traceBody.str();
    }

    DiagnosticStackTraceResult DiagnosticPrinter::createDiagnosticStackTrace(
        std::shared_ptr<DiagnosticVector> diagnostics
    ) {
        DiagnosticStackTraceResult result = std::make_pair(std::nullopt, 0);

        if (diagnostics->isEmpty()) {
            return result;
        }

        std::stringstream stringStream{};
        bool isPrime = true;

        // TODO: Verify it's actually copied (debug and inspect).
        // Stack is copied upon assignment operator usage.
        std::vector<ionshared::Diagnostic> diagnosticsNativeVector = diagnostics->unwrap();

        // TODO: Variable 'longestLineNumberDigits' needed to calculate extra prefix spaces. Loop through the diagnostics and find the highest line number.

        for (const auto& diagnostic : diagnosticsNativeVector) {
            bool isErrorLike = diagnostic.kind == ionshared::DiagnosticKind::Error
                || diagnostic.kind == ionshared::DiagnosticKind::Fatal
                || diagnostic.kind == ionshared::DiagnosticKind::InternalError;

            // Increment the error-like counter on the result if applicable.
            if (isErrorLike) {
                result.second++;
            }

            stringStream << DiagnosticPrinter::createTraceHeader(diagnostic);

            if (diagnostic.sourceLocation.has_value()) {
                // NOTE: This extra space will separate the header and the body by 1 space.
                stringStream << "\n" << this->createTraceBody(diagnostic, isPrime);
            }

            // Raise the prime flag to take effect upon next iteration.
            isPrime = false;
        }

        /**
         * Finally, if highlight was specified, append a reset instruction
         * at the end to clear applied formatting.
         */
        if (this->opts.colors) {
            stringStream << ConsoleColor::reset;
        }

        result.first = stringStream.str();

        return result;
    }

    bool DiagnosticPrinter::printDiagnosticStackTrace(
        std::shared_ptr<DiagnosticVector> diagnostics
    ) {
        DiagnosticStackTraceResult diagnosticStackTraceResult =
            this->createDiagnosticStackTrace(diagnostics);

        if (diagnosticStackTraceResult.second == 0 || !diagnosticStackTraceResult.first.has_value()) {
            return false;
        }

        std::cout << *diagnosticStackTraceResult.first;

        return true;
    }
}
