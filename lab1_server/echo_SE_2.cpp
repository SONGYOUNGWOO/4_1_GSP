#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib, "WS2_32.LIB")

//using namespace std;

const short SERVER_PORT = 3000;
const int BUFSIZE = 1024;


int main()
{

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 0), &WSAData);

	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
	if (s_socket <= 0) std::cout << "socket error" << std::endl;
	else std::cout << "socket success" << std::endl;	

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(s_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(SOCKADDR_IN));
	listen(s_socket, SOMAXCONN);
	INT addr_size = sizeof(SOCKADDR_IN);
	SOCKET c_socket = WSAAccept(s_socket, 
		reinterpret_cast<sockaddr*>(&addr), &addr_size, NULL, NULL);



	while (true) {

		char recv_buf[BUFSIZE];
		WSABUF recv_wsabuf[1];
		recv_wsabuf[0].len = sizeof(recv_buf);
		recv_wsabuf[0].buf = recv_buf;
		DWORD recv_byte;
		DWORD recv_flag = 0;
		auto ret = WSARecv(c_socket, recv_wsabuf, 1, &recv_byte, &recv_flag, NULL, NULL);
		recv_buf[recv_byte] = 0;
		std::cout << "From Client [" << recv_byte << "bytes] : " << recv_buf << std::endl;

		char buf[BUFSIZE];
		memcpy(buf, recv_buf, recv_byte);
		WSABUF wsabuf[1];
		wsabuf[0].buf = buf;
		wsabuf[0].len = static_cast<ULONG>(recv_byte);
		DWORD sent_byte = 0;
		WSASend(c_socket, wsabuf, 1, &sent_byte, 0, NULL, NULL);

	}
	closesocket(c_socket);
	WSACleanup();
}