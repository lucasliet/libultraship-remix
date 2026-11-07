#ifdef __PS5__
#include "ship/audio/PS5AudioPlayer.h"

#include <algorithm>
#include <array>
#include <spdlog/spdlog.h>

extern "C" {
int32_t sceAudioOutInit(void);
int32_t sceAudioOutOpen(int32_t userId, int32_t type, int32_t index, uint32_t len, uint32_t freq, uint32_t param);
int32_t sceAudioOutOutput(int32_t handle, const void* samples);
int32_t sceAudioOutClose(int32_t handle);
}

namespace Ship {
namespace {
constexpr int32_t kSystemUser = 0xFF;
constexpr int32_t kMainPort = 0;
constexpr uint32_t kOutputRate = 48000;
constexpr uint32_t kGrainFrames = 256;
constexpr uint32_t kFormatS16Stereo = 1;
constexpr size_t kMaxQueuedFrames = kOutputRate / 4;
constexpr int32_t kOutputChannels = 2;
} // namespace

PS5AudioPlayer::~PS5AudioPlayer() {
    DoClose();
}

bool PS5AudioPlayer::DoInit() {
    int32_t init = sceAudioOutInit();
    if (init < 0) {
        SPDLOG_ERROR("sceAudioOutInit failed: {:#x}", static_cast<uint32_t>(init));
        return false;
    }
    mPort = sceAudioOutOpen(kSystemUser, kMainPort, 0, kGrainFrames, kOutputRate, kFormatS16Stereo);
    if (mPort < 0) {
        SPDLOG_ERROR("sceAudioOutOpen failed: {:#x}", static_cast<uint32_t>(mPort));
        return false;
    }
    mRunning = true;
    mWorker = std::thread(&PS5AudioPlayer::OutputLoop, this);
    SPDLOG_INFO("PS5 audio: {} Hz game stream resampled to {} Hz", GetSampleRate(), kOutputRate);
    return true;
}

void PS5AudioPlayer::DoClose() {
    if (!mRunning.exchange(false)) {
        return;
    }
    mQueueReady.notify_all();
    if (mWorker.joinable()) {
        mWorker.join();
    }
    sceAudioOutOutput(mPort, nullptr);
    sceAudioOutClose(mPort);
    mPort = -1;
}

int32_t PS5AudioPlayer::Buffered() {
    std::lock_guard<std::mutex> lock(mQueueMutex);
    size_t outputFrames = mQueue.size() / kOutputChannels;
    return static_cast<int32_t>(outputFrames * GetSampleRate() / kOutputRate);
}

void PS5AudioPlayer::DoPlay(const uint8_t* buf, size_t len) {
    int32_t channels = GetNumOutputChannels();
    size_t frameCount = len / (sizeof(int16_t) * channels);
    QueueResampled(reinterpret_cast<const int16_t*>(buf), frameCount, channels);
}

void PS5AudioPlayer::QueueResampled(const int16_t* frames, size_t frameCount, int32_t channels) {
    const double step = static_cast<double>(GetSampleRate()) / kOutputRate;
    std::lock_guard<std::mutex> lock(mQueueMutex);
    if (mQueue.size() / kOutputChannels > kMaxQueuedFrames) {
        return;
    }
    while (mSourcePosition < static_cast<double>(frameCount)) {
        size_t index = static_cast<size_t>(mSourcePosition);
        double fraction = mSourcePosition - static_cast<double>(index);
        for (int32_t channel = 0; channel < kOutputChannels; channel++) {
            int16_t previous = index == 0 ? mPreviousFrame[channel] : frames[(index - 1) * channels + channel];
            int16_t current = frames[index * channels + channel];
            mQueue.push_back(static_cast<int16_t>(previous + (current - previous) * fraction));
        }
        mSourcePosition += step;
    }
    mSourcePosition -= static_cast<double>(frameCount);
    if (frameCount > 0) {
        mPreviousFrame[0] = frames[(frameCount - 1) * channels];
        mPreviousFrame[1] = frames[(frameCount - 1) * channels + 1];
    }
    mQueueReady.notify_one();
}

void PS5AudioPlayer::OutputLoop() {
    std::array<int16_t, kGrainFrames * kOutputChannels> grain{};
    while (mRunning) {
        {
            std::unique_lock<std::mutex> lock(mQueueMutex);
            size_t available = std::min(mQueue.size(), grain.size());
            std::copy_n(mQueue.begin(), available, grain.begin());
            mQueue.erase(mQueue.begin(), mQueue.begin() + available);
            std::fill(grain.begin() + available, grain.end(), 0);
        }
        if (sceAudioOutOutput(mPort, grain.data()) < 0) {
            SPDLOG_ERROR("sceAudioOutOutput failed; PS5 audio stopped");
            return;
        }
    }
}
} // namespace Ship
#endif
