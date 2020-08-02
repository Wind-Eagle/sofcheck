#ifndef SOF_BOT_API_TYPES_INCLUDED
#define SOF_BOT_API_TYPES_INCLUDED

#include <chrono>
#include <cstdint>
#include <limits>

#include "core/move.h"
#include "core/types.h"

namespace SoFBotApi {

using std::chrono::milliseconds;

constexpr size_t INFINITE_MOVES = static_cast<size_t>(-1);

// Time control for one side
struct TimeControlSide {
  milliseconds time;  // Time left on the clock (or `milliseconds::max()` if unset)
  milliseconds inc;   // Time added after each move

  inline constexpr TimeControlSide() : time(milliseconds::max()), inc() {}
};

// Time control for both sides
struct TimeControl {
  TimeControlSide white;
  TimeControlSide black;
  size_t movesToGo;  // Positive number that represents the number of moves until time control
                     // changes. If it doesn't change, the value is equal to `INFINITE_MOVES`

  inline constexpr TimeControl() : movesToGo(INFINITE_MOVES) {}

  inline constexpr TimeControlSide &operator[](SoFCore::Color color) {
    return (color == SoFCore::Color::White) ? white : black;
  }

  inline constexpr const TimeControlSide &operator[](SoFCore::Color color) const {
    return (color == SoFCore::Color::White) ? white : black;
  }
};

// The type denoting if the position cost is exact, or it is a lower/upper bound
enum class PositionCostBound : uint8_t { Exact = 0, Lowerbound = 1, Upperbound = 2 };

// Type of the position cost
enum class PositionCostType {
  Centipawns,  // The postion cost is stored in centipawns
  Checkmate    // The position cost is stored in moves until checkmate
};

// Minimum and maximum allowed values (in centipawns) for `PositionCost`
constexpr int32_t MIN_POSITION_COST = -2'000'000'000;
constexpr int32_t MAX_POSITION_COST = 2'000'000'000;

// Structure that represents the position cost
struct PositionCost {
public:
  inline constexpr PositionCost() : value_(0) {}

  // Returns the position cost type
  inline constexpr PositionCostType type() const {
    return (MIN_POSITION_COST <= value_ && value_ <= MAX_POSITION_COST)
               ? PositionCostType::Centipawns
               : PositionCostType::Checkmate;
  }

  // Creates a position cost in centipawns. The greater the parameter, the better the cost for
  // current moving side.
  //
  // The parameter `cp` must be in range from `MIN_POSITION_COST` to `MAX_POSITION_COST`, otherwise
  // the behaviour is undefined.
  inline static constexpr PositionCost centipawns(int32_t cp) { return PositionCost(cp); }

  // Create a position in moves (not plies!) until checkmate. Depending on value, `moves` denotes
  // the following:
  // - if `moves <= 0`, then current moving side receives checkmate in `-moves` moves
  // - if `moves > 0`, then current moving side can checkmate in `moves` moves
  inline static constexpr PositionCost checkMate(int16_t moves) {
    const int32_t value = (moves <= 0) ? (VALUE_MIN - moves) : (VALUE_MAX - (moves - 1));
    return PositionCost(value);
  }

  // Returns the position cost in centipawns. If `type()` is not equal to `Centipawns`, then the
  // behaviour is undefined
  inline constexpr int32_t centipawns() const { return value_; }

  // Returns the number of moves until checkmate in the format described above. If `type()` is not
  // equal to `Centipawns`, then the behaviour is undefined
  inline constexpr int16_t checkMate() const {
    return (value_ < 0) ? (VALUE_MIN - value_) : (VALUE_MAX - value_ + 1);
  }

  // Comparison operators
#define D_ADD_COMPARE(op)                                                    \
  inline constexpr friend bool operator op(PositionCost a, PositionCost b) { \
    return a.value_ op b.value_;                                             \
  }
  D_ADD_COMPARE(==);
  D_ADD_COMPARE(!=);
  D_ADD_COMPARE(<);
  D_ADD_COMPARE(<=);
  D_ADD_COMPARE(>);
  D_ADD_COMPARE(>=);
#undef D_ADD_COMPARE

private:
  constexpr static int32_t VALUE_MIN = std::numeric_limits<int32_t>::min();
  constexpr static int32_t VALUE_MAX = std::numeric_limits<int32_t>::max();

  inline explicit constexpr PositionCost(int32_t value) : value_(value) {}

  int32_t value_;
};

// The type that indicates number of permille
using permille_t = uint16_t;

// Intermediate search result
struct SearchResult {
  size_t depth;             // Search depth (in plies)
  const SoFCore::Move *pv;  // The best line found
  size_t pvLen;             // Length of the best line found (if not present, set to zero)
  PositionCost cost;        // Estimated position cost
  PositionCostBound bound;  // Is position cost exact?
};

}  // namespace SoFBotApi

#endif  // SOF_BOT_API_TYPES_INCLUDED
