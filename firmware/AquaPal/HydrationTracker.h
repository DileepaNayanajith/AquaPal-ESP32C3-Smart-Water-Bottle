#pragma once

#include <math.h>
#include <stdint.h>

// Small hardware-independent tracker so the event logic can be tested on a computer.
// It estimates drinking only from a stable reduction in measured bottle level.
// Without a tilt sensor it cannot reliably distinguish drinking from movement.
class HydrationTracker {
 public:
  enum Event { NONE, INITIALIZED, DRINK_ESTIMATED, REFILL_ESTIMATED };

  explicit HydrationTracker(uint16_t capacityMl = 2000)
      : capacityMl_(capacityMl) {}

  Event observe(float measuredMl) {
    if (!isfinite(measuredMl) || measuredMl < 0) return NONE;
    if (measuredMl > capacityMl_) measuredMl = capacityMl_;
    const uint16_t sample = static_cast<uint16_t>(lroundf(measuredMl));

    // Require three consecutive close measurements before changing state.
    if (candidateCount_ == 0 || difference(sample, candidateMl_) > 20) {
      candidateMl_ = sample;
      candidateCount_ = 1;
    } else if (candidateCount_ < 3) {
      ++candidateCount_;
    }
    if (candidateCount_ < 3) return NONE;

    if (!initialized_) {
      currentMl_ = candidateMl_;
      initialized_ = true;
      return INITIALIZED;
    }

    const int32_t delta = static_cast<int32_t>(currentMl_) - candidateMl_;
    if (delta >= 35) {
      consumedMl_ += static_cast<uint32_t>(delta);
      currentMl_ = candidateMl_;
      return DRINK_ESTIMATED;
    }
    if (delta <= -60) {
      currentMl_ = candidateMl_;
      return REFILL_ESTIMATED;
    }
    return NONE;
  }

  bool initialized() const { return initialized_; }
  uint16_t levelMl() const { return currentMl_; }
  uint32_t consumedMl() const { return consumedMl_; }
  void restoreConsumed(uint32_t ml) { consumedMl_ = ml; }
  void resetDaily() { consumedMl_ = 0; }
  void setGoal(uint16_t ml) { goalMl_ = ml; }
  uint16_t goalMl() const { return goalMl_; }
  uint16_t capacityMl() const { return capacityMl_; }

 private:
  static uint16_t difference(uint16_t a, uint16_t b) {
    return a > b ? a - b : b - a;
  }
  const uint16_t capacityMl_;
  uint16_t goalMl_ = 2000;
  uint16_t currentMl_ = 0;
  uint16_t candidateMl_ = 0;
  uint8_t candidateCount_ = 0;
  uint32_t consumedMl_ = 0;
  bool initialized_ = false;
};
