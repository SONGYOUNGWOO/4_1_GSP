#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib, "WS2_32.LIB")

//using namespace std;

const short SERVER_PORT = 3000;
const char* SERVER_ADDR = "127.0.0.1";

const int BUFSIZE = 1024;

int main()
{
	std::wcout.imbue(std::locale("korean"));

	WSADATA WSAData; //ms에서 windows 프로그래밍할 때 필요
	WSAStartup(MAKEWORD(2,0), &WSAData);

	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &addr.sin_addr);
	WSAConnect(c_socket, reinterpret_cast<sockaddr * >(&addr), 
		sizeof(SOCKADDR_IN), NULL, NULL, NULL,NULL);


	while (true) {

		char buf[BUFSIZE];
		std::cout << "INPUT : "; 
		std::cin.getline(buf, sizeof(buf));

		WSABUF wsabuf[1];
		wsabuf[0].buf = buf;
		wsabuf[0].len = static_cast<ULONG>(strlen(buf)); 

		DWORD sent_byte;
		WSASend(c_socket, wsabuf, 1, &sent_byte, 0, NULL, NULL);

		char recv_buf[BUFSIZE];

		WSABUF recv_wsabuf[1];
		recv_wsabuf[0].len = sizeof(recv_buf);
		recv_wsabuf[0].buf = recv_buf;

		DWORD recv_byte = 0;
		DWORD recv_flag = 0;
		WSARecv(c_socket, recv_wsabuf, 1, &recv_byte, &recv_flag, NULL, NULL);
		recv_buf[recv_byte] = 0;

		std::cout << "From Server [" << recv_byte << "bytes] : " << recv_buf << std::endl;

	}
	closesocket(c_socket);
	WSACleanup();	
}