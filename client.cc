#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <string>
#include <thread>

#pragma pack(1)
struct SessionMsg {
    int32_t number;
    int32_t length;
};
#pragma pack(1)

struct PayloadMsg {
        int32_t length;
        char data[0];
    };

using namespace std;

int main(int argc, char *argv[])
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servAddress;
    memset(&servAddress, 0, sizeof(sockaddr_in));
    servAddress.sin_addr.s_addr = inet_addr("118.89.68.241");
    servAddress.sin_family = AF_INET;
    servAddress.sin_port = htons(static_cast<in_port_t>(9901));
    int ret = connect(sockfd, (const struct sockaddr*)&servAddress, sizeof(sockaddr_in));
    perror("worng message is: ");
    assert(ret == 0);

    string receiveData;
    char buf[1024] ={0};
    string data;
    for (int i = 0; i < 8192; i++)
    data.append(string(to_string(1)));
    cout << data.size() << endl;

    int msglength = 1024;
    SessionMsg sessionMessage = {0, 0};
    sessionMessage.length = htonl(msglength);
    
    sessionMessage.number = htonl(atoi(argv[1]));
    int count = send(sockfd, &sessionMessage, sizeof(SessionMsg), 0);
    assert(count == sizeof(SessionMsg));
    count = recv(sockfd, buf, 3, 0);
    assert(count == 3);

    for (int i =0; i < atoi(argv[1]); i++){
        cout << i + 1 << endl;
        PayloadMsg *payloadMessage = static_cast<PayloadMsg*>(malloc(sizeof(int32_t) + 4096));
        payloadMessage->length = htonl(msglength);
        memcpy(payloadMessage->data, data.c_str(), msglength);
        count = send(sockfd, payloadMessage, sizeof(int32_t) + msglength, 0);
        assert(count == (sizeof(int32_t) + msglength));
        //memset(buf,0,1024);
        while ((count = recv(sockfd, buf, 3, 0)) == -1)
        cout << "count" << count << endl;
        cout << buf << endl;
        assert(count == 3);
    }
    //shutdown(sockfd, SHUT_WR);
    this_thread::sleep_for(chrono::milliseconds(800));
    close(sockfd);
    return 0;
}
