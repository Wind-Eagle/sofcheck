#include "search/private/limits.h"

namespace SoFSearch::Private {

using namespace std::chrono_literals;

using std::chrono::milliseconds;

SearchLimits SearchLimits::withTimeControl(const SoFCore::Board &board,
                                           const SoFBotApi::TimeControl &timeControl) {
  milliseconds totalTime = timeControl[board.side].time;
  const milliseconds inc = timeControl[board.side].inc;
  if (totalTime == milliseconds::max()) {
    // This happens when the server didn't set time properly. I don't know whether such cases are
    // valid, but it's better to come up with some definite value other than infinity.
    totalTime = 1h;
  }
  milliseconds time = totalTime;
  if (board.moveNumber < 10) {
    time /= 40;
  } else {
    time /= 20;
  }
  if (board.moveNumber > 30) {
    time += totalTime / 40;
  }
  time += inc;
  time = std::min(time, totalTime - 35ms);
  time = std::max(time, 1ms);
  return SearchLimits{DEPTH_UNLIMITED, NODES_UNLIMITED, time, timeControl};
}

}  // namespace SoFSearch::Private
