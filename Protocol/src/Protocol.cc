#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <thread>
#include <iostream>
#include <mutex>
#include <atomic>

#include <Protocol/Protocol.hpp>
#include <Reactor/Reactor.hpp>

namespace Protocol{
    //useless data statistics
    //std::atomic_int threadCount;
    //type definition
    struct Protocol::server {
        int servfd;
        std::string port;
        Reactor::Reactor reactor;   //defaul initialization
    };
    Protocol::Protocol(const std::string port):
        impl(new server)
    {
        impl->port = port;
    }
    Protocol::~Protocol()
    {
        //for now, nothing need to do
    }
    int Protocol::Init()
    {
        //first thing, set deamon process
        // {
        //     switch(fork()) {
        //         case -1:
        //             return -1;
        //         case 0:
        //             break;
        //         default:
        //             _exit(0);
        //     }
        //     umask(0);
        //     int fd = open("/dev/null", O_RDWR);
        //     dup2(fd, STDIN_FILENO);
        //     dup2(fd, STDOUT_FILENO);
        //     dup2(fd, STDERR_FILENO);
        //     close(fd);
        //     return setsid();
        // }
        //second thing, avoid asynchronous signal stop the process
        {
            struct sigaction sa;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            sa.sa_handler = SIG_IGN;
            if (-1 == sigaction(SIGPIPE, &sa, NULL))
                return -1;
        }
       
        return 0;
        
    }
    int Protocol::Listen()
    {
        //impl->port = "9901";
        impl->servfd = 0;

        impl->servfd = socket(AF_INET, SOCK_STREAM, 0);
        if (impl->servfd <= 0) {
            perror("socket error");
            return -1;
        }

        /* make sure the socket is non-blocking */
        {
            int flags = 0;
            int ret = 0;
            flags = fcntl(impl->servfd, F_GETFL, 0);
            ret = fcntl(impl->servfd, F_SETFL, flags | O_NONBLOCK);
            if(ret < 0) {
                perror("couldn't set socket as non blocking! ");
                close(impl->servfd);
                return -1;
            }
        }
        /* prevent address from being taken */
        {
            int optval = 1;
            setsockopt(impl->servfd, SOL_SOCKET, SO_REUSEADDR,
                        &optval, sizeof(optval));
        }
        /* bind the address to the socket */
        {
            struct sockaddr_in servAddress;
            memset(&servAddress, 0, sizeof(sockaddr_in));
            servAddress.sin_addr.s_addr = htonl(INADDR_ANY);
            servAddress.sin_family = AF_INET;
            servAddress.sin_port = htons(static_cast<in_port_t>(stoi(impl->port)));
            
            if (bind(impl->servfd, (struct sockaddr*)(&servAddress), 
                        sizeof(servAddress)) == -1){
                perror("bind error");
                close(impl->servfd);
                return -1;
            }

        }
        /* listen incoming */
        if (listen(impl->servfd, SOMAXCONN) < 0) {
            perror("couldn't start listening");
            close(impl->servfd);
            return -1;
        }
        return impl->servfd;
    }

    int Protocol::ConnectionService()
    {
        int ret;
        ret = impl->reactor.Init();
        if (ret == -1)
            return -1;

        if (-1 == impl->reactor.Add(impl->servfd))
            return -1;
        return 0;
    }
    
    bool Protocol::Run()
    {
        //for now ,we created thread dynamically
        //especially, only one thread can call epoll_wait at a time
        //this method has many problems

        int ret = impl->reactor.Wait(impl->servfd);\
        switch (ret)
        {
        case 0/* constant-expression */:
            /* code */
            //std::cerr << "wait time out, recall it later" << std::endl;
            break;
        case -1:
            perror("system call failed: ");
            this->Close();
            exit(-1);
        default:
            break;
        }
        return true;
    }
    
    int Protocol::Close()
    {
        //clear connetcion
        close(impl->servfd);
        return 0;
    }

}
