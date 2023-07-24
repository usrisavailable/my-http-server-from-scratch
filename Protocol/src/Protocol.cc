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
#include <Protocol/Http.hpp>

#define log(fmt, ...) \
    fprintf(stderr, "line%d: " fmt "\n", __LINE__, __VA_ARGS__)

namespace Protocol{
    //useless data statistics
    //std::atomic_int threadCount;
    //type definition
    struct Protocol::server {
        int servfd;
        std::string port;
        std::shared_ptr< Reactor::Reactor > reactor;   //default initialization
        std::array< Connection*, 1024> connPool;
    };
    Protocol::Protocol(const std::string port):
        impl(new server)
    {
        impl->port = port;
        impl->reactor = std::make_shared< Reactor::Reactor >();
        for (int n = 0; n < impl->connPool.size(); n++)
        {   
            //every Connection object should have a Reactor object
            impl->connPool[n] = new Connection();
            impl->connPool[n]->reactor = impl->reactor->data();
        }
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
        ret = impl->reactor->Init();
        if (ret == -1)
            return -1;
        //bind serverfd and connection object
        Connection *conn;
        {
            for (auto& iter : impl->connPool)
                if (!iter->status)
                    {conn = iter; break;}
        }
        conn->fd = impl->servfd;
        conn->status = 1;
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
        ev.data.ptr = conn;
        if (-1 == impl->reactor->Add(impl->servfd, ev))
            return -1;
        return 0;
    }
    
    bool Protocol::Run()
    {
        //for now ,we created thread dynamically
        //especially, only one thread can call epoll_wait at a time
        //this method has many problems
        struct epoll_event* epollEventArray;
        int eventNum = 0;
        epollEventArray = impl->reactor->Wait(eventNum);
        switch (eventNum)
        {
        case 0 /* constant-expression */:
            /* code */
            //std::cerr << "wait time out, recall it later" << std::endl;
            break;
        case -1:
            perror("system call wait failed: ");
            this->Close();
            exit(-1);
        default:
            this->PerformTask(epollEventArray, eventNum);
            break;
        }
        return true;
    }
    void Protocol::PerformTask(struct epoll_event* epollEventArray, int eventNum)
    {
        int n = 0;
        Connection *conn = nullptr;
        int event;
        std::cerr << "the total event is: " << eventNum << std::endl;
        for (n; n < eventNum; n++)
        {
            conn = static_cast< Connection* >(epollEventArray[n].data.ptr);
            event = epollEventArray[n].events;
            std::cerr << "fd is: " << conn->fd << std::endl;
            std::cerr << "status is: " << conn->status << std::endl;
            if (conn->IsServer(impl->servfd))
            {
                conn->AcceptRemoteClient(impl->connPool);
            }
            else
            {
                //std::thread t;
                //now, we just use main thread
                if (event & EPOLLIN)
                    std::cerr << "EPOLLIN" << std::endl;
                if (event & EPOLLOUT)
                    std::cerr << "EPOLLOUT" << std::endl;
                if (event & EPOLLRDHUP)
                    std::cerr << "EPOLLRDHUP" << std::endl;
                conn->WriteToRemoteClient();
                //for static service, we need to collect resource
                //if (event & EPOLLRDHUP)
                {
                      impl->reactor->Del(conn->fd);
                      //std::cerr << impl->reactor.use_count() << std::endl;
                      close(conn->fd);
                      conn->status = 0;
                      conn->fd = -1;
                }
            }
        }
        return;
    }

    int Protocol::Close()
    {
        //clear connetcion
        close(impl->servfd);
        //some data object use new operator
        for (auto& iter : impl->connPool)
            delete iter;
        return 0;
    }

    //class Connetcion defined bleow
    Connection::Connection():
        fd(-1), status(0)
    {
    }
    Connection::~Connection()
    {}
    bool Connection::IsServer(int fd)
    {
        if (this->fd == fd)
            return true;
        return false;
    }
    //read and write callback function initialization
    int Connection::AcceptRemoteClient(std::array< Connection*, 1024>& connPool)
    {
        int remoteFd;
        Connection *conn = nullptr;
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
        while ((remoteFd = accept(this->fd, NULL, NULL)) > 0) {
            conn = nullptr;
            for(auto& iter : connPool)
            {
                if (!iter->status)
                    {conn = iter; break;}
            }
            if (conn == nullptr)
            {
                std::cerr << "connection poll is full, later try" << std::endl;
                return -1;
            }
            conn->fd = remoteFd;
            conn->status = 1;

            ev.data.ptr = conn;
            std::cerr << "the remote fd is:" << remoteFd << std::endl;
            this->reactor->Add(remoteFd, ev);
        }
        return 0;
    }
    int Connection::WriteToRemoteClient()
    {
        //now we provide two types of services
        //buffer for receive data delivered by client
        std::string buf;
        buf.resize(8192);
        recv(this->fd, const_cast<char *>(buf.data()), buf.size(), 0);
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
        servicesMap["image"] = Http::ImageService;
        servicesMap["time"] = Http::TimeService;
        servicesMap["notfind"] = Http::DefaultService;
        
        if (servicesMap.find(reqContent) != servicesMap.end())
            servicesMap[reqContent](this->fd);
        else
            servicesMap["notfind"](this->fd);
        
        return 0;
    }
    int Connection::ReadFromRemoteClient()
    {
        return 0;
    }

}
