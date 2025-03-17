#define STBI_MALLOC(sz) malloc(sz)
#define STBI_REALLOC(p,newsz) realloc(p,newsz)
#define STBI_FREE(p) free(p)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define _CRT_SECURE_NO_WARNINGS 
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")
#pragma comment(lib, "ws2_32.lib")

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>

// 체스판 및 체스말 관련 상수
const int BOARD_SIZE = 8;
const float BOARD_PIXEL = 454.0f;
const float TILE_SIZE = 2.0f * (BOARD_PIXEL / 486.0f) / BOARD_SIZE;
const float PIECE_SIZE = TILE_SIZE * 0.8f;

GLuint boardTexture, pieceTexture;

// 체스말 클래스 (초기 위치: (3,3))
class ChessPiece {
public:
    short x, y;
    ChessPiece() : x(3), y(3) {}
};

ChessPiece piece;
SOCKET clientSocket;

enum MovementCommand : short {
    CMD_UP = 1,
    CMD_DOWN = 2,
    CMD_LEFT = 3,
    CMD_RIGHT = 4
};

struct Coordinates {
    short x;
    short y;
};

struct AsyncIOContext {
    OVERLAPPED overlapped;
    WSABUF wsabuf;
    char buffer[1024];
    int operation; // 1: 전송, 2: 수신
};

void error_display(const char* msg, int err_no) {
    WCHAR* lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, err_no, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);
    std::cout << msg;
    std::wcout << L" 에러 " << lpMsgBuf << std::endl;
    while (true);
    LocalFree(lpMsgBuf);
}

// 클라이언트 비동기 완료 루틴
void CALLBACK IoCompletionRoutineClient(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD flags) {
    AsyncIOContext* context = reinterpret_cast<AsyncIOContext*>(lpOverlapped);
    if (error != 0 || bytesTransferred == 0) {
        std::cout << "Client I/O operation error or connection closed." << std::endl;
        return;
    }
    if (context->operation == 1) { // 전송 완료
        std::cout << "Send operation completed, sent bytes: " << bytesTransferred << std::endl;
        // 전송 후 새 좌표 수신 시작
        AsyncIOContext* recvContext = new AsyncIOContext;
        ZeroMemory(recvContext, sizeof(AsyncIOContext));
        recvContext->operation = 2; // 수신
        recvContext->wsabuf.buf = recvContext->buffer;
        recvContext->wsabuf.len = sizeof(Coordinates);
        DWORD flags = 0;
        DWORD recvBytes = 0;
        int ret = WSARecv(clientSocket, &recvContext->wsabuf, 1, &recvBytes, &flags, &recvContext->overlapped, IoCompletionRoutineClient);
        if (ret == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err != WSA_IO_PENDING) {
                std::cout << "WSARecv failed on client with error: " << err << std::endl;
                delete recvContext;
            }
        }
        delete context;
    }
    else if (context->operation == 2) { // 수신 완료
        if (bytesTransferred < sizeof(Coordinates)) {
            std::cout << "Received incomplete coordinates." << std::endl;
            delete context;
            return;
        }
        Coordinates coords;
        memcpy(&coords, context->buffer, sizeof(Coordinates));
        piece.x = coords.x;
        piece.y = coords.y;
        std::cout << "Updated piece position: (" << piece.x << ", " << piece.y << ")" << std::endl;
        glutPostRedisplay();
        delete context;
    }
}

// 이동 명령을 전송하는 함수 (비동기)
void sendMovementCommand(short command) {
    AsyncIOContext* sendContext = new AsyncIOContext;
    ZeroMemory(sendContext, sizeof(AsyncIOContext));
    sendContext->operation = 1; // 전송
    sendContext->wsabuf.buf = sendContext->buffer;
    sendContext->wsabuf.len = sizeof(short);
    memcpy(sendContext->buffer, &command, sizeof(short));
    DWORD sentBytes = 0;
    int ret = WSASend(clientSocket, &sendContext->wsabuf, 1, &sentBytes, 0, &sendContext->overlapped, IoCompletionRoutineClient);
    if (ret == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            std::cout << "WSASend failed on client with error: " << err << std::endl;
            delete sendContext;
            return;
        }
    }
}

// GLUT 키 이벤트: 해당 방향의 명령 전송
void keyboard(int key, int x, int y) {
    short command = 0;
    switch (key) {
    case GLUT_KEY_UP:    command = CMD_UP; break;
    case GLUT_KEY_DOWN:  command = CMD_DOWN; break;
    case GLUT_KEY_LEFT:  command = CMD_LEFT; break;
    case GLUT_KEY_RIGHT: command = CMD_RIGHT; break;
    default: return;
    }
    sendMovementCommand(command);
}

// 이 함수는 I/O 완료 처리를 위한 전용 워커 스레드입니다.
// 메인 스레드(GLUT)는 SleepEx를 호출하지 않고 일반적으로 실행됩니다.
DWORD WINAPI IoCompletionThread(LPVOID lpParam) {
    // 이 스레드는 alertable 상태로 남아 APC(비동기 완료 루틴)를 처리합니다.
    while (true) {
        SleepEx(100, TRUE);  // 이 스레드에서만 SleepEx를 호출합니다.
    }
    return 0;
}

GLuint loadTexture(const char* filename) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        std::cout << "Failed to load image: " << filename << std::endl;
        return 0;
    }
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
    return textureID;
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    boardTexture = loadTexture("assets/board.png");
    pieceTexture = loadTexture("assets/black_king.png");
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, boardTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -0.5f);
    glEnd();

    float offset = TILE_SIZE / 2;
    float piece_offset = (TILE_SIZE - PIECE_SIZE);
    float piecePosX = -1.0f + piece.x * TILE_SIZE + offset + piece_offset;
    float piecePosY = 1.0f - piece.y * TILE_SIZE - offset - piece_offset;

    glBindTexture(GL_TEXTURE_2D, pieceTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(piecePosX - PIECE_SIZE / 2, piecePosY + PIECE_SIZE / 2, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(piecePosX + PIECE_SIZE / 2, piecePosY + PIECE_SIZE / 2, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(piecePosX + PIECE_SIZE / 2, piecePosY - PIECE_SIZE / 2, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(piecePosX - PIECE_SIZE / 2, piecePosY - PIECE_SIZE / 2, 0.0f);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

void initNetwork() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup failed" << std::endl;
        exit(1);
    }
    std::string serverIP;
    std::cout << "Enter server IP address: ";
    std::cin >> serverIP;
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cout << "Error creating socket" << std::endl;
        exit(1);
    }
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3000);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Connection failed" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        exit(1);
    }
    std::cout << "Connected to server" << std::endl;
}

int main(int argc, char** argv) {
    initNetwork();

    // 별도의 I/O 완료 전담 스레드 생성 (이 스레드가 alertable 상태에서 SleepEx 호출)
    HANDLE hThread = CreateThread(NULL, 0, IoCompletionThread, NULL, 0, NULL);
    if (!hThread) {
        std::cout << "Failed to create I/O completion thread." << std::endl;
        exit(1);
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Chess Board with Texture");

    glewInit();
    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(keyboard);
    glutMainLoop();

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
