#pragma once

#include <string>
#include <map>
#include <optional>
#include <ilc/misc/helpers.h>

namespace ilc::jit {
    static ionshared::Map<std::string, Callback> actions =
        ionshared::Map<std::string, Callback>();

    static void registerCommonActions() {
        jit::actions.set("quit", []() {
            // TODO: std::exit is not a safe method to exit the program.
            std::exit(EXIT_SUCCESS);
        });

        jit::actions.set("clear", []() {
            #if defined(OS_WINDOWS)
                system("cls");
            #elif defined(OS_LINUX) || defined(OS_MAC)
                system("clear");
            #else
                log::error("That action is platform-specific, and this platform is unknown or unsupported");
            #endif
        });
    }
}
