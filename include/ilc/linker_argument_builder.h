#pragma once

#include <sstream>
#include <filesystem>
#include "linker_util.h"
#include "linker_kind.h"

namespace ilc {
    class LinkerArgumentBuilder {
    private:
        std::stringstream argumentStream{};

    public:
        const LinkerKind targetLinkerKind;

        explicit LinkerArgumentBuilder(LinkerKind target) noexcept :
            targetLinkerKind(target) {
            //
        }

        /**
         * Append text without a space at the beginning.
         */
        LinkerArgumentBuilder& append(std::string text) {
            this->argumentStream << text;

            return *this;
        }

        LinkerArgumentBuilder& appendSpace() {
            return this->append(" ");
        }

        /**
         * Append text with a space at the beginning.
         */
        LinkerArgumentBuilder& add(std::string text) {
            return this->appendSpace()
                .append(text);
        }

        LinkerArgumentBuilder& addPositional(std::string text) {
            return this->add(text);
        }

        LinkerArgumentBuilder& addPath(std::filesystem::path path) {
            // TODO: Ensure paths are either absolute or converted to absolute paths to avoid linker environment problems.
            return this->add(path.string());
        }

        LinkerArgumentBuilder& addFlag(std::string name, bool isShort) {
            return this->add(
                linker_util::getLinkerKindPrefix(
                    this->targetLinkerKind,
                    isShort
                )
            )
                .append(name);
        }

        LinkerArgumentBuilder& addOption(
            std::string name,
            std::string value,
            bool isShort
        ) {
            return this->addFlag(name, isShort)
                .add(value);
        }

        [[nodiscard]] std::string finish() const {
            return this->argumentStream.str();
        }
    };
}
