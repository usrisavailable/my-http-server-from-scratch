/**this module contain everything
 * for now, I/O MUX and dispatcher are included
 * the instance created by Protocol control everything
 */
#pragma once

#include <memory>

#include <Reactor/Reactor.hpp>

namespace Protocol{
    /**this class used to define the server
     * behaviors and creation and destroy
     */
    class Protocol{
    public:
        //methods below is the lifetime mangement
        /**initialize the some data of the server object
         * @param port a server listenting to
         */
        Protocol(const std::string port);
        /**destroy all data
         */
        ~Protocol();
    public:
        Protocol(const Protocol&) = delete;
        Protocol(const Protocol&&) = delete;
        Protocol& operator=(const Protocol&) = delete;
        Protocol&& operator=(const Protocol&&) = delete;
    public:
        /**this method read the basic configuration item
         * for now, we just put the string in argv
         * @ret a integer greater than 0 or -1 when error occured
         */
        int Init();
        /**this method make the socket becoming listened status
         * @ret a integer greater than 0 or -1 when error occured
         */
        int Listen();
        /**this method make the service started
         * @ret a integer greater than 0 or -1 when error occured
         */
        int ConnectionService();
        /**this method keeps the service running
         * the logic control need to call this method again and again
         * @ret true indicate event occured, false indiacate no event
         */
        bool Run();
        /**this method close the service
         * and collect the resource 
         * for now, call close() directly
         */
        int Close();
        /**this method do real work
         * this will do now, later think about other way
         * @param epollEventArray is returned by wait, and eventNum is all event occured
         */
        void PerformTask(struct epoll_event* epollEventArray, int eventNum);
        //for now, it is useless
        void IsRun(bool flag)
        {
            this->run = flag;
        }
    private:
        std::atomic_bool run;
        struct server;
        std::unique_ptr< struct server > impl;
    };

    /**we need create a connection pool
     * it is used to bind a fd and its behavior
     * not need a new a module
     * besides, a fd should be assoicated wiht a Connection object.
     */
    //class Connection;
    //using callbackFn = std::function< void(Connection*) >;
    class Connection{
    public:
        /**life cycle mangement
         * though all member is public
         * @ret none, just perform none-staitc member initialization
         */
        Connection();
        virtual ~Connection();
    public:
        Connection(const Connection&) = default;
        Connection& operator=(const Connection&) = default;
        Connection(const Connection&&) = delete;
        Connection& operator=(const Connection&&) = delete;
    public:
        /**a sign used to distinguish server socket and client socket
         * class user need to pass the serverfd(no better way?)
         * @param file descriptor need to judgement
         * @ret server for ture, otherwise false
         */
        bool IsServer(int fd);
    public:
        //the remote file descriptor
        int fd;
        //to observe the connection, wether or not expired
        //so, we do not consider the value of fd
        int status;
        //reserve a Reactor object
        std::shared_ptr< Reactor::Reactor > reactor;
        //function object is better than function pointer
        //may be a better way???
        //it appears so weird, ???
        /**this method accept a remote connection
         * @param connPoll is a connection poll that contain all connected connection
         * @ret 0 for sucess and -1 for error
         */
        virtual int AcceptRemoteClient(std::array< Connection*, 1024>& connPool);
        /**this method just as its name implies
         * how it works depennd on what the service can do
         * @ret 
         */
        virtual int ReadFromRemoteClient() ;
        /**this method just as its name implies
         * how it works depennd on what the service can do
         * @ret 
         */
        virtual int WriteToRemoteClient();
    };
    //we still follow composition over in inheritance
    class ConnectionEcho : public Connection{
    public:
        /**life cycle mangement
         * though all member is public
         * @ret none, just perform none-staitc member initialization
         */
        ConnectionEcho();
        virtual ~ConnectionEcho();
    public:
        ConnectionEcho(const ConnectionEcho&) = default;
        ConnectionEcho& operator=(const ConnectionEcho&) = default;
        ConnectionEcho(const ConnectionEcho&&) = delete;
        ConnectionEcho& operator=(const ConnectionEcho&&) = delete;
    public:
        //for echo service, two methods are enough
        /**this method just as its name implies
         * how it works depennd on what the service can do
         * @ret 
         */
        virtual int ReadFromRemoteClient() override;
        /**this method just as its name implies
         * how it works depennd on what the service can do
         * @ret 
         */
        virtual int WriteToRemoteClient() override;
        /**echo service
         * receive session message first and then receive payload message
         * @ret 0 for success or -1 for fail
         */
        int EchoRDWR();
    };
}
