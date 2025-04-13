#pragma once

#include <chrono>

#include <boost/intrusive/list_hook.hpp>

#include <tcp_connection.h>

class tcp_sink_server;

class tcp_sink_connection final : public flyzero::tcp_connection {
    friend tcp_sink_server;

    using list_hook  = boost::intrusive::list_member_hook<>;
    using time_point = std::chrono::steady_clock::time_point;

public:
    /**
     * @brief 构造函数
     *
     * @param server 服务器实例
     * @param fd 套接字
     */
    tcp_sink_connection(tcp_sink_server &server, flyzero::file_descriptor &&fd);

    /**
     * @brief 禁止拷贝 & 移动
     */
    tcp_sink_connection(const tcp_sink_connection &) = delete;
    tcp_sink_connection(tcp_sink_connection &&)      = delete;
    void operator=(const tcp_sink_connection &)      = delete;
    void operator=(tcp_sink_connection &&)           = delete;

    /**
     * @brief 析构函数
     */
    ~tcp_sink_connection() override = default;

    /**
     * @brief 设置超时时间
     *
     * @param deadline 超时时间
     */
    void set_deadline(time_point deadline);

    /**
     * @brief 获取超时时间
     *
     * @return 超时时间
     */
    [[nodiscard]] time_point get_deadline() const;

protected:
    /**
     * @brief 读取数据
     *
     * @param data 数据
     * @param size 数据大小
     * @return 读取的字节数
     */
    size_t on_read(const void *data, size_t size) override;

    /**
     * @brief 写入数据
     *
     * @param data 数据
     * @param size 数据大小
     * @return 写入的字节数
     */
    size_t on_write(void *data, size_t size) override;

    /**
     * @brief 关闭连接
     */
    void on_close() override;

    tcp_sink_server &server_;       ///< 服务器实例
    list_hook        list_hook_{};  ///< 链表钩子
    time_point       deadline_{};   ///< 超时时间
};

inline tcp_sink_connection::tcp_sink_connection(tcp_sink_server           &server,
                                                flyzero::file_descriptor &&fd)
    : tcp_connection{std::move(fd), 4096, 0}, server_{server} {}

inline void tcp_sink_connection::set_deadline(time_point deadline) { deadline_ = deadline; }

inline auto tcp_sink_connection::get_deadline() const -> time_point { return deadline_; }
