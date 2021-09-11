#pragma once

namespace ilc {
    struct OutputGeneratorResult {
        bool success = false;

        /**
         * Verifies that the output files exist.
         */
        [[nodiscard]] bool verify();
    };
}
