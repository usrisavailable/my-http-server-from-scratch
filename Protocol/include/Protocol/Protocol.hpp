/**this module contain everything
 * for now, I/O MUX and dispatcher are included
 * the instance created by Protocol control everything
 */
#pragma once

#include <memory>

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
    private:
        struct server;
        std::unique_ptr< struct server > impl;
    };
}
