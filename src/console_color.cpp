#include <ilc/cli/options.h>
#include <ilc/cli/console_color.h>

namespace ilc {
    const std::string ConsoleColor::reset = ConsoleColor::make((uint32_t)ConsoleSpecial::Reset);

    const std::string ConsoleColor::bold = ConsoleColor::make((uint32_t)ConsoleSpecial::Bold);

    const std::string ConsoleColor::underline = ConsoleColor::make((uint32_t)ConsoleSpecial::Underline);

    const std::string ConsoleColor::invert = ConsoleColor::make((uint32_t)ConsoleSpecial::Invert);

    std::string ConsoleColor::make(uint32_t code, std::optional<uint32_t> colorCode) {
        std::string base = "\033[" + std::to_string(code);

        if (colorCode.has_value()) {
            base += ";" + std::to_string(*colorCode);
        }

        return base + "m";
    }

    std::string ConsoleColor::apply(std::string text, ColorKind code) {
        // TODO: Repeated below.
        if (cli::options.noColor) {
            return text;
        }

        return ConsoleColor::make((uint32_t)code) + text;
    }

    std::string ConsoleColor::coat(std::string text, ColorKind code) {
        // TODO: Repeated above.
        if (cli::options.noColor) {
            return text;
        }

        return ConsoleColor::apply(text, code) + ConsoleColor::reset;
    }

    std::string ConsoleColor::red(std::string text) {
        return ConsoleColor::coat(text, ColorKind::ForegroundRed);
    }

    std::string ConsoleColor::green(std::string text) {
        return ConsoleColor::coat(text, ColorKind::ForegroundGreen);
    }

    std::string ConsoleColor::blue(std::string text) {
        return ConsoleColor::coat(text, ColorKind::ForegroundBlue);
    }

    std::string ConsoleColor::cyan(std::string text) {
        return ConsoleColor::coat(text, ColorKind::ForegroundCyan);
    }

    std::string ConsoleColor::white(std::string text) {
        return ConsoleColor::coat(text, ColorKind::ForegroundWhite);
    }

    std::string ConsoleColor::black(std::string text) {
        return ConsoleColor::coat(text, ColorKind::ForegroundBlack);
    }

    std::string ConsoleColor::yellow(std::string text) {
        return ConsoleColor::coat(text, ColorKind::ForegroundYellow);
    }

    std::string ConsoleColor::magenta(std::string text) {
        return ConsoleColor::coat(text, ColorKind::ForegroundMagenta);
    }
}
