#include "core/timer.h"

namespace ant::core {

Timer::Timer(double fixed_step_seconds)
    : fixed_step_seconds_(fixed_step_seconds) {}

void Timer::Tick() {
    const auto now = clock::now();
    if (!started_) {
        last_tick_ = now;
        started_ = true;
        delta_seconds_ = 0.0;
        return;
    }

    const auto elapsed = std::chrono::duration<double>(now - last_tick_).count();
    last_tick_ = now;
    delta_seconds_ = elapsed;
    if (delta_seconds_ > 0.25) {
        delta_seconds_ = 0.25;
    }
    accumulator_seconds_ += delta_seconds_;
}

void Timer::Reset() {
    started_ = false;
    delta_seconds_ = 0.0;
    accumulator_seconds_ = 0.0;
}

double Timer::DeltaSeconds() const {
    return delta_seconds_;
}

double Timer::FixedStepSeconds() const {
    return fixed_step_seconds_;
}

double Timer::AccumulatorSeconds() const {
    return accumulator_seconds_;
}

bool Timer::ShouldStep() const {
    return accumulator_seconds_ >= fixed_step_seconds_;
}

void Timer::ConsumeStep() {
    accumulator_seconds_ -= fixed_step_seconds_;
}

void Timer::ForceStep() {
    accumulator_seconds_ += fixed_step_seconds_;
}

} // namespace ant::core
