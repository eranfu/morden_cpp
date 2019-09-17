//
// Created by 11193 on 2019/9/17.
//

#ifndef MORDEN_CPP_SERVER_HTTP_HPP
#define MORDEN_CPP_SERVER_HTTP_HPP

#include "server_base.hpp"

namespace Web {
    using HTTP = boost::asio::ip::tcp::socket;

    template<>
    class Server<HTTP> : public ServerBase<HTTP> {
    public:
        using ServerBase<HTTP>::ServerBase;

    protected:
        void accept() override {
            auto socket = std::make_shared<HTTP>(io_service);
            acceptor.async_accept(*socket, [this, socket](const boost::system::error_code &ec) {
                accept();
                if (!ec) {
                    parse_request_and_respond(socket);
                }
            });
        }
    };
}

#endif //MORDEN_CPP_SERVER_HTTP_HPP
