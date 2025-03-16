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

// ���� ip : 125.190.104.16

// ü���� �� ü���� ���� ���
const int BOARD_SIZE = 8;
const float BOARD_PIXEL = 454.0f;
const float TILE_SIZE = 2.0f * (BOARD_PIXEL / 486.0f) / BOARD_SIZE;
const float PIECE_SIZE = TILE_SIZE * 0.8f;

GLuint boardTexture, pieceTexture;

// ü���� Ŭ���� (�ʱ� ��ġ: (3,3))
class ChessPiece {
public:
    short x, y;
    ChessPiece() : x(3), y(3) {}
};

ChessPiece piece;
SOCKET clientSocket;

// �̵� ��� ������ (������ ����)
enum MovementCommand : short {
    CMD_UP = 1,
    CMD_DOWN = 2,
    CMD_LEFT = 3,
    CMD_RIGHT = 4
};

// ������ ������ ��ǥ ������ ����ü
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

    // ü���� �׸���
    glBindTexture(GL_TEXTURE_2D, boardTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, 1.0f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -0.5f);
    glEnd();

    // ü������ ��ġ ��� �� �׸���
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

// ��Ʈ��ũ �ʱ�ȭ �� ���� ���� (���α׷� ���� �� ���� IP �Է�)
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

// Ű���� �̺�Ʈ ó��: �ش� ������ short ��� ���� ��, �����κ��� �� ��ǥ ����
void keyboard(int key, int x, int y) {
    short command = 0;
    switch (key) {
    case GLUT_KEY_UP:    command = CMD_UP;    break;
    case GLUT_KEY_DOWN:  command = CMD_DOWN;  break;
    case GLUT_KEY_LEFT:  command = CMD_LEFT;  break;
    case GLUT_KEY_RIGHT: command = CMD_RIGHT; break;
    default: return;
    }

    // �̵� ��� ���� (short 2����Ʈ)
    int sent_bytes = send(clientSocket, reinterpret_cast<char*>(&command), sizeof(command), 0);
    if (sent_bytes != sizeof(command)) {
        std::cout << "Send error" << std::endl;
        return;
    }

    // �����κ��� �� ��ǥ ����
    Coordinates coords;
    int recv_bytes = recv(clientSocket, reinterpret_cast<char*>(&coords), sizeof(coords), 0);
    if (recv_bytes != sizeof(coords)) {
        std::cout << "Receive error or connection closed" << std::endl;
        return;
    }

    // ���ŵ� ��ǥ�� ü���� ��ġ ����
    piece.x = coords.x;
    piece.y = coords.y;
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    initNetwork(); // ��Ʈ��ũ ����

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
