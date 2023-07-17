#include <Protocol/Protocol.hpp>
#include <gtest/gtest.h>

// TEST(ProtocolTest, Placeholder)
// {
//     Protocol::Protocol protocol;
//     ASSERT_NE(protocol.Init(), -1);
//     ASSERT_NE(protocol.Listen(), -1);
//     ASSERT_NE(protocol.ConnectionService(), -1);
//     ASSERT_TRUE(protocol.Run());
// }
int main(int argc, char *argv[])
{
    pid_t pid;
    Protocol::Protocol protocol(std::string("9901"));
    protocol.Init();
    protocol.Listen();
    protocol.ConnectionService();
    while(true)
    protocol.Run();
    return 0;
}
