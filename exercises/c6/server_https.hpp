#ifndef MORDEN_CPP_SERVER_HTTPS_HPP
#define MORDEN_CPP_SERVER_HTTPS_HPP

#include <boost/asio/ssl.hpp>
#include "server_base.hpp"

namespace Web {
    using HTTPS = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

    template<>
    class Server<HTTPS> : public ServerBase<HTTPS> {

    };
}

#endif //MORDEN_CPP_SERVER_HTTPS_HPP
