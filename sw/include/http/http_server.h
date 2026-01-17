#pragma once
#include <functional>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <utility>

struct SocketHandler {
public:
    explicit SocketHandler(int fd) : fd_(fd) {
    }
    ~SocketHandler() {
        if (fd_ >= 0)
            close(fd_);
    }
    int get() const {
        return fd_;
    }

private:
    int fd_;
};

class HttpServer {
public:
    using Handler = std::function<std::pair<std::string, std::string>()>;

    HttpServer(int port = 8080);
    ~HttpServer();

    void add_route(const std::string& path, Handler handler);
    void start();

private:
    int port_number;
    bool is_running;
    std::unordered_map<std::string, Handler> current_routes;

    void handle_client(int client_socket);
};
