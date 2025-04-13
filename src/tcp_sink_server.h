#pragma once

#include <cassert>

#include <chrono>

#include <boost/intrusive/list.hpp>

#include <tcp_server.h>

#include "event_dispatch.h"
#include "tcp_sink_connection.h"

class tcp_sink_server final : public flyzero::tcp_server,
                              public flyzero::event_dispatch::loop_listener {
    using connection_list =
        boost::intrusive::list<tcp_sink_connection,
                               boost::intrusive::member_hook<tcp_sink_connection,
                                                             tcp_sink_connection::list_hook,
                                                             &tcp_sink_connection::list_hook_> >;

public:
    /**
     * @brief 构造函数
     *
     * @param dispatcher 事件分发器
     * @param addr 监听地址，格式为 "ip:port"
     */
    tcp_sink_server(flyzero::event_dispatch &dispatcher, const char *addr);

    /**
     * @brief 构造函数
     *
     * @param dispatcher 事件分发器
     * @param ip 监听 IP 地址
     * @param port 监听端口
     */
    tcp_sink_server(flyzero::event_dispatch &dispatcher, in_addr_t ip, uint16_t port);

    /**
     * @brief 禁止拷贝 & 移动
     */
    tcp_sink_server(const tcp_sink_server &) = delete;
    tcp_sink_server(tcp_sink_server &&)      = delete;
    void operator=(const tcp_sink_server &)  = delete;
    void operator=(tcp_sink_server &&)       = delete;

    /**
     * @brief 析构函数
     */
    ~tcp_sink_server() override = default;

    /**
     * @brief 关闭连接
     *
     * @param conn 要关闭的连接
     */
    void close(tcp_sink_connection &conn);

protected:
    /**
     * @brief 接收连接
     *
     * @param sock 套接字
     * @param addr 地址
     * @param addrlen 地址长度
     */
    void on_accept(flyzero::file_descriptor &&sock,
                   const sockaddr_storage    &addr,
                   socklen_t                  addrlen) override;

    /**
     * @brief 事件循环
     */
    void on_loop() override;

    /**
     * @brief 关闭非活动连接
     */
    void close_nonactive_connection();

    /**
     * @brief 清理关闭列表
     */
    void cleanup_closing_list();

private:
    flyzero::event_dispatch   &dispatcher_;      ///< 事件分发器
    connection_list            active_list_{};   ///< 活动连接列表
    connection_list            closing_list_{};  ///< 关闭连接列表
    const std::chrono::minutes timeout_{5};      ///< 超时时间
};

inline tcp_sink_server::tcp_sink_server(flyzero::event_dispatch &dispatcher,
                                        in_addr_t                ip,
                                        uint16_t                 port)
    : tcp_server{listen(ip, port)}, loop_listener{}, dispatcher_{dispatcher} {
    dispatcher_.register_loop_listener(*this);
    dispatcher_.register_io_listener(*this, flyzero::event_dispatch::event::read);
}

inline void tcp_sink_server::close(tcp_sink_connection &conn) {
    auto const it = active_list_.iterator_to(conn);
    closing_list_.splice(closing_list_.begin(), active_list_, it);
}
