#pragma once

#include <string>
#include <ionlang/lexical/token.h>

namespace ilc {
    struct CodeHighlight {
        static std::string coat(ionlang::Token token);
    };
}
