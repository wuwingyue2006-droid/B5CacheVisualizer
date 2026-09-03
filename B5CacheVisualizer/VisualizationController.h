#pragma once

#include "VisualizationFrame.h"

#include <cstddef>
#include <vector>

namespace b5cacheui {

enum class PlaybackState {
    Stopped,
    Playing,
    Paused,
    Reviewing
};

enum class PlaybackSpeed {
    Slow,
    Normal,
    Fast
};

class VisualizationController {
public:
    void Reset() noexcept;
    void Append(VisualizationFrame frame);

    bool MovePrevious() noexcept;
    bool MoveNext() noexcept;
    bool MoveToLatest() noexcept;

    bool CanMovePrevious() const noexcept;
    bool HasRecordedNext() const noexcept;
    bool IsAtLatest() const noexcept;

    const VisualizationFrame* Current() const noexcept;
    const std::vector<VisualizationFrame>& Frames() const noexcept;
    std::size_t CurrentPosition() const noexcept;
    std::size_t FrameCount() const noexcept;

    void Start() noexcept;
    void Pause() noexcept;
    void Stop() noexcept;
    PlaybackState State() const noexcept;

    void SetSpeed(PlaybackSpeed speed) noexcept;
    PlaybackSpeed Speed() const noexcept;
    unsigned int TimerIntervalMs() const noexcept;

private:
    static constexpr std::size_t kNoFrame = static_cast<std::size_t>(-1);

    std::vector<VisualizationFrame> frames_;
    std::size_t currentIndex_ = kNoFrame;
    PlaybackState state_ = PlaybackState::Stopped;
    PlaybackSpeed speed_ = PlaybackSpeed::Normal;
};

}  // namespace b5cacheui
