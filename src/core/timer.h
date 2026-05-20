#pragma once

#include <chrono>

namespace ant::core {

class Timer final {
public:
    explicit Timer(double fixed_step_seconds = 1.0 / 60.0);

    void Tick();
    void Reset();

    double DeltaSeconds() const;
    double FixedStepSeconds() const;
    double AccumulatorSeconds() const;

    bool ShouldStep() const;
    void ConsumeStep();
    void ForceStep();

private:
    using clock = std::chrono::high_resolution_clock;

    clock::time_point last_tick_{};
    double delta_seconds_{0.0};
    double accumulator_seconds_{0.0};
    double fixed_step_seconds_{0.0};
    bool started_{false};
};

} // namespace ant::core
