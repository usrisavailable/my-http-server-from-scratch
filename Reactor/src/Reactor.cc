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
    {}
    Reactor::~Reactor()
    {}
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
    int Reactor::Add(int fd)
    {
        int ret;
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
        ev.data.fd = fd;
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
        ret = close(fd);
        if (ret == -1) {
            log("%s", strerror(errno));
            return -1;
        }
        return ret;
    }
    void StaticService(int fd);
    int Reactor::Wait(int servfd)
    {
        int ret;
        memset(impl->epollEventArray, 0, sizeof(impl->epollEventArray));
        //now the third and fourth agrument is fixed
        //it will be alternative
        std::lock_guard< std::mutex > lockEpoll(impl->epollMutex);
        ret = epoll_wait(impl->epollFd, impl->epollEventArray, 1024, 800);
        //std::cerr << ret << " events arrived" << std::endl;
        if (-1 == ret)
            return -1;
        for (int i = 0; i < ret; i++)
        {
            struct epoll_event ev;
            if (impl->epollEventArray[i].data.fd == servfd)
            {
                //we dont care whether the system call success or not
                int remoteFd;
                struct epoll_event ev;
                while ((remoteFd = accept(impl->epollEventArray[i].data.fd, NULL, NULL)) > 0) {
                    std::cerr << "the remote fd is:" << remoteFd << std::endl;
                    ev.data.fd = remoteFd;
                    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                    epoll_ctl(impl->epollFd, EPOLL_CTL_ADD, remoteFd, &ev);
                }
            } else{
                struct epoll_event ev = impl->epollEventArray[i];
                if (!(ev.events & (EPOLLIN | EPOLLOUT)))
                    continue;
                std::thread t(StaticService, static_cast< int >(ev.data.fd));
                t.detach();
                continue;;
            }
        }
        //if do it here, this method will be complex
        return ret;
    }
    void TimeService(int fd)
    {
        int ret;
        time_t timeNow;
        time(&timeNow);
        std::string currentTime = ctime(&timeNow);
        //clear substring [pos, end)
        currentTime.erase(currentTime.find('\n'));
        currentTime.append(std::string("\r\n"));
        //construct http reponse head
        std::string ReponseHead = {
            "HTTP/1.1 200 OK\r\n"
            "Server: time web server\r\n"
            "Connection: close\r\n"
            "Content-type: text/plain\r\n"
        };
        //construct http reponse body
        std::string ReponseBody = {
             "the current time is:\r\n"
         };
        ReponseBody.append(currentTime);
        //only complete body, and acquire the real size
        ReponseHead.append(std::string("Content-Length: "));
        ReponseHead.append(std::to_string(ReponseBody.size()));
        ReponseHead.append(std::string("\r\n\r\n"));
        
        std::string sendBuf;
        sendBuf.append(ReponseHead);
        sendBuf.append(ReponseBody);

        //one connection just send once
        send(fd, sendBuf.c_str(), sendBuf.size(), 0);

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        //collect the system resource
        close(fd);
        std::cerr << "the connection is closed: " << fd << std::endl;
        std::cerr << "the thread is: " << std::this_thread::get_id() << std::endl;
        return;
    }
    void ImageService(int fd)
    {
        int ret;
        //construct the http reponse head
        std::string ReponseHead = {
            "HTTP/1.1 200 OK\r\n"
            "Server: time web server\r\n"
            "Connection: close\r\n"
            "Content-type: image/jpeg\r\n"
        };
        //one way is read the entire file
        //another way is system call mmap and unmmap
        //no matter how, the data is transmited by binary stream
        char *addr;
        int length;
        {
            //this section
            int fd = open("../Resource/demo.jpg", O_RDWR);
            //confirm the relative path
            //system("pwd");
            if (fd == -1)
                {
                    perror("open jpg file failed!");
                    exit(1);
                }
            struct stat statbuf;
            ret = fstat(fd, &statbuf);
            if (ret == -1)
                {
                    perror("fstat failed!");
                    exit(1);
                }
            length = statbuf.st_size;
            addr = static_cast< char *>(mmap(NULL, length, 
                    PROT_READ, MAP_PRIVATE, fd, 0));
            if (addr == MAP_FAILED)
                {
                    perror("mmap failed!");
                    exit(1);
                }
            //the length of ReponseBody must be correct, or failed
            ReponseHead.append(std::string("Content-Length: "));
            ReponseHead.append(std::to_string(statbuf.st_size));
            ReponseHead.append(std::string("\r\n\r\n"));
            //the open file descriptor can be collected now
            close(fd);
        }

        //ReponseHead and ReponseBody can be send separately
        std::string sendBuf;
        sendBuf.append(ReponseHead);
        //std::cerr << "http content length is: " << ReponseBody.size() << std::endl;
        //one connection just send once
        send(fd, sendBuf.c_str(), sendBuf.size(), 0);
        send(fd, addr, length, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        //after provid static content, collect the resource
        close(fd);
        munmap(addr, length);
        std::cerr << "the connection is closed: " << fd << std::endl;
        std::cerr << "the thread is: " << std::this_thread::get_id() << std::endl;
        return;
    }
    void DefaultService(int fd)
    {
        int ret;
        //construct the http reponse head
        std::string ReponseHead = {
            "HTTP/1.1 200 OK\r\n"
            "Server: time web server\r\n"
            "Connection: close\r\n"
            "Content-type: text/html\r\n"
        };
        //one way is read the entire file
        //another way is system call mmap and unmmap
        //no matter how, the data is transmited by binary stream
        char *addr;
        int length;
        {
            //this section
            int fd = open("../Resource/index.html", O_RDWR);
            //confirm the relative path
            //system("pwd");
            if (fd == -1)
                {
                    perror("open html file failed!");
                    exit(1);
                }
            struct stat statbuf;
            ret = fstat(fd, &statbuf);
            if (ret == -1)
                {
                    perror("fstat failed!");
                    exit(1);
                }
            length = statbuf.st_size;
            addr = static_cast< char *>(mmap(NULL, length, 
                    PROT_READ, MAP_PRIVATE, fd, 0));
            if (addr == MAP_FAILED)
                {
                    perror("mmap failed!");
                    exit(1);
                }
            //the length of ReponseBody must be correct, or failed
            ReponseHead.append(std::string("Content-Length: "));
            ReponseHead.append(std::to_string(statbuf.st_size));
            ReponseHead.append(std::string("\r\n\r\n"));
            //the open file descriptor can be collected now
            close(fd);
        }

        //ReponseHead and ReponseBody can be send separately
        std::string sendBuf;
        sendBuf.append(ReponseHead);
        //std::cerr << "http content length is: " << ReponseBody.size() << std::endl;
        //one connection just send once
        send(fd, sendBuf.c_str(), sendBuf.size(), 0);
        send(fd, addr, length, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        //after provid static content, collect the resource
        close(fd);
        munmap(addr, length);
        std::cerr << "the connection is closed: " << fd << std::endl;
        std::cerr << "the thread is: " << std::this_thread::get_id() << std::endl;
        return;
    }
    void StaticService(int fd)
    {
        //now we provide two types of services
        //buffer for receive data delivered by client
        std::string buf;
        buf.resize(8192);
        recv(fd, const_cast<char *>(buf.data()), buf.size(), 0);
        //std::cout << buf << std::endl;

        //we need to get the request URI
        std::string separatorOne = "/";
        std::string separatorTwo = " ";
        std::string reqContent;
        //std::string::iterator iter = reqContent.begin();
        std::string::size_type currPos = 0;
        std::string::size_type endPos = std::string::npos;
        {
            currPos = buf.find(separatorOne);
            ++currPos;
            endPos = buf.find(separatorTwo, currPos);
            //this way is useless
            /* iter += currPos;
            int len = 0;
            while (!isspace(*iter))
            {
                len++;
                iter++;
            } */
            reqContent = buf.substr(currPos, endPos - currPos);
            //std::cout << "the URI is:" << reqContent << std::endl;
            // std::this_thread::sleep_for(std::chrono::milliseconds(800));
            // exit(1);
        }
        //initialize and register all types services
        std::map< std::string, std::function< void(int) > > servicesMap;
        servicesMap["image"] = ImageService;
        servicesMap["time"] = TimeService;
        servicesMap["notfind"] = DefaultService;
        
        if (servicesMap.find(reqContent) != servicesMap.end())
            servicesMap[reqContent](fd);
        else
            servicesMap["notfind"](fd);
        
        return;
    }
}
