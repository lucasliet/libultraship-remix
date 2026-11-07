#pragma once

#ifdef __PS5__
#include <string>

namespace Ship {
/**
 * PS5 native title glue: fixed 1080p output, the writable data folder and the read-only title image.
 */
class PS5 {
  public:
    /**
     * Writes the output size the ps5-opengl surface uses (1920x1080).
     * @param width receives the width in pixels
     * @param height receives the height in pixels
     */
    static void GetDisplaySize(int* width, int* height);

    /**
     * Returns the folder for config, saves, logs and the extracted archive: the title folder
     * itself (/app0), which is writable because the title ships as a folder.
     * @return the folder path, without a trailing slash
     */
    static std::string GetDataDirectory();

    /**
     * Returns the read-only title image (/app0) holding the packaged assets.
     * @return the folder path, without a trailing slash
     */
    static std::string GetBundleDirectory();

    /**
     * Logs that the main archive is missing and writes the same message next to the saves,
     * since a native title has no message box.
     * @param archivePath the archive path that was not found
     */
    static void ThrowMissingOTR(const std::string& archivePath);
};
} // namespace Ship
#endif
