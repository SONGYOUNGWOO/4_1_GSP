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
#include <iostream>
#include <string>

// 서버 ip : 125.190.104.16

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

// 이동 명령 열거형 (서버와 동일)
enum MovementCommand : short {
    CMD_UP = 1,
    CMD_DOWN = 2,
    CMD_LEFT = 3,
    CMD_RIGHT = 4
};

// 서버가 보내는 좌표 데이터 구조체
struct Coordinates {
    short x;
    short y;
};

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

    // 체스판 그리기
    glBindTexture(GL_TEXTURE_2D, boardTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -0.5f);
    glEnd();

    // 체스말의 위치 계산 및 그리기
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

// 네트워크 초기화 및 서버 연결 (프로그램 시작 시 서버 IP 입력)
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

// 키보드 이벤트 처리: 해당 방향의 short 명령 전송 후, 서버로부터 새 좌표 수신
void keyboard(int key, int x, int y) {
    short command = 0;
    switch (key) {
    case GLUT_KEY_UP:    command = CMD_UP;    break;
    case GLUT_KEY_DOWN:  command = CMD_DOWN;  break;
    case GLUT_KEY_LEFT:  command = CMD_LEFT;  break;
    case GLUT_KEY_RIGHT: command = CMD_RIGHT; break;
    default: return;
    }

    // 이동 명령 전송 (short 2바이트)
    int sent_bytes = send(clientSocket, reinterpret_cast<char*>(&command), sizeof(command), 0);
    if (sent_bytes != sizeof(command)) {
        std::cout << "Send error" << std::endl;
        return;
    }

    // 서버로부터 새 좌표 수신
    Coordinates coords;
    int recv_bytes = recv(clientSocket, reinterpret_cast<char*>(&coords), sizeof(coords), 0);
    if (recv_bytes != sizeof(coords)) {
        std::cout << "Receive error or connection closed" << std::endl;
        return;
    }

    // 수신된 좌표로 체스말 위치 갱신
    piece.x = coords.x;
    piece.y = coords.y;
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    initNetwork(); // 네트워크 연결

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
