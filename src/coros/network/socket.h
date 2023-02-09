#ifndef COROS_SOCKET_H
#define COROS_SOCKET_H

#include "coros/event/monitor.h"
#include "coros/event/manager.h"
#include "coros/awaiter/flush_awaiter.h"
#include "coros/awaiter/read_awaiter.h"
#include "coros/awaiter/skip_awaiter.h"
#include "coros/awaiter/write_awaiter.h"
#include "coros/memory/buffer.h"
#include "stream.h"

#include <atomic>
#include <cstddef>
#include <sys/socket.h>

namespace coros::base {
    class ThreadPool;

    struct SocketDetails {
        int socket_fd;
        sockaddr_storage client_addr;
        socklen_t addr_size;
    };

    class Socket {
        private:
            SocketDetails details;
            SocketEventManager event_manager;
            SocketStream stream;
            ByteBuffer input_buffer;
            ByteBuffer output_buffer;
            std::atomic_bool is_closed;
        public:
            Socket(SocketDetails details, SocketEventMonitor& event_monitor, 
                   ThreadPool& thread_pool);
            SocketReadAwaiter read(std::byte* dest, long long size, bool read_fully);
            SocketSkipAwaiter skip(long long size, bool skip_fully);
            SocketWriteAwaiter write(std::byte* src, long long size);
            SocketFlushAwaiter flush();
            int get_fd();
            void close_socket();
    };
}

#endif
