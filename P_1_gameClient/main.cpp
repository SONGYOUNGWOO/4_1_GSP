#define STBI_MALLOC(sz) malloc(sz)
#define STBI_REALLOC(p,newsz) realloc(p,newsz)
#define STBI_FREE(p) free(p)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define _CRT_SECURE_NO_WARNINGS 
// glew32.lib freeglut.lib
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

const int BOARD_SIZE = 8; // 체스판 크기 (8x8)
const float BOARD_PIXEL = 454.0f; // 체스말이 이동할 수 있는 보드 크기 (픽셀 기준)
const float TILE_SIZE = 2.0f * (BOARD_PIXEL / 486.0f) / BOARD_SIZE; // 체스말이 이동할 정확한 한 칸 크기
const float PIECE_SIZE = TILE_SIZE * 0.8f;

GLuint boardTexture, pieceTexture; // 체스판 & 체스말 텍스처 ID

class ChessPiece {
public:
    int x, y;

    ChessPiece() : x(3), y(3) {}

    void moveUp() { if (y > 0) y--; }
    void moveDown() { if (y < BOARD_SIZE - 1) y++; }
    void moveLeft() { if (x > 0) x--; }
    void moveRight() { if (x < BOARD_SIZE - 1) x++; }
};

ChessPiece piece; // 체스말 


GLuint loadTexture(const char* filename) {
    int width, height, channels;

    stbi_set_flip_vertically_on_load(1); // ✅ Y축 Flip(반전) 적용

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



void keyboard(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:    piece.moveUp();    break;
    case GLUT_KEY_DOWN:  piece.moveDown();  break;
    case GLUT_KEY_LEFT:  piece.moveLeft();  break;
    case GLUT_KEY_RIGHT: piece.moveRight(); break;
    }
    glutPostRedisplay(); 
}


void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}


int main(int argc, char** argv) {
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

    return 0;
}
