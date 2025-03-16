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

const float PIECE_SIZE = 0.2f; // 체스말 크기 (화면 크기 대비 조정)

GLuint pieceTexture; // 체스말 텍스처 ID

// 체스말 위치
class ChessPiece {
public:
    float x, y;
    ChessPiece() : x(0.0f), y(0.0f) {} // 중앙 
};

ChessPiece piece; // 체스말 객체

// ✅ 텍스처 로드 함수
GLuint loadTexture(const char* filename) {
    int width, height, channels;

    stbi_set_flip_vertically_on_load(1); // ✅ Y축 Flip(반전) 적용 !!

    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        std::cout << "Failed to load image: " << filename << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB; // ✅ PNG 파일의 RGBA 지원 추가
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
}


// ✅ 체스말 텍스처 로드
void init() {
    glEnable(GL_DEPTH_TEST); // ✅ 깊이 버퍼 활성화
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // ✅ 배경색 
    pieceTexture = loadTexture("assets/black_king.png");  
}

// ✅ 화면 그리기 (체스말만 출력)
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND); // ✅ PNG 알파 채널 적용
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, pieceTexture);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(piece.x - PIECE_SIZE / 2, piece.y + PIECE_SIZE / 2, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(piece.x + PIECE_SIZE / 2, piece.y + PIECE_SIZE / 2, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(piece.x + PIECE_SIZE / 2, piece.y - PIECE_SIZE / 2, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(piece.x - PIECE_SIZE / 2, piece.y - PIECE_SIZE / 2, 0.0f);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glutSwapBuffers();
}


// ✅ 창 크기 
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

// ✅ 메인 함수
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); 
    glutInitWindowSize(800, 800);
    glutCreateWindow("Chess Piece Only");

    glewInit();
    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMainLoop();

    return 0;
}
