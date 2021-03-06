#ifndef SOF_SEARCH_PRIVATE_JOB_RUNNER_INCLUDED
#define SOF_SEARCH_PRIVATE_JOB_RUNNER_INCLUDED

#include <atomic>
#include <mutex>
#include <thread>

#include "bot_api/server.h"
#include "search/private/job.h"
#include "search/private/limits.h"
#include "search/private/transposition_table.h"
#include "search/private/types.h"

namespace SoFSearch::Private {

// The class that runs multiple search jobs simultaneously and controls them.
class JobRunner {
public:
  inline explicit JobRunner(SoFBotApi::Server &server) : server_(server) {}

  ~JobRunner();

  // Stops the search. This operation is asynchronous, so the jobs may work for some time after you
  // call this function.
  void stop();

  // Stops the search and waits until it is really stopped. If the search is not started, this
  // function does nothing.
  void join();

  // Starts the search. If the search is already started, the previous search is stopped in a
  // blocked manner (i.e. by calling `join()`)
  void start(const Position &position, const SearchLimits &limits, size_t numJobs);

  // Indicates that the hash table size (in bytes) must be changed to `size`. The resize operation
  // may be deferred until the search is stopped.
  void hashResize(size_t size);

  // Indicates that the hash table must be cleared. The clear operation may be deferred until the
  // search is stopped.
  void hashClear();

  // Enables or disables debug mode. In debug mode the jobs may send extra information to server.
  inline void setDebugMode(const bool enable) {
    debugMode_.store(enable, std::memory_order_relaxed);
  }

  // Returns `true` if debug mode is enabled
  inline bool isDebugMode() const { return debugMode_.load(std::memory_order_relaxed); }

private:
  // Main function of the thread which controls all the running jobs.
  void runMainThread(const Position &position, const SearchLimits &limits, size_t numJobs);

  JobCommunicator comm_;
  TranspositionTable tt_;
  SoFBotApi::Server &server_;

  std::thread mainThread_;
  std::mutex hashChangeLock_;
  size_t hashSize_ = TranspositionTable::DEFAULT_SIZE;
  std::atomic<bool> debugMode_ = false;
  bool clearHash_ = false;
  bool canChangeHash_ = true;
};

}  // namespace SoFSearch::Private

#endif  // SOF_SEARCH_PRIVATE_JOB_RUNNER_INCLUDED
