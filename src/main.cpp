#include <iostream>
#include <stdexcept>

#include <event_dispatch.h>

#include "tcp_sink_server.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: tcp_sink_server [ip:]<port>" << std::endl;
        return 1;
    }

    try {
        flyzero::event_dispatch dispatch;
        tcp_sink_server         server{dispatch, argv[1]};
        dispatch.run_loop(std::chrono::milliseconds{100});
        return 0;
    } catch (const std::invalid_argument &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}