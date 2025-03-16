#define _CRT_SECURE_NO_WARNINGS 
//glew32.lib freeglut.lib
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")

#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <random>

using namespace std;

// 체스판 크기
const int BOARD_SIZE = 8;

// 체스 말의 위치 (초기 위치: 0,0)
class ChessPiece {
public:
    int x, y;

    ChessPiece() : x(0), y(0) {}  // 초기 위치 설정

    void moveUp() { if (y > 0) y--; }
    void moveDown() { if (y < BOARD_SIZE - 1) y++; }
    void moveLeft() { if (x > 0) x--; }
    void moveRight() { if (x < BOARD_SIZE - 1) x++; }
};

// 체스 말 객체 생성
ChessPiece piece;

// 윈도우 크기
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

// 체스판을 그리는 함수
void drawBoard() {
    float cellSize = 2.0f / BOARD_SIZE;  // -1 ~ 1 범위를 8x8로 분할

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            float x = -1.0f + j * cellSize;
            float y = 1.0f - i * cellSize;

            // 검정 & 흰색 타일 구분
            if ((i + j) % 2 == 0)
                glColor3f(1.0f, 1.0f, 1.0f); // 흰색
            else {
                //glColor3f(0.0f, 0.0f, 0.0f); // 검정색
                //glColor3f(0.6f, 0.3f, 0.0f); // 갈색
                glColor3f(0.7f, 0.4f, 0.2f); // 갈색
            }

            // 사각형(체스판 타일) 그리기
            glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + cellSize, y);
            glVertex2f(x + cellSize, y - cellSize);
            glVertex2f(x, y - cellSize);
            glEnd();
        }
    }
}

// 체스 말을 그리는 함수
void drawPiece() {
    float cellSize = 2.0f / BOARD_SIZE;
    float x = -1.0f + piece.x * cellSize + cellSize / 2;
    float y = 1.0f - piece.y * cellSize - cellSize / 2;

    glColor3f(1.0f, 0.0f, 0.0f); // 빨간색 체스 말
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 360; i += 30) {
        float angle = i * 3.14159f / 180.0f;
        glVertex2f(x + 0.05f * cos(angle), y + 0.05f * sin(angle));
    }
    glEnd();
}

// 화면을 그리는 콜백 함수
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawBoard(); // 체스판 그리기
    drawPiece(); // 체스 말 그리기

    glutSwapBuffers();
}

// 키 입력 콜백 함수
void keyboard(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        piece.moveUp();
        break;
    case GLUT_KEY_DOWN:
        piece.moveDown();
        break;
    case GLUT_KEY_LEFT:
        piece.moveLeft();
        break;
    case GLUT_KEY_RIGHT:
        piece.moveRight();
        break;
    }
    glutPostRedisplay(); // 화면 갱신
}

// 창 크기 조정 콜백
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

// 메인 함수
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("OpenGL Chess Board");

    glewExperimental = GL_TRUE;
    glewInit();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(keyboard); // 키 입력 처리
    glutMainLoop();

    return 0;
}
