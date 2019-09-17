//
// Created by 11193 on 2019/9/11.
//

#include <iostream>
#include "server_http.hpp"
#include "handler.hpp"

using namespace Web;

int main() {
    Server<HTTP> server(12345, 12);
    std::cout << "Server starting at port: 12345" << std::endl;
    start_server(server);
    return 0;
}
