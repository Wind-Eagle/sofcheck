#ifndef SOF_ENGINE_CLIENTS_UCI_INCLUDED
#define SOF_ENGINE_CLIENTS_UCI_INCLUDED

#include <chrono>
#include <istream>
#include <ostream>

#include "core/move.h"
#include "engine_base/client.h"
#include "engine_base/connector.h"
#include "engine_base/server.h"
#include "util/no_copy_move.h"

namespace SoFEngineClients {

using SoFEngineBase::ApiResult;
using SoFEngineBase::Client;
using SoFEngineBase::PollResult;

// A server connector for UCI chess engines
class UciServerConnector final : public SoFEngineBase::ServerConnector, public SoFUtil::NoCopyMove {
public:
  const char *name() const override { return "UCI Server"; }
  const char *author() const override { return "SoFCheck developers"; }

  ApiResult finishSearch(SoFCore::Move bestMove) override;
  ApiResult sendString(const char *str) override;
  ApiResult sendResult(const SoFEngineBase::SearchResult &result) override;
  ApiResult sendNodeCount(uint64_t nodes) override;
  ApiResult sendHashFull(SoFEngineBase::permille_t hashFull) override;
  ApiResult sendCurrMove(SoFCore::Move move, size_t moveNumber = 0) override;
  ApiResult reportError(const char *message) override;

  PollResult poll() override;

  // Construct `UciServerConnector` with default streams
  UciServerConnector();

  // Construct `UciServerConnector` with custom
  UciServerConnector(std::istream &in, std::ostream &out, std::ostream &err);

  ~UciServerConnector() override;

protected:
  ApiResult connect(Client *client) override;
  void disconnect() override;

private:
  // Makes sure that the client is connected
  void ensureClient();

  // Returns the amount of time the search is running. If the search was not started, the behaviour
  // is undefined
  inline auto getSearchTime() const { return std::chrono::steady_clock::now() - searchStartTime_; }

  bool searchStarted_;
  std::chrono::time_point<std::chrono::steady_clock> searchStartTime_;
  Client *client_;
  std::istream &in_;
  std::ostream &out_;
  std::ostream &err_;
};

}  // namespace SoFEngineClients

#endif  // SOF_ENGINE_CLIENTS_UCI_INCLUDED