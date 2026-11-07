#pragma once
#ifdef __PS5__
#include "AudioPlayer.h"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

namespace Ship {
/**
 * Audio output through sceAudioOut (the PS5 SDL2 build has no audio subsystem). The game's stereo
 * stream is resampled to the 48 kHz the system port accepts and handed over in fixed grains by a
 * worker thread, since sceAudioOutOutput blocks until the hardware takes the previous grain.
 */
class PS5AudioPlayer final : public AudioPlayer {
  public:
    PS5AudioPlayer(AudioSettings settings) : AudioPlayer(settings) {
    }
    ~PS5AudioPlayer();

    int32_t Buffered() override;

  protected:
    bool DoInit() override;
    void DoClose() override;
    void DoPlay(const uint8_t* buf, size_t len) override;

  private:
    void OutputLoop();
    void QueueResampled(const int16_t* frames, size_t frameCount, int32_t channels);

    int32_t mPort = -1;
    std::thread mWorker;
    std::atomic<bool> mRunning = false;
    std::mutex mQueueMutex;
    std::condition_variable mQueueReady;
    std::deque<int16_t> mQueue;
    double mSourcePosition = 0.0;
    int16_t mPreviousFrame[2] = { 0, 0 };
};
} // namespace Ship
#endif
