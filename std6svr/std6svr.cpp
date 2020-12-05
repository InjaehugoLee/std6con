#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>      //printf
#include <stdlib.h>      //atoi
#include <string.h>      //strerror()
#include <errno.h>      //errno
#include <winsock2.h>
#include <WS2tcpip.h>   //inet_ntop
#include <time.h>
#include <iostream>
using namespace std;

#pragma comment(lib,"ws2_32.lib")

#define MAXPENDING         5   //Maximum outstanding connection requests
#define STRINGBUFSIZE   1024


int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    SOCKADDR_IN serverAddress, clientAddress;

    if (argc != 2)//Test for correct number of arguments
    {
        printf("std1Svr <Server Port>\n");
        return 0;
    }

    //소켓 라이브러리 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup() Error!\n");
        return 0;
    }

    int servPort = atoi(argv[1]);   //First arg: local port

    //=======================================================
    //1.socket() - Create socket for incoming connections
    //=======================================================
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        printf("socket() failed![%d:%s]\n", errno, strerror(errno));
        return 0;
    }

    //Construct local address structure
    memset(&serverAddress, 0, sizeof(serverAddress));   //Zero out structure
    serverAddress.sin_family = AF_INET;               //IPv4 address family
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);   //Any incoming interface
    serverAddress.sin_port = htons(servPort);         //Local port

    //========================================================
    //2.bin() - Bin to the local address
    //========================================================
    if (bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        printf("bind() failed![%d:%s]\n", errno, strerror(errno));
        closesocket(serverSocket);
        return 0;
    }

    //=========================================================
    //3.listen() - Mark the socket so it will listen for incoming connections
    //=========================================================
    if (listen(serverSocket, MAXPENDING) == SOCKET_ERROR)
    {
        printf("listen() failed![%d:%s]\n", errno, strerror(errno));
        closesocket(serverSocket);
        return 0;
    }

    char clientName[STRINGBUFSIZE];

    int clientAddressLen = sizeof(clientAddress);

    printf("stand by...\n");

    for (;;)
    {
        clientName[0] = '\0';   //setempty

        //==============================================
        //4.accept() - Wait for a client to connect --> block mode
        //==============================================
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLen);
        if (clientSocket < 0)
        {
            printf("accept() failed![%d:%s]\n", errno, strerror(errno));
            continue;
        }

        //clientSocket is connected to a client!
        if (inet_ntop(AF_INET, &clientAddress.sin_addr.s_addr, clientName, sizeof(clientName)) == NULL)
        {
            printf("Unable to get client address![%d:%s]\n", errno, strerror(errno));
            closesocket(clientSocket);
            continue;
        }

        printf("[%s:%d]", clientName, ntohs(clientAddress.sin_port));

        //**************************************************************************************
        //========================================================
        //5.recv() - Receive message from client
        //========================================================
        int req[3];

        int numBytesRcvd = recv(clientSocket, (char*)req, sizeof(req), 0);

        req[0] = ntohl(req[0]); // 리틀 엔디언으로 바꿔야 하는 이유: 우리 컴퓨터가 리틀 엔디언 cpu라서.
        req[1] = ntohl(req[1]);
        req[2] = ntohl(req[2]);


        //클라이언트로부터 요청받은 데이터 개수가 0개 이하이거나
        //6개를 초과하면 디폴트로 2개만 처리하도록 함.
        /*if (req[1] <= 0 || 6 < req[1])req[1] = 2;*/

        printf("recv:[%d,%d,%d]\n", req[0], req[1], req[2]);

        //----------------------------------------
        //data생성

        srand(time(NULL));
        int data[10];
        for (int i = 0; i < req[2]; i++)
        {
            data[i] = htonl(req[0] + (rand() % 100) + 1);
        }
        //----------------------------------------
        const char* str1 = "hello";
        char sendstr[1024];

        int tsize = htonl(req[2]); // 클라이언트로 보내려면  빅 엔디언 형식으로 바꿔야함.
        memcpy(sendstr, &tsize, sizeof(int));//제일 앞에 전체 길이

        memcpy(sendstr + 4, data, sizeof(int) * req[2]);

        int len = htonl(strlen(str1));

        memcpy(sendstr + 4 + (sizeof(int) * req[2]),&len , sizeof(int));

        memcpy(sendstr + 8 + (sizeof(int) * req[2]), str1, strlen(str1));

    
        send(clientSocket, sendstr, 8 + (req[2] * sizeof(int)) + strlen(str1), 0);

        printf("send data\n");
        //**************************************************************************************

        closesocket(clientSocket);//Close client socket
    }
    WSACleanup();

    return 1;
}