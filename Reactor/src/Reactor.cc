#include <Reactor/Reactor.hpp>

#include <sys/epoll.h>
#include <mutex>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <array>

#define log(fmt, ...) \
    fprintf(stderr, "line%d: " fmt "\n", __LINE__, __VA_ARGS__)

namespace Reactor{

    //the data struct
    struct Reactor::ReactorData {
        int epollFd;
        struct epoll_event epollEventArray[1024];
        std::mutex epollMutex;
    };
    Reactor::Reactor():
        impl(new ReactorData)
    {
    }
    Reactor::~Reactor()
    {
        //nothing to do now
    }
    std::shared_ptr< Reactor > Reactor::data()
    {
        return shared_from_this();
    }
    int Reactor::Init()
    {
        int fd = epoll_create1(0);
        if (fd == -1) {
            log("%s", strerror(errno));
            return -1;
        }
        impl->epollFd = fd;
        return fd;
    }
    int Reactor::Add(int fd, struct epoll_event ev)
    {
        int ret;
        std::lock_guard< std::mutex > lockEpoll(impl->epollMutex);
        ret = epoll_ctl(impl->epollFd, EPOLL_CTL_ADD, fd, &ev);
        if (ret == -1) {
            log("%s", strerror(errno));
            return -1;
        }
        return ret;
    }
    int Reactor::Del(int fd)
    {
        int ret;
        std::lock_guard< std::mutex > lockEpoll(impl->epollMutex);
        ret = epoll_ctl(impl->epollFd, EPOLL_CTL_DEL, fd, NULL);
        if (ret == -1) {
            log("%s", strerror(errno));
            return -1;
        }
        return ret;
    }
    struct epoll_event* Reactor::Wait(int& evnetNum)
    {
        int ret = 0;
        memset(impl->epollEventArray, 0, sizeof(impl->epollEventArray));
        //now the third and fourth agrument is fixed
        //it will be alternative
        std::lock_guard< std::mutex > lockEpoll(impl->epollMutex);
        ret = epoll_wait(impl->epollFd, impl->epollEventArray, 1024, 800);
        evnetNum = ret;
        //std::cerr << ret << " events arrived" << std::endl;
        if (-1 == ret)
            return nullptr;
        //if do it here, this method will be complex
        //in this version, we delete Service Code successfuly
        return impl->epollEventArray;
    }
    

}
