#include "tcp_sink_connection.h"
#include "tcp_sink_server.h"

size_t tcp_sink_connection::on_read(const void *data, size_t size) {
    (void)data;
    // tcp_sink 丢弃所有数据
    return size;
}

size_t tcp_sink_connection::on_write(void *data, size_t size) {
    (void)data;
    // tcp_sink 从不写入数据
    abort();
    return size;
}

void tcp_sink_connection::on_close() { server_.close(*this); }