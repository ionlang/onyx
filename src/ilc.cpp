// Include the cross-platform header before anything else.
#include <ilc/cli/cross_platform.h>

#include <queue>
#include <filesystem>
#include <CLI11/CLI11.hpp>
#include <llvm/Target/TargetMachine.h>
#include <ionshared/util.h>
#include <ionlang/static_init.h>
#include <ionir/construct/type_void.h>
#include <ionir/construct/prototype.h>
#include <ilc/cli/log.h>
#include <ilc/jit_driver.h>
#include <ilc/jit.h>
#include <ilc/driver.h>
#include <ilc/cli/commands.h>
#include <ilc/util.h>

#define ILC_CLI_VERSION "1.0.0"

using namespace ilc;

void setupCli(CLI::App& app) {
  // Command(s).
  cli::astCommand = app.add_subcommand(
    "ast",
    "Trace resulting abstract syntax tree (AST)"
  );

  cli::jitCommand = app.add_subcommand(
    "jit",
    "Use JIT to compile code REPL-style"
  );

  // Option(s).
  app.add_option(
    "files",
    cli::options.inputFilePaths,
    "Input files to process"
  )
    ->check(CLI::ExistingFile);

//    app.add_option("-p,--passes", [&](std::vector<std::string> passes) {
//        cli::options.passes.clear();
//
//        for (const auto &pass : passes) {
//            cli::options.passes.insert(cli::PassKind::NameResolution);
//
//            // TODO: Use CLI11's check.
//            if (pass == "type-check") {
//                cli::options.passes.insert(cli::PassKind::TypeChecking);
//            }
////            else if (pass == "name-resolution") {
////            cli::options.passes.insert(cli::PassKind::NameResolution);
////            }
//            else if (pass == "macro-expansion") {
//                cli::options.passes.insert(cli::PassKind::MacroExpansion);
//            }
//            else if (pass == "borrow-check") {
//                cli::options.passes.insert(cli::PassKind::BorrowCheck);
//            }
//            else {
//                return false;
//            }
//        }
//
//        return true;
//    })
//        ->default_val("macro-expansion,name-resolution,type-check,borrow-check");

  // TODO: Temporary for debugging.
  app.add_flag("-n,--name-resolution-only", cli::options.temp_nameResOnly);

  app.add_option("-l,--phase-level", cli::options.phaseLevel)
    ->check(CLI::Range(0, 2))
    ->default_val(std::to_string((int)cli::PhaseLevel::CodeGeneration));

  app.add_option(
    "-d,--output-directory",
    cli::options.outputDirectoryPath,
    "The directory path where output files will be written to"
  )
    ->default_val("build");

  std::string defaultOutputExecutablePath = "program";

  if (!util::isPlatformUnixLike) {
    defaultOutputExecutablePath += ".exe";
  }

  app.add_option(
    "-o,--output-executable",
    cli::options.outputExecutablePath,
    "The output executable file name"
  )
    ->default_val(defaultOutputExecutablePath);

  app.add_option(
    "-t,--target",
    cli::options.target,
    "The target triple to pass on to LLVM"
  )
    ->default_val(llvm::sys::getDefaultTargetTriple());

  // Flag(s).
  app.add_flag(
    "-c,--no-color",
    cli::options.noColor,
    "Do not print color codes"
  );

  app.add_flag(
    "-b,--no-verbose",
    cli::options.noVerbose,
    "Omit verbose messages"
  );

  app.add_flag(
    "-r,--print-phases",
    cli::options.doPrintPhases,
    "Print phases"
  );

  app.add_flag(
    "-i,--llvm-ir",
    cli::options.doLlvmIr,
    "Whether to emit LLVM IR or LLVM bitcode"
  );

  cli::jitCommand->add_flag(
    "-w,--throw",
    cli::options.doJitThrow,
    "Throw errors instead of capturing them"
  );
}

int main(int argc, char** argv) {
  CLI::App app{"Ionlang command-line utility"};

  setupCli(app);

  // Parse arguments.
  CLI11_PARSE(app, argc, argv);

  // TODO: Temporary.
  if (cli::options.temp_nameResOnly) {
    cli::options.passes.clear();
    cli::options.passes.insert(cli::PassKind::NameResolution);
  }

  // Static initialization(s).
  ionlang::static_init::init();

  if (cli::jitCommand->parsed()) {
    jit::registerCommonActions();
    log::info("Entering REPL mode; type '\\quit' to exit");
    log::verbose("Actions registered: " + std::to_string(jit::actions.getSize()));

    std::string input{};
    JitDriver jitDriver{};

    while (true) {
      std::cout << ConsoleColor::coat("<> ", ColorKind::ForegroundGray);
      std::cout.flush();

      // If the EOF bit is on, terminate program.
      if (std::getline(std::cin, input).eof()) {
        std::cout << std::endl;
        
        return EXIT_SUCCESS;
      }
      
      // TODO: Throwing linker reference error.
      // Trim whitespace off input string.
//            input = ionshared::util::trim(input);

      // Input string was empty, continue to next prompt.
      if (input.length() == 0) {
        continue;
      }
      // An action is being specified.
      else if (input[0] == '\\') {
        std::string actionName = input.substr(1);

        if (jit::actions.contains(actionName)) {
          std::optional<Callback> action = jit::actions.lookup(actionName);

          if (!action.has_value()) {
            throw std::runtime_error("Expected action to be set");
          }

          (*action)();
        }
        else {
          log::error("Unrecognized action '" + actionName + "'; Type '\\quit' to exit");
        }

        continue;
      }

      std::cout << "--- Input: ("
        << input.length()
        << " character(s)) ---\n"
        << input
        << std::endl;

      jitDriver.run(input);
    }
  }
  else if (cli::astCommand->parsed()) {
    // TODO: Hard-coded debugging test.
    auto args = std::make_shared<ionir::Args>();
    auto returnType = std::make_shared<ionir::TypeVoid>();

    // TODO: Module is nullptr.
    auto prototype =
      std::make_shared<ionir::Prototype>("foobar", args, returnType);

    std::queue<std::shared_ptr<ionir::Construct>> childrenQueue{};

    // Push initial child.
    childrenQueue.push(prototype->nativeCast());

    // TODO: Some kind of depth counter? Currently not working exactly as intended.

    // Begin recursive children iteration.
    while (!childrenQueue.empty()) {
      childrenQueue.pop();

      std::shared_ptr<ionir::Construct> child = childrenQueue.back();
      ionir::Ast innerChildren = child->getChildrenNodes();

      std::cout << "-- "
        << child->findConstructKindName().value_or("Unknown")
        << std::endl;

      // Queue inner children if applicable.
      if (!innerChildren.empty()) {
        for (const auto innerChild : innerChildren) {
            childrenQueue.push(innerChild);
        }

        // TODO: std::cout a newline if this is a terminal node, this way we can stack '--' like a tree?
      }
    }
  }
  else if (!cli::options.inputFilePaths.empty()) {
    log::verbose("Processing " + std::to_string(cli::options.inputFilePaths.size()) + " input file(s)");

    std::stringstream inputStringStream{};
    Driver driver{};
    std::string outputFileExtension = std::string(".") + (cli::options.doLlvmIr ? "ll" : "o");

    // Create the output directory if it doesn't already exist.
    if (!std::filesystem::exists(cli::options.outputDirectoryPath)) {
      log::verbose("Creating output directory '" + cli::options.outputDirectoryPath + "'");

      // Ensure the directory was created, otherwise fail the process.
      if (!std::filesystem::create_directory(cli::options.outputDirectoryPath)) {
        log::error("Output directory could not be created");

        return EXIT_FAILURE;
      }
    }

    bool success = true;
    std::vector<std::filesystem::path> outputFilePaths{};

    log::verbose("Using target triple: " + cli::options.target);

    for (const auto& inputFilePath : cli::options.inputFilePaths) {
      inputStringStream << std::ifstream(inputFilePath).rdbuf();

      std::filesystem::path outputFilePath =
        std::filesystem::path(cli::options.outputDirectoryPath)
          .append(inputFilePath)
          .concat(outputFileExtension);

      outputFilePaths.push_back(outputFilePath);

      log::verbose("Generating '" + outputFilePath.string() + "'");

      // Stop processing input files if the driver fails to run.
      if (!driver.process(outputFilePath, inputStringStream.str())) {
        success = false;

        break;
      }
    }

    // TODO: Should instead be 'outputType'.
    if (!cli::options.doLlvmIr && success) {
      driver.link(outputFilePaths);
    }

    if (!success) {
      // TODO: Error or verbose?
      log::verbose("Generation failed");

      return EXIT_FAILURE;
    }
  }
  else {
    log::error("No input files; Use --help to view commands");
  }

  return EXIT_SUCCESS;
}
