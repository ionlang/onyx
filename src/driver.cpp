#include <memory>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/Triple.h>
#include <llvm/CodeGen/CommandFlags.inc>
#include <llvm/CodeGen/LinkAllCodegenComponents.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
#include <llvm/CodeGen/TargetPassConfig.h>
#include <llvm/CodeGen/TargetSubtargetInfo.h>
#include <llvm/IR/DiagnosticInfo.h>
#include <llvm/IR/DiagnosticPrinter.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/RemarkStreamer.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <ionshared/diagnostics/diagnostic.h>
#include <ionshared/llvm/llvm_module.h>
#include <ionir/passes/lowering/llvm_lowering_pass.h>
#include <ionir/passes/type_system/type_check_pass.h>
#include <ionir/passes/type_system/borrow_check_pass.h>
#include <ionir/passes/semantic/entry_point_check_pass.h>
#include <ionlang/passes/lowering/ionir_lowering_pass.h>
#include <ionlang/passes/semantic/macro_expansion_pass.h>
#include <ionlang/passes/semantic/name_resolution_pass.h>
#include <ionlang/lexical/lexer.h>
#include <ionlang/syntax/parser.h>
#include <ilc/passes/ionlang/ionlang_logger_pass.h>
#include <ilc/diagnostics/diagnostic_printer.h>
#include <ilc/cli/log.h>
#include <ilc/processing/linker_invoker.h>
#include <ilc/processing/driver.h>

namespace ilc {
    std::vector<ionlang::Token> Driver::lex() {
        ionlang::Lexer lexer{this->input};
        std::vector<ionlang::Token> tokens = lexer.scan();

        std::cout
            << ConsoleColor::coat(
                "--- Lexer: " + std::to_string(tokens.size()) + " token(s) ---",
                ColorKind::ForegroundGreen
            )

            << std::endl;

        size_t counter = 0;

        for (auto& token : tokens) {
            // TODO: Only do if specified by option 'trim'.
            if (counter == 10) {
                std::cout << ConsoleColor::coat("... trimmed ...", ColorKind::ForegroundGray)
                    << std::endl;

                break;
            }

            std::cout << token << std::endl;
            counter++;
        }

        return tokens;
    }

    ionshared::OptPtr<ionlang::Module> Driver::parse(
        std::vector<ionlang::Token> tokens,
        std::shared_ptr<DiagnosticVector> diagnostics
    ) {
        ionlang::TokenStream tokenStream{tokens};

        ionlang::Parser parser{
            tokenStream,
            std::make_shared<ionshared::DiagnosticBuilder>(diagnostics)
        };

        this->tokenStream = tokenStream;

        try {
            ionlang::AstPtrResult<ionlang::Module> moduleResult = parser.parseModule();

            // TODO: Improve if block?
            if (ionlang::util::hasValue(moduleResult)) {
                // TODO: What if multiple top-level, in-line constructs are parsed? (Additional note below).
                std::cout << ConsoleColor::coat("--- Parser ---", ColorKind::ForegroundGreen)
                    << std::endl;

                return ionlang::util::getResultValue(moduleResult);
            }

            DiagnosticPrinter diagnosticPrinter{DiagnosticPrinterOpts{
                this->input,
                tokenStream
            }};

            DiagnosticStackTraceResult printResult =
                diagnosticPrinter.createDiagnosticStackTrace(diagnostics);

            // TODO: Check for null ->make().
            if (printResult.first.has_value()) {
                std::cout << *printResult.first;
                std::cout.flush();
            }
            else {
                log::error("Could not create stack-trace");
            }
        }
        catch (std::exception& exception) {
            log::error("Parser: " + std::string(exception.what()));
            this->tryThrow(exception);
        }

        return std::nullopt;
    }

    std::optional<std::vector<llvm::Module*>> Driver::lowerToLlvmIr(
        std::shared_ptr<ionlang::Module> module,
        std::shared_ptr<DiagnosticVector> diagnostics
    ) {
        try {
            // TODO: Creating mock AST?
            ionlang::Ast ionLangAst{
                module
            };

            /**
             * Create a pass manager instance & run applicable passes
             * over the resulting AST.
             */
            ionlang::PassManager ionLangPassManager{};

            std::shared_ptr<ionshared::PassContext> passContext =
                std::make_shared<ionshared::PassContext>(diagnostics);

            // Register all passes to be used by the pass manager.
            // TODO: Create and implement IonLangLogger pass.
            if (cli::options.passes.contains(cli::PassKind::IonLangLogger)) {
                ionLangPassManager.registerPass(
                    std::make_shared<IonLangLoggerPass>(passContext)
                );
            }

            if (cli::options.passes.contains(cli::PassKind::MacroExpansion)) {
                ionLangPassManager.registerPass(
                    std::make_shared<ionlang::MacroExpansionPass>(passContext)
                );
            }

            if (cli::options.passes.contains(cli::PassKind::NameResolution)) {
                ionLangPassManager.registerPass(
                    std::make_shared<ionlang::NameResolutionPass>(passContext)
                );
            }

            if (!ionLangPassManager.passes.empty()) {
                log::verbose("Running "
                    + std::to_string(ionLangPassManager.passes.size())
                    + " Ion pass(es)"
                );
            }

            // Execute the pass manager against the parser's resulting AST.
            ionLangPassManager.run(ionLangAst);

            // TODO: CRITICAL: Should be used with the PassManager instance, as a normal pass instead of manually invoking the visit functions.
            ionlang::IonIrLoweringPass ionIrLoweringPass{passContext};

            // TODO: What if multiple top-level constructs are defined in-line? Use ionir::Driver (finish it first) and use its resulting Ast. (Additional note above).
            // Visit the parsed module construct.
            ionIrLoweringPass.visitModule(module);

            // TODO: Verify a module exists/was emitted, before retrieving it.

            ionshared::OptPtr<ionir::Module> ionIrModuleBuffer =
                ionIrLoweringPass.getModules()->unwrap().begin()->second;

            if (!ionshared::util::hasValue(ionIrModuleBuffer)) {
                throw std::runtime_error("Module is nullptr");
            }

            ionir::Ast ionIrAst{
                *ionIrModuleBuffer
            };

            ionir::PassManager ionIrPassManager{};

            // Register passes.
            if (cli::options.passes.contains(cli::PassKind::EntryPointCheck)) {
                ionIrPassManager.registerPass(
                    std::make_shared<ionir::EntryPointCheckPass>(passContext)
                );
            }

            if (cli::options.passes.contains(cli::PassKind::TypeChecking)) {
                ionIrPassManager.registerPass(
                    std::make_shared<ionir::TypeCheckPass>(passContext)
                );
            }

            if (cli::options.passes.contains(cli::PassKind::BorrowCheck)) {
                ionIrPassManager.registerPass(
                    std::make_shared<ionir::BorrowCheckPass>(passContext)
                );
            }

            if (!ionIrPassManager.passes.empty()) {
                log::verbose("Running "
                    + std::to_string(ionIrPassManager.passes.size())
                    + " IonIR pass(es)"
                );
            }

            // Run the pass manager on the IonIR AST.
            ionIrPassManager.run(ionIrAst);

            DiagnosticPrinter diagnosticPrinter{DiagnosticPrinterOpts{
                this->input,
                *this->tokenStream
            }};

            DiagnosticStackTraceResult diagnosticStackTraceResult =
                diagnosticPrinter.createDiagnosticStackTrace(diagnostics);

            // TODO: Blocking multi-modules?
            if (diagnosticStackTraceResult.second > 0) {
                std::cout << " --- LLVM code-generation: "
                    << diagnosticStackTraceResult.second
                    << " error(s) encountered ---"
                    << std::endl;

                diagnosticPrinter.printDiagnosticStackTrace(diagnostics);

                return std::nullopt;
            }

            // TODO: Where should optimization passes occur? Before or after type-checking?
//            ionirPassManager.registerPass(std::make_shared<ionir::DeadCodeEliminationPass>());

            // Now, make the ionir::LlvmCodegenPass.
            ionir::LlvmLoweringPass ionIrLlvmLoweringPass{passContext};

            // Visit the resulting IonIR module buffer from the IonLang codegen pass.
            ionIrLlvmLoweringPass.visitModule(*ionIrModuleBuffer);

            std::map<std::string, std::shared_ptr<llvm::Module>> modules =
                ionIrLlvmLoweringPass.llvmModules->unwrap();

            if (modules.empty()) {
                std::cout
                    << ConsoleColor::coat(
                        "--- LLVM code-generation contained no modules ---",
                        ColorKind::ForegroundGreen
                    )

                    << std::endl;

                return std::nullopt;
            }

            std::vector<llvm::Module*> result{};

            // Display the resulting code of all the modules.
            for (const auto& [key, value] : modules) {
                std::cout
                    << ConsoleColor::coat(
                        "--- LLVM code-generation: " + key + " ---",
                        ColorKind::ForegroundGreen
                    )

                    << std::endl;

                ionshared::LlvmModule llvmModule{value.get()};

                llvmModule.printIr();
                result.push_back(value.get());
            }

            return result;
        }
        catch (std::exception& exception) {
            log::error("LLVM code-generation: " + std::string(exception.what()));
            this->tryThrow(exception);
        }

        return std::nullopt;
    }

    bool Driver::writeObjectFile(llvm::Module* module) {
        // Initialize targets for emitting object code.
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        llvm::Triple targetTriple{cli::options.target};
        std::string error{};

        module->setTargetTriple(targetTriple.getTriple());

        const llvm::Target* target = llvm::TargetRegistry::lookupTarget(
            targetTriple.getTriple(),
            error
        );

        /**
         * The requested target could not be found. This might occur if
         * the target registry was not previously initialized, or if a
         * bogus target triple was provided.
         */
        if (!target) {
            log::error("Could not lookup target: " + error);

            return false;
        }

        std::string cpuName = llvm::sys::getHostCPUName();

        // Build CPU features.
        SubtargetFeatures subtargetFeatures{};
        StringMap<bool> hostFeatures{};

        if (sys::getHostCPUFeatures(hostFeatures)) {
            for (auto& feature : hostFeatures) {
                subtargetFeatures.AddFeature(feature.first(), feature.second);
            }
        }

        std::string cpuFeatures = subtargetFeatures.getString();
        llvm::TargetOptions targetOptions{};
        llvm::Optional<llvm::Reloc::Model> relocationModel{};

        std::shared_ptr<llvm::TargetMachine> targetMachine{
            target->createTargetMachine(
                targetTriple.getTriple(),
                cpuName,
                cpuFeatures,
                targetOptions,
                relocationModel
            )
        };

        /**
         * Configure the module's data layout and target triple
         * for optimization benefits (performance). Optimizations
         * benefit from knowing about the target triple and data
         * layout.
         */
         // TODO: ->createDataLayout() is causing SIGSEGV.
//        llvmModule->setDataLayout(targetMachine->createDataLayout());
//        llvmModule->setTargetTriple(targetTriple.getTriple());

        std::error_code errorCode{};

        llvm::raw_fd_ostream destination{
            this->outputFilePath.string(),
            errorCode,
            llvm::sys::fs::OF_None
        };

        if (errorCode) {
            log::error("Could not open output file: " + errorCode.message());

            return false;
        }

        llvm::legacy::PassManager passManager{};

        llvm::CodeGenFileType outputFileType =
            llvm::CodeGenFileType::CGFT_ObjectFile;

        // NOTE: Returns true upon failure.
        bool failed = targetMachine->addPassesToEmitFile(
            passManager,
            destination,
            nullptr,
            outputFileType
        );

        if (failed) {
            log::error("LLVM cannot emit this type of file");

            return false;
        }

        passManager.run(*module);
        destination.flush();

        return true;
    }

    Driver::Driver() noexcept :
        outputFilePath(),
        input(),
        tokenStream(std::nullopt) {
        //
    }

    bool Driver::link(std::vector<std::filesystem::path> objectFilePaths) {
        log::verbose("Linking " + std::to_string(objectFilePaths.size()) + " file(s)");

        LinkerInvoker linkerInvoker{objectFilePaths};

        // TODO: Pass the linker kind from options.
        std::optional<int> invocationResult = linkerInvoker.invoke(LinkerKind::GCC);

        return invocationResult.has_value() && *invocationResult == EXIT_SUCCESS;
    }

    void Driver::tryThrow(std::exception exception) {
        if (cli::options.doJitThrow) {
            throw exception;
        }
    }

    bool Driver::process(
        std::filesystem::path outputFilePath,
        std::string input
    ) {
        this->outputFilePath = outputFilePath;
        this->input = input;

        std::vector<ionlang::Token> tokens = this->lex();

        std::shared_ptr<DiagnosticVector> diagnostics =
            std::make_shared<DiagnosticVector>();

        ionshared::OptPtr<ionlang::Module> ionLangModules = this->parse(tokens, diagnostics);

        if (!ionshared::util::hasValue(ionLangModules)) {
            return false;
        }
        // TODO: Throwing SIGSEGV (nullptr). This may be due to cli::astCommand not being a smart pointer and going out of scope.
        // AST command was parsed. Only print AST then exit.
//        else if (cli::astCommand->parsed()) {
//            ionlang::Ast ionLangAst = {
//                *ionLangModules
//            };
//
//            ionlang::PassManager ionLangPassManager = ionlang::PassManager();
//
//            std::shared_ptr<ionshared::PassContext> passContext =
//                std::make_shared<ionshared::PassContext>();
//
//            ionLangPassManager.registerPass(
//                std::make_shared<IonLangPrinterPass>(passContext)
//            );
//
//            ionLangPassManager.run(ionLangAst);
//
//            return true;
//        }

        std::optional<std::vector<llvm::Module*>> llvmModules =
            this->lowerToLlvmIr(*ionLangModules, diagnostics);

        if (!llvmModules.has_value() || llvmModules->empty()) {
            return false;
        }

        // TODO: Processing only first module until implemented support for multiple (consider multiple modules inside a single file).
        return this->writeObjectFile(llvmModules.value()[0]);
    }
}
