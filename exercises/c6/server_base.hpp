//
// Created by 11193 on 2019/9/11.
//

#ifndef MORDEN_CPP_SERVER_BASE_HPP
#define MORDEN_CPP_SERVER_BASE_HPP

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <regex>
#include <thread>
#include <boost/asio.hpp>

namespace Web {
    struct Request {
        // request method, POST, GET; path; HTTP version
        std::string method;
        // request method, POST, GET; path; HTTP version
        std::string path;
        // request method, POST, GET; path; HTTP version
        std::string http_version;
        // use smart pointer for reference counting of content
        std::shared_ptr<std::istream> content;
        // hash container, key-value dict
        std::unordered_map<std::string, std::string> header;
        // use regular expression for path match
        std::smatch path_match;
    };

    template<typename TK, typename TV>
    std::ostream &operator<<(std::ostream &ostream, const std::unordered_map<TK, TV> um) {
        ostream << "{";
        bool first = true;
        for (const auto &[k, v] : um) {
            if (!first) {
                ostream << ", ";
            }
            ostream << k << ": " << v;
            first = false;
        }
        ostream << "}";
        return ostream;
    }

    std::ostream &operator<<(std::ostream &ostream, const Request &request) {
        ostream << "{" << std::endl;
        ostream << "\tmethod: " << request.method << std::endl;
        ostream << "\tpath: " << request.path << std::endl;
        ostream << "\thttp_version: " << request.http_version << std::endl;
        ostream << "\tcontent: " << request.content << std::endl;
        ostream << "\theader: " << request.header << std::endl;
        ostream << "}" << std::endl;
        return ostream;
    }

    typedef std::map<std::string,
            std::unordered_map<std::string, std::function<void(std::ostream &, Request &)>>> resource_type;

    template<typename socket_type>
    class ServerBase {
    public:
        resource_type resource;
        resource_type default_resource;

        ServerBase(unsigned short port, std::size_t num_threads)
                : acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
                  num_threads(num_threads) {
        }

        void start() {
            for (auto it = resource.begin(); it != resource.end(); ++it) {
                all_resources.push_back(std::move(it));
            }

            for (auto it = default_resource.begin(); it != default_resource.end(); ++it) {
                all_resources.push_back(std::move(it));
            }

            accept();

            for (int i = 1; i < num_threads; ++i) {
                threads.push_back(std::thread([&]() {
                    io_service.run();
                }));
            }

            io_service.run();

            for (auto &t:threads) {
                t.join();
            }
        }

    protected:
        boost::asio::io_service io_service;
        boost::asio::ip::tcp::acceptor acceptor;

        virtual void accept() {
        }

        void parse_request_and_respond(std::shared_ptr<socket_type> socket) {
            auto buffer = std::make_shared<boost::asio::streambuf>();
            boost::asio::async_read_until(
                    *socket, *buffer, "\r\n\r\n",
                    [this, socket, buffer](boost::system::error_code ec, std::size_t bytes_transferred) {
                        if (!!ec) {
                            return;
                        }
                        std::size_t total = buffer->size();
                        std::istream stream(buffer.get());
                        auto request = std::make_shared<Request>();
                        *request = parse_request(stream);
                        if (request->header.count("Content-Length") > 0) {
                            auto content_length = std::stoull(request->header["Content-Length"]);
                            boost::asio::async_read(
                                    *socket, *buffer,
                                    boost::asio::transfer_exactly(content_length - (total - bytes_transferred)),
                                    [request, buffer, this, socket](const boost::system::error_code &ec,
                                                                    std::size_t bytes_transferred) {
                                        if (!!ec) {
                                            return;
                                        }
                                        request->content = std::make_shared<std::istream>(buffer.get());
                                        respond(socket, request);
                                    });
                        } else {
                            respond(socket, request);
                        }
                    });
        }

        Request parse_request(std::istream &stream) const {
            Request request{};
            std::regex regex("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
            std::smatch match;
            std::string line;
            std::getline(stream, line);
            line.pop_back();
            std::cout << line << std::endl;
            if (std::regex_match(line, match, regex)) {
                request.method = match[1];
                request.path = match[2];
                request.http_version = match[3];

                regex = "^([^:]*): ?(.*)$";
                bool matched;
                do {
                    std::getline(stream, line);
                    line.pop_back();
                    std::cout << line << std::endl;
                    matched = std::regex_match(line, match, regex);
                    if (matched) {
                        request.header[match[1]] = match[2];
                    }
                } while (matched);
            }

            std::cout << request;
            return request;
        }

        void respond(std::shared_ptr<socket_type> socket, const std::shared_ptr<Request> &request) {
            for (const auto &res_it: all_resources) {
                std::regex regex(res_it->first);
                std::smatch match;
                if (std::regex_match(request->path, match, regex)) {
                    if (res_it->second.count(request->method) > 0) {
                        request->path_match = std::move(match);
                        auto write_buf = std::make_shared<boost::asio::streambuf>();
                        std::ostream response(write_buf.get());
                        res_it->second[request->method](response, *request);
                        boost::asio::async_write(
                                *socket, *write_buf,
                                [this, request, socket]
                                        (const boost::system::error_code &ec, std::size_t bytes_transferred) {
                                    if (!ec && std::stof(request->http_version) > 1.05f) {
                                        parse_request_and_respond(socket);
                                    }
                                });
                    }
                }
            }
        }

    private:
        std::vector<resource_type::iterator> all_resources;
        size_t num_threads;
        std::vector<std::thread> threads;
    };

    template<typename socket_type>
    class Server : public ServerBase<socket_type> {
        using ServerBase<socket_type>::ServerBase;
    };
}

#endif //MORDEN_CPP_SERVER_BASE_HPP
