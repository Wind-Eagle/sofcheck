#ifndef SOF_BOT_API_CONNECTION_INCLUDED
#define SOF_BOT_API_CONNECTION_INCLUDED

#include <memory>

#include "bot_api/api_base.h"
#include "bot_api/client.h"
#include "bot_api/connector.h"
#include "bot_api/server.h"
#include "util/result.h"

namespace SoFBotApi {

// Class that holds the connection between the client and the server
class Connection {
public:
  // Performs a single `poll()` on the client or on the server (depending on the connection type)
  PollResult poll();

  // Polls the client and the server until it shutdown or error. Returns `Ok` if it gets shutdown,
  // otherwise returns the error returned from `poll()`
  PollResult runPollLoop();

  // Creates a client-side connection (i.e. the type of connections used by chess engines). Returns
  // error if one of the sides was unable to connect
  static SoFUtil::Result<Connection, ApiResult> clientSide(std::unique_ptr<Client> client,
                                                           std::unique_ptr<ServerConnector> server);

  // Creates a server-side connection (i.e. the type of connections used by GUIs). Returns error if
  // one of the sides was unable to connect
  static SoFUtil::Result<Connection, ApiResult> serverSide(std::unique_ptr<ClientConnector> client,
                                                           std::unique_ptr<Server> server);

  template <typename Client, typename Server>
  inline static SoFUtil::Result<Connection, ApiResult> clientSide() {
    return clientSide(std::make_unique<Client>(), std::make_unique<Server>());
  }

  template <typename Client, typename Server>
  inline static SoFUtil::Result<Connection, ApiResult> serverSide() {
    return serverSide(std::make_unique<Client>(), std::make_unique<Server>());
  }

  Connection(Connection &&) = default;
  Connection &operator=(Connection &&) = default;

  ~Connection();

private:
  Connection(std::unique_ptr<Client> client, std::unique_ptr<Server> server, Connector *connector)
      : client_(std::move(client)), server_(std::move(server)), connector_(connector) {}

  std::unique_ptr<Client> client_;
  std::unique_ptr<Server> server_;
  Connector *connector_;  // Equal either to client or to server, won't be freed automatically
};

}  // namespace SoFBotApi

#endif  // SOF_BOT_API_CONNECTION_INCLUDED
