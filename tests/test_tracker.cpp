#include <cassert>
#include <iostream>
#include "../firmware/AquaPal/HydrationTracker.h"

int main() {
  HydrationTracker t(2000);
  assert(t.observe(1600) == HydrationTracker::NONE);
  assert(t.observe(1602) == HydrationTracker::NONE);
  assert(t.observe(1598) == HydrationTracker::INITIALIZED);
  assert(t.levelMl() >= 1598 && t.levelMl() <= 1602);

  // A little noise should not be mistaken for drinking.
  t.observe(1590); t.observe(1591); t.observe(1589);
  assert(t.consumedMl() == 0);

  t.observe(1400); t.observe(1401);
  assert(t.observe(1399) == HydrationTracker::DRINK_ESTIMATED);
  assert(t.consumedMl() >= 190 && t.consumedMl() <= 210);

  uint32_t drank = t.consumedMl();
  t.observe(1950); t.observe(1951);
  assert(t.observe(1949) == HydrationTracker::REFILL_ESTIMATED);
  assert(t.consumedMl() == drank);

  t.resetDaily();
  assert(t.consumedMl() == 0);
  t.setGoal(1500);
  assert(t.goalMl() == 1500);
  std::cout << "HydrationTracker tests passed\n";
}
