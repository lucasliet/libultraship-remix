#pragma once

#include <cstddef>
#include <cstdint>

namespace Fast {

/* Answers of the display list range lookup (RegisterDLRangeLookup, RegisterDLBoundsCheck). */
constexpr int kDLBoundsUnknown = 0;
constexpr int kDLBoundsInRange = 1;
constexpr int kDLBoundsWalkedPast = 2;

/* Slots of the longest command the walker steps over at once (G_TEXRECT and G_TEXRECTFLIP take three), so a walk
 * that leaves a range lands at most this many commands after its end. */
constexpr size_t kDLWalkOffCommandSlots = 3;

/**
 * Tells whether a walk stepped off the end of a registered range onto cmd. Only the commands right after the
 * range end count: the rest of the walked-past window is general heap, where legitimate lists can be allocated.
 * @param result Answer of the range lookup for cmd.
 * @param cmd Address of the command about to be walked.
 * @param rangeEnd End of the registered range cmd lies after (the start of the walked-past span).
 * @param commandSize Size of one command in bytes.
 * @return true when cmd is one of the first commands after the range.
 */
constexpr bool DLWalkSteppedOffRange(int result, uintptr_t cmd, uintptr_t rangeEnd, size_t commandSize) {
    return (result == kDLBoundsWalkedPast) && ((cmd - rangeEnd) < (kDLWalkOffCommandSlots * commandSize));
}

} // namespace Fast
