#pragma once

namespace Comm{
    /**message head
    * packet amount is number; length Byte each packet
    * 1 Byte alignment
    */
    #pragma pack(1)
    struct SessionMsg {
        int32_t number;
        int32_t length;
    };
    #pragma pack(1)

    /**message body
    * @param length stands for this packet is length Byte
    * @data is variable length array, it has the remaining memory of the structure
    */
    struct PayloadMsg {
        int32_t length;
        char data[0];
    };
    /**a simple wrapper function
     * just read Bytes the third argument specifies
     */
    int SendAll(int fd, void* buf, int length, int flag)
    {
        int bufPtr = 0;
        int count = 0;
        int len = length;
        while(len)
        {
            count = send(fd, buf + bufPtr, length - bufPtr, 0);
            if (count == -1)
                return -1;
            len -= count;
            bufPtr += count;
        }
        std::cerr << "send byte number is: " << bufPtr << std::endl;
        assert(bufPtr == length);
        return 0;
    }
    int RecvAll(int fd, void* buf, int length, int flag)
    {
        int bufPtr = 0;
        int count = 0;
        int len = length;
        while(len){
            count = recv(fd, buf, length - count, flag);
            if (count == -1)
                return -1;
            len -= count;
            bufPtr += count;
        }
        std::cerr << "recv byte number is: " << bufPtr << std::endl;
        assert(bufPtr == length);
        return 0;
    }
}
