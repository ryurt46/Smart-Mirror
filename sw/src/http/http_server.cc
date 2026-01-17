#include "http/http_server.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

HttpServer::HttpServer(int port) : port_number(port), is_running(false) {
}
HttpServer::~HttpServer() {
}

void HttpServer::add_route(const std::string& path, Handler handler) {
    std::string clean = path;
    if (!clean.empty() && clean.back() == '/')
        clean.pop_back();
    current_routes[clean] = handler;
}

void HttpServer::start() {
    SocketHandler server_fd(socket(AF_INET, SOCK_STREAM, 0));
    if (server_fd.get() < 0) {
        perror("socket failed");
        return;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_number);

    int opt = 1;
    if (setsockopt(server_fd.get(), SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) <
        0) {
        perror("setsockopt failed");
        return;
    }

    if (bind(server_fd.get(), (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return;
    }

    if (listen(server_fd.get(), 10) < 0) {
        perror("listen failed");
        return;
    }

    std::cout << "Server listening on port " << port_number << "\n";

    is_running = true;
    while (is_running) {
        socklen_t len = sizeof(address);
        int client = accept(server_fd.get(), (struct sockaddr*)&address, &len);
        if (client < 0) {
            perror("accept failed");
            continue;
        }
        std::thread(&HttpServer::handle_client, this, client).detach();
    }
}

void HttpServer::handle_client(int client_socket) {
    SocketHandler client(client_socket);

    std::string request;
    char tmp[1024];
    ssize_t n;
    while ((n = read(client.get(), tmp, sizeof(tmp))) > 0) {
        request.append(tmp, n);
        if (request.size() >= 4 && request.substr(request.size() - 4) == "\r\n\r\n") {
            break;
        }
    }

    std::istringstream req(request);
    std::string method, raw_path;
    req >> method >> raw_path;

    std::string path = raw_path.substr(0, raw_path.find('?'));
    if (!path.empty() && path.back() == '/')
        path.pop_back();

    std::string body, type, status;

    if (current_routes.contains(path)) {
        try {
            auto [b, t] = current_routes[path]();
            body = b;
            type = t;
            status = "HTTP/1.1 200 OK\r\n";
        } catch (const std::exception&) {
            body = R"({"error":"Internal Server Error"})";
            type = "application/json";
            status = "HTTP/1.1 500 Internal Server Error\r\n";
        }
    } else {
        body = R"({"error":"Not Found"})";
        type = "application/json";
        status = "HTTP/1.1 404 Not Found\r\n";
    }

    std::ostringstream res;
    res << status << "Content-Type: " << type << "\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << body;

    auto s = res.str();
    send(client.get(), s.c_str(), s.size(), 0);
}
