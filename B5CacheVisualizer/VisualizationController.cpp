#include "VisualizationController.h"

#include <stdexcept>
#include <utility>

namespace b5cacheui {

void VisualizationController::Reset() noexcept {
    frames_.clear();
    currentIndex_ = kNoFrame;
    state_ = PlaybackState::Stopped;
}

void VisualizationController::Append(VisualizationFrame frame) {
    if (!frames_.empty() && !IsAtLatest()) {
        throw std::logic_error("Cannot append a visualization frame while reviewing history.");
    }

    frame.frameNumber = frames_.size() + 1;
    frames_.push_back(std::move(frame));
    currentIndex_ = frames_.size() - 1;
    if (state_ != PlaybackState::Playing) {
        state_ = PlaybackState::Stopped;
    }
}

bool VisualizationController::MovePrevious() noexcept {
    if (!CanMovePrevious()) {
        return false;
    }

    --currentIndex_;
    state_ = PlaybackState::Reviewing;
    return true;
}

bool VisualizationController::MoveNext() noexcept {
    if (!HasRecordedNext()) {
        return false;
    }

    ++currentIndex_;
    if (state_ != PlaybackState::Playing) {
        state_ = IsAtLatest() ? PlaybackState::Stopped : PlaybackState::Reviewing;
    }
    return true;
}

bool VisualizationController::MoveToLatest() noexcept {
    if (frames_.empty()) {
        return false;
    }

    currentIndex_ = frames_.size() - 1;
    if (state_ != PlaybackState::Playing) {
        state_ = PlaybackState::Stopped;
    }
    return true;
}

bool VisualizationController::CanMovePrevious() const noexcept {
    return currentIndex_ != kNoFrame && currentIndex_ > 0;
}

bool VisualizationController::HasRecordedNext() const noexcept {
    return currentIndex_ != kNoFrame && currentIndex_ + 1 < frames_.size();
}

bool VisualizationController::IsAtLatest() const noexcept {
    return currentIndex_ != kNoFrame && currentIndex_ + 1 == frames_.size();
}

const VisualizationFrame* VisualizationController::Current() const noexcept {
    if (currentIndex_ == kNoFrame || currentIndex_ >= frames_.size()) {
        return nullptr;
    }
    return &frames_[currentIndex_];
}

const std::vector<VisualizationFrame>& VisualizationController::Frames() const noexcept {
    return frames_;
}

std::size_t VisualizationController::CurrentPosition() const noexcept {
    return currentIndex_ == kNoFrame ? 0 : currentIndex_ + 1;
}

std::size_t VisualizationController::FrameCount() const noexcept {
    return frames_.size();
}

void VisualizationController::Start() noexcept {
    state_ = PlaybackState::Playing;
}

void VisualizationController::Pause() noexcept {
    if (state_ == PlaybackState::Playing) {
        state_ = PlaybackState::Paused;
    }
}

void VisualizationController::Stop() noexcept {
    state_ = PlaybackState::Stopped;
}

PlaybackState VisualizationController::State() const noexcept {
    return state_;
}

void VisualizationController::SetSpeed(const PlaybackSpeed speed) noexcept {
    speed_ = speed;
}

PlaybackSpeed VisualizationController::Speed() const noexcept {
    return speed_;
}

unsigned int VisualizationController::TimerIntervalMs() const noexcept {
    switch (speed_) {
    case PlaybackSpeed::Slow:
        return 1200;
    case PlaybackSpeed::Fast:
        return 250;
    case PlaybackSpeed::Normal:
    default:
        return 650;
    }
}

}  // namespace b5cacheui
