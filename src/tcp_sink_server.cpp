#include "tcp_sink_server.h"

#include <event_dispatch.h>
#include <utility.h>
#include <stdexcept>

inline std::pair<in_addr_t, in_port_t> split_addr(const char *addr) {
    if (!addr) {
        throw std::invalid_argument{"Invalid address"};
    }

    in_addr_t ip;
    auto const [ip_str, port_str] = flyzero::utility::split(addr, ':');
    if (ip_str.empty()) {  // 支持 ":port" 的格式
        ip = INADDR_ANY;
    } else if (inet_pton(AF_INET, ip_str.data(), &ip) != 1) {
        throw std::invalid_argument{"Invalid IP address"};
    }

    if (port_str.empty()) {
        throw std::invalid_argument{"Invalid port number"};
    }

    char      *endptr = nullptr;
    auto const num    = strtol(port_str.data(), &endptr, 10);
    if (!endptr || *endptr != '\0' || num <= 0 || std::numeric_limits<in_port_t>::max() < num) {
        throw std::invalid_argument{"Invalid port number"};
    }

    auto const port = static_cast<in_port_t>(num);
    return {ip, port};
}

tcp_sink_server::tcp_sink_server(flyzero::event_dispatch &dispatcher, const char *addr)
    : tcp_sink_server{dispatcher, split_addr(addr).first, split_addr(addr).second} {}

void tcp_sink_server::on_accept(flyzero::file_descriptor &&sock,
                                const sockaddr_storage    &addr,
                                socklen_t                  addrlen) {
    (void)addr;
    (void)addrlen;
    auto const conn = new tcp_sink_connection(*this, std::move(sock));
    auto const now  = std::chrono::steady_clock::now();
    conn->set_deadline(now + timeout_);
    dispatcher_.register_io_listener(*conn, flyzero::event_dispatch::event::read);
    active_list_.push_back(*conn);
}

void tcp_sink_server::on_loop() {
    close_nonactive_connection();

    cleanup_closing_list();
}

inline void tcp_sink_server::close_nonactive_connection() {
    auto const now = std::chrono::steady_clock::now();
    while (!active_list_.empty()) {
        auto &front = active_list_.front();
        if (front.get_deadline() > now) {
            break;
        }

        close(front);
    }
}

inline void tcp_sink_server::cleanup_closing_list() {
    closing_list_.clear_and_dispose([](tcp_sink_connection *conn) { delete conn; });
}
