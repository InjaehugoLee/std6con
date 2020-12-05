#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>      //printf
#include <stdlib.h>      //atoi
#include <string.h>      //strerror()
#include <errno.h>      //errno
#include <winsock2.h>
#include <WS2tcpip.h>   //inet_ntop

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

        req[0] = ntohl(req[0]);     //변환 쪽이네 ㅇㅅㅇ 
        req[1] = ntohl(req[1]);
        req[2] = ntohl(req[2]);


        //클라이언트로부터 요청받은 데이터 개수가 0개 이하이거나
        //6개를 초과하면 디폴트로 2개만 처리하도록 함.
        /*if (req[1] <= 0 || 6 < req[1])req[1] = 2;*/

        printf("recv:[%d,%d,%d]\n", req[0], req[1], req[2]);

        //----------------------------------------
        //data생성
        int data[10];
        for (int i = 0; i < req[2]; i++)                    //req[2]이 client의 n
        {
            data[i] = htonl(req[0] * i);                    //req[0]이 client의 s
        }
        //----------------------------------------

        char sendstr[1024];

        const char* mess = "send by B";                     //넣을 string?

        int offset = 0;                                     //offset 초기화
        int tsize = htonl(req[2]);                          //total size -> 총길이를 4byte로 들어가기 위한 준비. 00 05
        memcpy(sendstr, &tsize, sizeof(int));               //제일 앞에 전체 길이 넣을 메모리? , 주소값 ( 값) , 사이즈 크기4byte
        offset += sizeof(int);                              //offset은 int형 4byte를 썻으니까 뒤로 4byte밀려나감. (더해짐)

        memcpy(sendstr + offset, data, sizeof(int) * req[2]);
        offset += (sizeof(int) * req[2]);                   //항상 req[2]처럼 맨뒤에있는 값

        int slen = strlen(mess);                            //mess의 길이를 먼저 slen에 저장.
        int tlen = htonl(slen);                             //slen의 길이를 net형식으로 저장한다 Big(보통숫자)으로 order하고 tlen에 저장한다.

        memcpy(sendstr + offset, &tlen, sizeof(int));       //sendstr 마지막에 4byte(int크기)에 mess의 길이 를 넣어준다.
        offset += sizeof(int);                              //off 밀려남 (더해짐)

        memcpy(sendstr + offset, mess, slen);               //진짜 mess에 대한 문자를 sendstr에 넣는 과정
        offset += slen;                                     //offset을 slen(mess의 길이) 만큼 더 해준다.

        send(clientSocket, sendstr, offset, 0);             //send에 sendstr을 전송하는데 얼만큼 (offset)길이 만큼 보내준다.        

        printf("send data\n");
        //**************************************************************************************

        closesocket(clientSocket);//Close client socket
    }
    WSACleanup();

    return 1;
}