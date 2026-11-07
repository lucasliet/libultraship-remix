#ifdef __PS5__
#include "ship/port/ps5/PS5Impl.h"

#include <cerrno>
#include <cstdio>
#include <sys/stat.h>

#include <spdlog/spdlog.h>

namespace Ship {
namespace {
constexpr int kDisplayWidth = 1920;
constexpr int kDisplayHeight = 1080;
constexpr const char* kDataDirectory = "/data/BattleShip-Remix";
constexpr const char* kFallbackDataDirectory = "/download0";
constexpr const char* kBundleDirectory = "/app0";

bool EnsureDirectory(const char* path) {
    return mkdir(path, 0777) == 0 || errno == EEXIST;
}

/**
 * Reports whether files can be created in a folder. A title installed as a folder (in /data/homebrew
 * or on an external drive) is writable through /app0; an .ffpkg image is not.
 */
bool CanWriteTo(const std::string& directory) {
    const std::string probe = directory + "/.write_probe";
    FILE* file = fopen(probe.c_str(), "w");
    if (file == nullptr) {
        return false;
    }
    fclose(file);
    remove(probe.c_str());
    return true;
}

/**
 * Picks the folder for the ROM, the archive, saves and logs: the title folder when it is writable,
 * so everything stays with the game, otherwise /data/BattleShip-Remix, then /download0.
 */
std::string ChooseDataDirectory() {
    if (CanWriteTo(kBundleDirectory)) {
        return kBundleDirectory;
    }
    return EnsureDirectory(kDataDirectory) ? kDataDirectory : kFallbackDataDirectory;
}
} // namespace

void PS5::GetDisplaySize(int* width, int* height) {
    *width = kDisplayWidth;
    *height = kDisplayHeight;
}

std::string PS5::GetDataDirectory() {
    static const std::string sDirectory = ChooseDataDirectory();
    return sDirectory;
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
