#include <Reactor/Reactor.hpp>
#include <gtest/gtest.h>
#include <Protocol/Protocol.hpp>

#include <thread>
#include <atomic>
#include <sys/time.h>

using namespace std;

static atomic< bool > run(false);
static void
TimeHandler(int sig)
{
    cout << "time to terminate the process" << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    run = true;
    //kill(getpid(), SIGTERM);
    return;
}
int main(int argc, char *argv[])
{
    pid_t pid = getpgid(0);
    cout << "the current id of process is:" << pid << endl;
    {
        //use a timer to terminate the process
        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = TimeHandler;
        if (sigaction(SIGALRM, &sa, NULL) == -1)
            exit(1);
    }
    {
        //set a timer, expiration for once
        //now server will close after 1 minutes
        struct itimerval itv;
        itv.it_interval.tv_sec = 0;
        itv.it_interval.tv_usec = 0;
        itv.it_value.tv_sec = 60 * 2;
        itv.it_value.tv_usec = 0;
        if (setitimer(ITIMER_REAL, &itv, 0) == -1)
            cout << "set timer failed, stop it manually" << endl;
    }
    
    string port;
    if (argc == 2)
        port = string(argv[1]);
    else
        port = "80";
    
    Protocol::Protocol timeService(port);
    timeService.Init();
    timeService.Listen();
    timeService.ConnectionService();
    //if we use signal 
    while (timeService.Run()) {
        if (run)
            break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    timeService.Close();
    cout << "process terminated!" << endl;
    return 0;
}
