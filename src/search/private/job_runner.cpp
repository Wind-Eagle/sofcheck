#include "search/private/job_runner.h"

#include <chrono>
#include <deque>
#include <string>
#include <utility>
#include <vector>

#include "core/board.h"
#include "core/move.h"
#include "core/movegen.h"
#include "util/defer.h"
#include "util/logging.h"
#include "util/random.h"

namespace SoFSearch::Private {

using namespace std::chrono_literals;
using namespace SoFUtil::Logging;

using SoFCore::Board;
using SoFCore::Move;
using std::chrono::steady_clock;

// Type of log entry
constexpr const char *JOB_RUNNER = "JobRunner";

class Stats {
public:
  inline uint64_t get(const JobStat stat) const { return stats_[static_cast<size_t>(stat)]; }

  inline uint64_t nodes() const { return get(JobStat::Nodes); }
  inline uint64_t ttHits() const { return get(JobStat::TtHits); }

  inline void add(const JobResults &results) {
    for (size_t i = 0; i < JOB_STAT_SZ; ++i) {
      stats_[i] += results.get(static_cast<JobStat>(i));
    }
  }

private:
  uint64_t stats_[JOB_STAT_SZ] = {};
};

void JobRunner::hashClear() {
  std::unique_lock lock(hashChangeLock_);
  if (!canChangeHash_) {
    clearHash_ = true;
    return;
  }
  tt_.clear();
}

void JobRunner::hashResize(const size_t size) {
  std::unique_lock lock(hashChangeLock_);
  if (!canChangeHash_) {
    hashSize_ = size;
    return;
  }
  tt_.resize(size, false);
  hashSize_ = tt_.sizeBytes();
}

void JobRunner::join() {
  if (mainThread_.joinable()) {
    comm_.stop();
    mainThread_.join();
  }
}

static Move pickRandomMove(Board board) {
  Move moves[SoFCore::BUFSZ_MOVES];
  const size_t count = genAllMoves(board, moves);
  SoFUtil::randomShuffle(moves, moves + count);
  for (size_t i = 0; i < count; ++i) {
    const Move move = moves[i];
    const SoFCore::MovePersistence persistence = moveMake(board, move);
    if (!isMoveLegal(board)) {
      moveUnmake(board, move, persistence);
      continue;
    }
    moveUnmake(board, move, persistence);
    return move;
  }
  return Move::null();
}

void JobRunner::runMainThread(const Position &position, const SearchLimits &limits,
                              const size_t numJobs) {
  {
    std::unique_lock lock(hashChangeLock_);
    canChangeHash_ = false;
  }
  SOF_DEFER({
    // Perform delayed requests to modify transposition table
    std::unique_lock lock(hashChangeLock_);
    tt_.resize(hashSize_, clearHash_);
    hashSize_ = tt_.sizeBytes();
    clearHash_ = false;
    canChangeHash_ = true;
  });

  // Create jobs and associated threads. We store the jobs in `deque` instead of `vector`, as `Job`
  // instances are not moveable
  std::deque<Job> jobs;
  for (size_t i = 0; i < numJobs; ++i) {
    jobs.emplace_back(comm_, tt_, server_, i);
  }
  std::vector<std::thread> threads;
  for (size_t i = 0; i < numJobs; ++i) {
    threads.emplace_back([&job = jobs[i], &position, &limits]() { job.run(position, limits); });
  }

  static constexpr auto STATS_UPDATE_INTERVAL = 3s;
  static constexpr auto THREAD_TICK_INTERVAL = 30ms;

  // Run loop in which we check the jobs' status
  const auto startTime = steady_clock::now();
  auto statsLastUpdatedTime = startTime;
  do {
    const auto now = steady_clock::now();

    // Collect stats
    Stats stats;
    for (const Job &job : jobs) {
      stats.add(job.results());
    }

    // Check if it's time to stop
    if (stats.nodes() > limits.nodes ||
        (limits.time != TIME_UNLIMITED && now - startTime > limits.time)) {
      comm_.stop();
    }

    // Print stats
    if (now >= statsLastUpdatedTime + STATS_UPDATE_INTERVAL) {
      server_.sendNodeCount(stats.nodes());
      server_.sendHashHits(stats.ttHits());
      while (now >= statsLastUpdatedTime + STATS_UPDATE_INTERVAL) {
        statsLastUpdatedTime += STATS_UPDATE_INTERVAL;
      }
    }
  } while (!comm_.wait(THREAD_TICK_INTERVAL));

  // Join job threads
  for (std::thread &thread : threads) {
    thread.join();
  }

  // Display best move
  size_t bestDepth = 0;
  Move bestMove = Move::null();
  for (const Job &job : jobs) {
    const size_t depth = job.results().depth();
    if (depth > bestDepth) {
      bestDepth = depth;
      bestMove = job.results().bestMove();
    }
  }
  if (bestMove == Move::null()) {
    if (bestDepth != 0) {
      logError(JOB_RUNNER) << "At least one depth is calculated, but the best move is not found";
    }
    bestMove = pickRandomMove(position.last);
  }
  server_.finishSearch(bestMove);
}

void JobRunner::start(const Position &position, const SearchLimits &limits, const size_t numJobs) {
  join();
  comm_.reset();
  tt_.nextEpoch();
  mainThread_ = std::thread(
      [this, position, limits, numJobs]() { runMainThread(position, limits, numJobs); });
}

void JobRunner::stop() { comm_.stop(); }

JobRunner::~JobRunner() { join(); }

}  // namespace SoFSearch::Private
