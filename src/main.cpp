#include <sys/syslog.h>
#include <syslog.h>

#include <iostream>
#include <stdexcept>

#include <event_dispatch.h>

#include "tcp_sink_connection.h"
#include "tcp_sink_server.h"

static tcp_sink_connection *on_new_tcp_sink_connection(tcp_sink_server           &server,
                                                       flyzero::file_descriptor &&sock,
                                                       const sockaddr_storage    &addr,
                                                       socklen_t                  addrlen) {
    auto &in_addr = reinterpret_cast<const sockaddr_in &>(addr);
    syslog(LOG_INFO, "New connection: %s:%d", inet_ntoa(in_addr.sin_addr), ntohs(in_addr.sin_port));
    return new tcp_sink_connection{
        server, std::move(sock), in_addr.sin_addr.s_addr, in_addr.sin_port};
}

static void on_del_tcp_sink_connection(tcp_sink_connection *conn) {
    syslog(LOG_INFO, "Close connection: %s:%d", inet_ntoa(conn->get_ip()), ntohs(conn->get_port()));
    delete conn;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [ip:]<port>" << std::endl;
        return 1;
    }

    // Open syslog for logging
    openlog("tcp-sink", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "tcp-sink started with argument: %s", argv[1]);

    try {
        flyzero::event_dispatch dispatch;
        tcp_sink_server         server{
            dispatch, argv[1], on_new_tcp_sink_connection, on_del_tcp_sink_connection};
        syslog(LOG_INFO, "tcp-sink listening on %s", argv[1]);
        dispatch.run_loop(std::chrono::milliseconds{100});
    } catch (const std::invalid_argument &e) {
        syslog(LOG_ERR, "%s", e.what());
        return 1;
    }

    // Close syslog
    closelog();
}