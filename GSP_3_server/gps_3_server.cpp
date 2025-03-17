#include <iostream>
#include <WS2tcpip.h>
#include <windows.h>
#pragma comment (lib, "WS2_32.LIB")

const int BOARD_SIZE = 8; // 체스판 크기 (8x8)

// 체스말 구조체 (초기 위치는 (3,3))
struct ChessPiece {
    short x;
    short y;
};

// 이동 명령 열거형 (short 값)
enum MovementCommand : short {
    CMD_UP = 1,
    CMD_DOWN = 2,
    CMD_LEFT = 3,
    CMD_RIGHT = 4
};

// 좌표 구조체 (서버가 클라이언트로 전송할 데이터)
struct Coordinates {
    short x;
    short y;
};

// 에러 메시지 출력 함수
void error_display(const char* msg, int err_no)
{
    WCHAR* lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        err_no,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        NULL
    );
    std::cout << msg;
    std::wcout << L" 에러 " << lpMsgBuf << std::endl;
    while (true); // 디버깅용 무한루프
    LocalFree(lpMsgBuf);
}

// updatePiece: short 명령에 따라 체스말 위치 업데이트
void updatePiece(ChessPiece& piece, short command)
{
    switch (command) {
    case CMD_UP:
        if (piece.y > 0)
            piece.y--;
        break;
    case CMD_DOWN:
        if (piece.y < BOARD_SIZE - 1)
            piece.y++;
        break;
    case CMD_LEFT:
        if (piece.x > 0)
            piece.x--;
        break;
    case CMD_RIGHT:
        if (piece.x < BOARD_SIZE - 1)
            piece.x++;
        break;
    default:
        break;
    }
}

// Server 클래스: 소켓 생성, 클라이언트 연결 수락 및 통신 처리
class Server {
public:
    Server(short port) : port_(port), serverSocket_(INVALID_SOCKET), clientSocket_(INVALID_SOCKET) {
        piece_.x = 3;
        piece_.y = 3;

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            error_display("WSAStartup failed", WSAGetLastError());
        }
        serverSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket_ == INVALID_SOCKET) {
            error_display("Socket creation error", WSAGetLastError());
        }
        SOCKADDR_IN addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(serverSocket_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
            error_display("Bind error", WSAGetLastError());
        }
        if (listen(serverSocket_, SOMAXCONN) == SOCKET_ERROR) {
            error_display("Listen error", WSAGetLastError());
        }
        std::cout << "Server is listening on port " << port_ << std::endl;
    }

    ~Server() {
        if (clientSocket_ != INVALID_SOCKET)
            closesocket(clientSocket_);
        if (serverSocket_ != INVALID_SOCKET)
            closesocket(serverSocket_);
        WSACleanup();
    }

    void run() {
        std::cout << "Waiting for client connection..." << std::endl;
        clientSocket_ = accept(serverSocket_, nullptr, nullptr);
        if (clientSocket_ == INVALID_SOCKET) {
            error_display("Accept error", WSAGetLastError());
        }
        std::cout << "Client connected." << std::endl;

        while (true) {
            short cmd;
            int recv_bytes = recv(clientSocket_, reinterpret_cast<char*>(&cmd), sizeof(cmd), 0);
            if (recv_bytes != sizeof(cmd)) {
                std::cout << "Connection closed or error." << std::endl;
                break;
            }
            std::cout << "Received command: " << cmd << std::endl;
            updatePiece(piece_, cmd);

            Coordinates coords;
            coords.x = piece_.x;
            coords.y = piece_.y;
            int sent_bytes = send(clientSocket_, reinterpret_cast<char*>(&coords), sizeof(coords), 0);
            if (sent_bytes != sizeof(coords)) {
                error_display("Send error", WSAGetLastError());
                break;
            }
        }
    }

private:
    short port_;
    SOCKET serverSocket_;
    SOCKET clientSocket_;
    ChessPiece piece_;
};

int main() {
    Server server(3000);
    server.run();
    return 0;
}
