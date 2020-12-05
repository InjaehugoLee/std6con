//std1Console.cpp

#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string>

using namespace std;

#pragma comment(lib,"ws2_32.lib")

int main(int argc, char* argv[])
{
    int ret;
    int s, e, n;
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
    recv(sock, (char*)&tsize, sizeof(int), 0);//전체개수
    tsize = ntohl(tsize);

    int data[10];
    recv(sock, (char*)data, sizeof(int) * tsize, 0);

    printf("count : %d\n", tsize);

    for (int i = 0; i < tsize; i++)
    {
        data[i] = ntohl(data[i]);
        printf("%d, ", data[i]);
       
    }

    int a;
    recv(sock, (char*)&a, sizeof(int), 0);
    a = ntohl(a);

    char b[256];
    recv(sock, (char*)b, a, 0);
    b[a] = '\0';
      
    printf("%s", b);
    printf("\n");

    closesocket(sock);
    WSACleanup();
}