/**this module do the job I/O multiplex
 * depends on system call like epoll
 */
#pragma once

#include <sys/epoll.h>
#include <memory>

namespace Reactor{
    /**this class control the reactor instance
     * need to design all API arguments exposed to user
     * data access is still denied beyond the class
     */
    class Reactor : public std::enable_shared_from_this< Reactor >{
    public:
        //life cycle mangement
        Reactor();
        ~Reactor();
    public:
        //do not copy or assignment
        Reactor(const Reactor&) = default;
        Reactor(const Reactor&&) = delete;
        Reactor& operator=(const Reactor&) = delete;
        Reactor& operator=(const Reactor&&) = delete;
    public:
        /**this method get a insatace of the origin data object
         * how to design this API?
         * @ret a pointer pointed to the objiect
         */
        std::shared_ptr< Reactor > data();
        /**this method initialize the data object
         * @ret -1 on error and non zero on failed
         */
        int Init();
        /**this method is a wrapper of epoll_ctl
         * @param fd is the resource need to monitor
         * @ret -1 on failed, and other on success
         */
        int Add(int fd);
        /**this method is a wrapper of epoll_ctl
         * @param fd is the resource need to monitor
         * @ret -1 on failed, and other on success
         */
        int Del(int fd);
        /**this method is a wrapper of epoll_wati
         * for now, evnet handler is not ready, maybe in other class 
         * @ret -1 on failed, and other on success
         */
        int Wait(int servfd);
    private:
        struct ReactorData;
        std::unique_ptr< ReactorData > impl;
    };
}
