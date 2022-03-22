#include "WebSocketServer.h"
#include <iostream>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

WebSocketServer::WebSocketServer(): CentralComputerThread(), queue_handler(NULL)
{
    m_server.init_asio();

    m_server.set_open_handler(bind(&WebSocketServer::on_open, this, ::_1));
    m_server.set_close_handler(bind(&WebSocketServer::on_close, this, ::_1));
    m_server.set_message_handler(bind(&WebSocketServer::on_message, this, ::_1, ::_2));
}

void WebSocketServer::operator()()
{
    run();
}

void WebSocketServer::on_open(connection_hdl hdl)
{
    m_connections.insert(hdl);
}

void WebSocketServer::on_close(connection_hdl hdl)
{
    m_connections.erase(hdl);
}

void WebSocketServer::on_message(connection_hdl hdl, server::message_ptr msg)
{
    if (queue_handler != NULL) {
        //std::cout << msg->get_payload() << std::endl;
        queue_handler->push_message(msg->get_payload());
    }
    else {
        std::cout << "Websocket server: Trying to push a message without setting the queue handler" << std::endl;
    }
}

void WebSocketServer::run()
{
    std::cout << "Starting WebSocket server on port: " << DEFAULT_WEBSOCKET_PORT << std::endl;
    m_server.listen(DEFAULT_WEBSOCKET_PORT);
    m_server.start_accept();
    m_server.run();
}

void WebSocketServer::set_queue_handler(WebSocketQueueHandler* queue_handler)
{
    this->queue_handler = queue_handler;
}

void WebSocketServer::send(std::string message)
{
    websocketpp::lib::error_code ec;
    for (auto it : m_connections) {
        m_server.send(it, message, websocketpp::frame::opcode::text, ec);

        if (ec) {
            std::cout << "> Error sending message: " << ec.message() << std::endl;
        }
    }

}

void WebSocketServer::request_termination()
{
    CentralComputerThread::request_termination();
    m_server.stop_listening();
    for (auto it : m_connections) {
        m_server.close(it, websocketpp::close::status::normal, "Server close");
    }
}
