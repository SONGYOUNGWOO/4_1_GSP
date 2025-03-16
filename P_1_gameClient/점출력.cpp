#define _CRT_SECURE_NO_WARNINGS 
//glew32.lib freeglut.lib
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")

#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <random>

using namespace std;

// ü���� ũ��
const int BOARD_SIZE = 8;

// ü�� ���� ��ġ (�ʱ� ��ġ: 0,0)
class ChessPiece {
public:
    int x, y;

    ChessPiece() : x(0), y(0) {}  // �ʱ� ��ġ ����

    void moveUp() { if (y > 0) y--; }
    void moveDown() { if (y < BOARD_SIZE - 1) y++; }
    void moveLeft() { if (x > 0) x--; }
    void moveRight() { if (x < BOARD_SIZE - 1) x++; }
};

// ü�� �� ��ü ����
ChessPiece piece;

// ������ ũ��
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

// ü������ �׸��� �Լ�
void drawBoard() {
    float cellSize = 2.0f / BOARD_SIZE;  // -1 ~ 1 ������ 8x8�� ����

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            float x = -1.0f + j * cellSize;
            float y = 1.0f - i * cellSize;

            // ���� & ��� Ÿ�� ����
            if ((i + j) % 2 == 0)
                glColor3f(1.0f, 1.0f, 1.0f); // ���
            else {
                //glColor3f(0.0f, 0.0f, 0.0f); // ������
                //glColor3f(0.6f, 0.3f, 0.0f); // ����
                glColor3f(0.7f, 0.4f, 0.2f); // ����
            }

            // �簢��(ü���� Ÿ��) �׸���
            glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + cellSize, y);
            glVertex2f(x + cellSize, y - cellSize);
            glVertex2f(x, y - cellSize);
            glEnd();
        }
    }
}

// ü�� ���� �׸��� �Լ�
void drawPiece() {
    float cellSize = 2.0f / BOARD_SIZE;
    float x = -1.0f + piece.x * cellSize + cellSize / 2;
    float y = 1.0f - piece.y * cellSize - cellSize / 2;

    glColor3f(1.0f, 0.0f, 0.0f); // ������ ü�� ��
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 360; i += 30) {
        float angle = i * 3.14159f / 180.0f;
        glVertex2f(x + 0.05f * cos(angle), y + 0.05f * sin(angle));
    }
    glEnd();
}

// ȭ���� �׸��� �ݹ� �Լ�
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawBoard(); // ü���� �׸���
    drawPiece(); // ü�� �� �׸���

    glutSwapBuffers();
}

// Ű �Է� �ݹ� �Լ�
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
    glutPostRedisplay(); // ȭ�� ����
}

// â ũ�� ���� �ݹ�
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

// ���� �Լ�
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("OpenGL Chess Board");

    glewExperimental = GL_TRUE;
    glewInit();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(keyboard); // Ű �Է� ó��
    glutMainLoop();

    return 0;
}
