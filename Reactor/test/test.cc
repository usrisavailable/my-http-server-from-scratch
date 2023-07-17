#include <Reactor/Reactor.hpp>
#include <gtest/gtest.h>

TEST(ReactorTest, Placeholder)
{
    Reactor::Reactor reactor;
    ASSERT_NE(reactor.Init(), -1);
    ASSERT_EQ(reactor.Add(5), -1);
    ASSERT_EQ(reactor.Del(6), -1);
    ASSERT_EQ(reactor.Wait(7), 0);
}
// int main(int argc, char *argv[])
// {
//     pid_t pid;
//     Protocol::Protocol protocol(std::string("9901"));
//     protocol.Init();
//     protocol.Listen();
//     protocol.ConnectionService();
//     while(true)
//     protocol.Run();
//     return 0;
// }
