#ifdef __PS5__
#include "ship/port/ps5/PS5Impl.h"

#include <cstdio>

#include <spdlog/spdlog.h>

namespace Ship {
namespace {
constexpr int kDisplayWidth = 1920;
constexpr int kDisplayHeight = 1080;
constexpr const char* kBundleDirectory = "/app0";

} // namespace

void PS5::GetDisplaySize(int* width, int* height) {
    *width = kDisplayWidth;
    *height = kDisplayHeight;
}

std::string PS5::GetDataDirectory() {
    // The title ships as a folder (in /data/homebrew or on an external drive), so everything the game
    // writes stays next to it.
    return kBundleDirectory;
}

std::string PS5::GetBundleDirectory() {
    return kBundleDirectory;
}

void PS5::ThrowMissingOTR(const std::string& archivePath) {
    SPDLOG_ERROR("Main archive not found at {}", archivePath);
    std::string notePath = GetDataDirectory() + "/MISSING_ARCHIVE.txt";
    FILE* note = fopen(notePath.c_str(), "w");
    if (note == nullptr) {
        return;
    }
    fprintf(note, "Main archive not found: %s\nCopy the ROM to %s and restart the title.\n", archivePath.c_str(),
            GetDataDirectory().c_str());
    fclose(note);
}
} // namespace Ship
#endif
