#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <ionshared/diagnostics/diagnostic.h>
#include <ionshared/container/vector.h>

namespace ilc {
    typedef void (*Callback)();

    typedef ionshared::Vector<ionshared::Diagnostic> DiagnosticVector;
}
