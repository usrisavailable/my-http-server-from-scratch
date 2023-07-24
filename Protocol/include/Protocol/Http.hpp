#pragma once

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

#include <Protocol/Protocol.hpp>

namespace Http{
    /**these method to do specific things
     * time and image are suported
     * need to complete it
     */
    class Connection;

    static void
    TimeService(int fd)
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
        //now the collect task do not perform here
        //close(fd);
        std::cerr << "service is: TimeService" << std::endl;
        std::cerr << "the thread is: " << std::this_thread::get_id() << std::endl;
        return;
    }
    static void
    ImageService(int fd)
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
        //now the resource collect do not perform here
        //close(fd);
        munmap(addr, length);
        std::cerr << "service is: ImageService" << std::endl;
        std::cerr << "the thread is: " << std::this_thread::get_id() << std::endl;
        return;
    }
    static void
    DefaultService(int fd)
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
        static char *addr = NULL;
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
        //close(fd);
        munmap(addr, length);
        std::cerr << "service is: DefaultService" << std::endl;
        std::cerr << "the thread is: " << std::this_thread::get_id() << std::endl;
        return;
    }
}
