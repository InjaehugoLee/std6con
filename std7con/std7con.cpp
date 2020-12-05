//std1Console.cpp

#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib")

int main(int argc, char* argv[])
{
    int ret;
    int s, e, n;
    //const char* d = "hello world";
    if (argc != 4)
    {
        s = 100;
        e = 200;
        n = 5;
    }
    else
    {
        s = atoi(argv[1]);
        e = atoi(argv[2]);
        n = atoi(argv[3]);
    }

    //----------------------------------------------
    //validate

    const char* serverip = "127.0.0.1";
    short port = 5000;
    short count;
    if (argc != 2)count = 2;
    else        count = atoi(argv[1]);

    //****************************************************************
    //--------------------------------------------------
    //initialize windows socket
    WSADATA wsaData = { 0 };
    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0)
    {
        printf("WSAStartup failed: %d\n", ret);
        return 1;
    }

    //-------------------------------------------------
    //socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        printf("socket() failed![%d]\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    //--------------------------------------------------
    sockaddr_in name;
    memset(&name, 0x00, sizeof(name));
    name.sin_family = AF_INET;
    inet_pton(AF_INET, serverip, &name.sin_addr);
    name.sin_port = htons(port);

    //connect
    if (connect(sock, (sockaddr*)&name, sizeof(name)) == SOCKET_ERROR)
    {
        printf("socket() failed![%d]\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    //************************************************************************

    int type[3];
    //type[0] = htons(0);//host to network short
    //type[1] = htons(count);
    type[0] = htonl(s);
    type[1] = htonl(e);
    type[2] = htonl(n);

    //send
    if (send(sock, (const char*)type, sizeof(type), 0) == SOCKET_ERROR)
    {
        printf("send() failed![%d]\n", WSAGetLastError());
    }
    else
        printf("success\n");

    int tsize;
    recv(sock, (char*)&tsize, sizeof(int), 0);          //전체개수
    tsize = ntohl(tsize);

    int data[10];
    recv(sock, (char*)data, sizeof(int) * tsize, 0);

    printf("count : %d\n", tsize);

    for (int i = 0; i < tsize; i++)
    {
        data[i] = ntohl(data[i]);
        printf("%d, ", data[i]);
    }
    printf("\n");

    //-------------------------------------------------------s, e, n을 출력한 부분 


    // ------------------------------------------------ 추가 문자 넣은것

    int tttt = 0;                                       //tttt은 초기값 없어도 된다.
    recv(sock, (char*)&tttt, sizeof(int), 0);           //server에서 받은 값, tttt에 4byte에 mess의 크기(9)를 저장한다?
    int tlen = ntohl(tttt);                             //tttt라는 값이 Big(현실 쓰는 숫자) - > little(network) 을 바꿔주고 tlen에 저장

    char msg[1024];                             //문자열을 저장할 배열 1024크기 선언 : msg

    recv(sock, msg, tlen, 0);                   //server에서 mess를 받아왔죠, 길이가 tlen만큼을 msg에 mess문자를 저장한다. 
    msg[tlen] = '\0';                           //msg[10]번째에 문자의 마지막 선언인 '\0'을 넣는다.

    printf("msg배열 : %s\n", msg);              //msg를 print한다.

    closesocket(sock);
    WSACleanup();
}