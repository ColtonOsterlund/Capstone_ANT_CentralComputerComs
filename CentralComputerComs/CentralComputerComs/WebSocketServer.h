#pragma once

#include <set>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "WebSocketQueueHandler.h"

using websocketpp::connection_hdl;

typedef websocketpp::server<websocketpp::config::asio> server;
typedef std::set<connection_hdl, std::owner_less<connection_hdl>> con_list;

#define DEFAULT_WEBSOCKET_PORT 8080

/* Forward declare to avoid association issue */
class WebSocketQueueHandler;

class WebSocketServer: public CentralComputerThread
{
public:
    WebSocketServer();
    // Allow the class to be callable as a thread
    void operator()() override;

    void on_open(connection_hdl hdl);

    void on_close(connection_hdl hdl);

    void on_message(connection_hdl hdl, server::message_ptr msg);

    void run();

    void set_queue_handler(WebSocketQueueHandler* queue_handler);

    void send(std::string message);

    void request_termination() override;


private:
    server m_server;
    con_list m_connections;

    WebSocketQueueHandler* queue_handler;
};

