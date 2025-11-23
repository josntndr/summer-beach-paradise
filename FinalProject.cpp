#ifdef _WIN32
#include <GL/freeglut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

// Window Dimensions
const int WIN_W = 1000;
const int WIN_H = 750;
constexpr float PI = 3.14159265358979323846f;

// --- Global State ---
static bool showCredits = true;
static bool isNightMode = false;
static float globalZoom = 1.0f;

// Umbrella global position
static float umbX_global = 700.0f;
static float umbY_global = 0.0f;

// --- NIGHT MODE ELEMENTS ---
struct Point { float x, y; };
std::vector<Point> stars;

// Shooting Star State
static float sStarX = -100.0f, sStarY = -100.0f;
static bool sStarActive = false;
static float sStarNextSpawnTime = 0.0f;

// --- VERTEX ARRAY STORAGE ---
std::vector<float> treeTrunkVertices;
std::vector<float> treeTrunkColors;
std::vector<float> treeTrunkNormals;

// Get time in seconds
float secs() {
    return glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

// ----------------- Initialization -----------------
void initPalmTreeGeometry() {
    float trunkW = 60.0f;
    float trunkH = 250.0f;

    for (int i = 0; i < 10; ++i) {
        float y0 = trunkH * i / 10.0f;
        float y1 = trunkH * (i + 1) / 10.0f;
        float w0 = trunkW * (1.0f - i * 0.05f);
        float w1 = trunkW * (1.0f - (i + 1) * 0.05f);
        float offsetX0 = powf((float)i / 10.0f, 2.0f) * 18.0f;
        float offsetX1 = powf((float)(i + 1) / 10.0f, 2.0f) * 18.0f;

        float p1x = -w0 * 0.5f + offsetX0, p1y = y0;
        float p2x = w0 * 0.5f + offsetX0, p2y = y0;
        float p3x = w1 * 0.5f + offsetX1, p3y = y1;
        float p4x = -w1 * 0.5f + offsetX1, p4y = y1;

        float r = 0.54f, g = 0.32f, b = 0.12f;

        treeTrunkVertices.insert(treeTrunkVertices.end(), { p1x, p1y, 0, p2x, p2y, 0, p4x, p4y, 0 });
        for (int k = 0; k < 3; k++) treeTrunkColors.insert(treeTrunkColors.end(), { r, g, b });
        for (int k = 0; k < 3; k++) treeTrunkNormals.insert(treeTrunkNormals.end(), { 0, 0, 1 });

        treeTrunkVertices.insert(treeTrunkVertices.end(), { p2x, p2y, 0, p3x, p3y, 0, p4x, p4y, 0 });
        for (int k = 0; k < 3; k++) treeTrunkColors.insert(treeTrunkColors.end(), { r, g, b });
        for (int k = 0; k < 3; k++) treeTrunkNormals.insert(treeTrunkNormals.end(), { 0, 0, 1 });
    }
}

void initStars() {
    srand(time(NULL));
    for (int i = 0; i < 200; ++i) {
        stars.push_back({ (float)(rand() % WIN_W), (float)(rand() % (int)(WIN_H * 0.6f)) + WIN_H * 0.4f });
    }
}

// ----------------- Text Helpers -----------------
void drawCenteredText(float x, float y, void* font, const std::string& text) {
    glDisable(GL_LIGHTING);
    int textWidth = 0;
    for (char c : text) textWidth += glutBitmapWidth(font, c);
    glRasterPos2f(x - (textWidth / 2.0f), y);
    for (char c : text) glutBitmapCharacter(font, c);
    if (isNightMode) glEnable(GL_LIGHTING);
}

// ----------------- Basic Shapes -----------------
void myFilledCircle(float cx, float cy, float r, int n = 48) {
    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= n; ++i) {
        float a = (float)i / n * 2.0f * PI;
        glVertex2f(cx + cosf(a) * r, cy + sinf(a) * r);
    }
    glEnd();
}

void myFilledEllipse(float cx, float cy, float rx, float ry, int n = 48) {
    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= n; ++i) {
        float a = (float)i / n * 2.0f * PI;
        glVertex2f(cx + cosf(a) * rx, cy + sinf(a) * ry);
    }
    glEnd();
}

void myFilledRect(float x, float y, float w, float h) {
    glNormal3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// ----------------- Credits Screen -----------------
void drawCredits() {
    glDisable(GL_LIGHTING);

    glBegin(GL_QUADS);
    glColor3f(0.2f, 0.1f, 0.4f); glVertex2f(0, WIN_H);
    glColor3f(0.2f, 0.1f, 0.4f); glVertex2f(WIN_W, WIN_H);
    glColor3f(0.9f, 0.5f, 0.3f); glVertex2f(WIN_W, 0);
    glColor3f(0.9f, 0.5f, 0.3f); glVertex2f(0, 0);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, 0.15f);

    float panelW = 600, panelH = 500;
    float panelX = (WIN_W - panelW) / 2;
    float panelY = (WIN_H - panelH) / 2;

    myFilledRect(panelX, panelY, panelW, panelH);

    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(panelX, panelY);
    glVertex2f(panelX + panelW, panelY);
    glVertex2f(panelX + panelW, panelY + panelH);
    glVertex2f(panelX, panelY + panelH);
    glEnd();

    glColor4f(1.0f, 0.9f, 0.4f, 0.9f);
    myFilledCircle(WIN_W / 2, panelY + panelH - 60, 40);
    glColor4f(1.0f, 1.0f, 1.0f, 0.2f);
    myFilledCircle(WIN_W / 2, panelY + panelH - 60, 50);

    glColor3f(1.0f, 1.0f, 1.0f);
    float startY = panelY + panelH - 140;

    glColor3f(1.0f, 0.95f, 0.8f);
    drawCenteredText(WIN_W / 2, startY, GLUT_BITMAP_TIMES_ROMAN_24, "SUMMER BEACH PARADISE");

    glColor3f(1.0f, 1.0f, 1.0f);
    drawCenteredText(WIN_W / 2, startY - 40, GLUT_BITMAP_HELVETICA_18, "Summer Vacation Simulation");

    glBegin(GL_LINES);
    glVertex2f(WIN_W / 2 - 100, startY - 60);
    glVertex2f(WIN_W / 2 + 100, startY - 60);
    glEnd();

    drawCenteredText(WIN_W / 2, startY - 100, GLUT_BITMAP_HELVETICA_18, "CREATED BY:");

    int nameSpacing = 30;
    float namesY = startY - 140;
    drawCenteredText(WIN_W / 2, namesY, GLUT_BITMAP_HELVETICA_18, "Will Gabriel C. Padilla - 202310870");
    drawCenteredText(WIN_W / 2, namesY - nameSpacing, GLUT_BITMAP_HELVETICA_18, "Josephine J. Santander - 202310348");
    drawCenteredText(WIN_W / 2, namesY - (nameSpacing * 2), GLUT_BITMAP_HELVETICA_18, "Paul Lewis J. Villamil - 202310868");

    glColor3f(0.8f, 1.0f, 0.8f);
    drawCenteredText(WIN_W / 2, panelY + 70, GLUT_BITMAP_HELVETICA_12, "Mouse Drag: Move Umbrella | 'N': Night Mode | +/-: Zoom");

    float blink = fabs(sin(secs() * 3.0f));
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f + (blink * 0.5f));
    drawCenteredText(WIN_W / 2, panelY + 30, GLUT_BITMAP_9_BY_15, "- PRESS ANY KEY TO START -");

    glDisable(GL_BLEND);
    glutSwapBuffers();
}

// ----------------- Scene Elements -----------------

// Draw Stars
void drawStars() {
    if (!isNightMode) return;
    glDisable(GL_LIGHTING);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 1.0f, 1.0f);
    for (const auto& p : stars) {
        float twinkle = 0.7f + 0.3f * sinf(secs() * 2.0f + p.x * 0.1f);
        glColor4f(1.0f, 1.0f, 1.0f, twinkle);
        glVertex2f(p.x, p.y);
    }
    glEnd();
    if (isNightMode) glEnable(GL_LIGHTING);
}

// Shooting Star Logic
void drawShootingStar(float t) {
    if (!isNightMode) { sStarActive = false; return; }

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);

    if (!sStarActive) {
        if (t > sStarNextSpawnTime) {
            sStarActive = true;
            sStarX = (float)(rand() % (WIN_W / 2) - 100);
            sStarY = (float)(WIN_H + (rand() % 200));
        }
    }
    else {
        sStarX += 3.5f;
        sStarY -= 2.0f;

        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
        glVertex2f(sStarX - 100, sStarY + 57.0f);
        glColor4f(0.8f, 0.9f, 1.0f, 1.0f);
        glVertex2f(sStarX, sStarY);
        glEnd();

        glPointSize(5.0f);
        glBegin(GL_POINTS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(sStarX, sStarY);
        glEnd();

        if (sStarX > WIN_W + 200 || sStarY < -200) {
            sStarActive = false;
            sStarNextSpawnTime = t + 3.0f + (rand() % 50) / 10.0f;
        }
    }
    glDisable(GL_BLEND);
    if (isNightMode) glEnable(GL_LIGHTING);
}

void drawCelestialBody(float cx, float cy, float coreR, float t) {
    if (isNightMode) {
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glColor4f(0.8f, 0.8f, 1.0f, 0.3f);
        myFilledCircle(cx, cy, coreR * 1.15f, 64);
        glDisable(GL_BLEND);

        glColor3f(0.92f, 0.94f, 1.0f);
        myFilledCircle(cx, cy, coreR, 64);

        glColor3f(0.85f, 0.87f, 0.95f);
        myFilledCircle(cx - 15, cy + 10, 12, 32);
        myFilledCircle(cx + 20, cy - 5, 8, 32);
        myFilledCircle(cx - 5, cy - 18, 10, 32);

        glEnable(GL_LIGHTING);
        GLfloat lightPos[] = { cx, cy, 100.0f, 1.0f };
        GLfloat lightColor[] = { 0.4f, 0.4f, 0.6f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    }
    else {
        glDisable(GL_LIGHTING);

        float pulse = 0.98f + 0.04f * (0.5f + 0.5f * sinf(t * 0.8f));
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        for (int i = 5; i >= 1; --i) {
            float R = coreR + i * coreR * 1.0f * pulse;
            float a = 0.04f * (6 - i);
            glColor4f(1.0f, 0.95f, 0.5f, a);
            myFilledCircle(cx, cy, R, 64);
        }
        glDisable(GL_BLEND);
        glColor3f(1.0f, 0.94f, 0.20f);
        myFilledCircle(cx, cy, coreR, 64);
    }
}

void drawCloud(float x, float y, float scale, float t) {
    float drift = x + t * 8.0f;
    float bob = sinf(t * 0.6f + x * 0.01f) * 5.0f;
    float Y = y + bob;

    if (isNightMode) glColor3f(0.4f, 0.4f, 0.5f);
    else glColor3f(1.0f, 1.0f, 1.0f);

    myFilledCircle(drift, Y, 45.0f * scale, 50);
    myFilledCircle(drift - 55.0f * scale, Y - 10.0f * scale, 40.0f * scale, 45);
    myFilledCircle(drift + 40.0f * scale, Y - 15.0f * scale, 35.0f * scale, 45);
    myFilledCircle(drift - 35.0f * scale, Y - 25.0f * scale, 25.0f * scale, 36);
    myFilledCircle(drift + 25.0f * scale, Y - 28.0f * scale, 28.0f * scale, 36);

    if (isNightMode) glColor4f(0.4f, 0.4f, 0.5f, 0.6f);
    else glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
    myFilledRect(drift - 60.0f * scale, Y - 35.0f * scale, 120.0f * scale, 20.0f * scale);
}

void drawWaveLines(float left, float right, float baseY, float t) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(3.0f);
    glNormal3f(0, 0, 1);
    for (int i = 0; i < 3; ++i) {
        float offset = baseY + i * 25.0f + 10.0f;
        float amp = 4.0f - i * 1.0f;
        float speed = 0.15f + i * 0.03f;
        glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
        glBegin(GL_LINE_STRIP);
        for (float x = left; x <= right; x += 8.0f) {
            float nx = x / 90.0f;
            float y = offset + sinf(nx + t * speed + i * 0.5f) * amp;
            glVertex2f(x, y);
        }
        glEnd();
    }
    glDisable(GL_BLEND);
}

void drawOceanBase(float left, float right, float top, float bottom, float t) {
    if (isNightMode) glColor3f(0.01f, 0.15f, 0.25f);
    else glColor3f(0.02f, 0.62f, 0.78f);

    myFilledRect(left, bottom, right - left, top - bottom);
    drawWaveLines(left, right, (top + bottom) * 0.5f, t);
}

void drawSailBoat(float cx, float cy, float t) {
    float boatBob = sinf(t * 1.2f) * 4.0f;
    float boatTilt = sinf(t * 1.0f) * 2.0f;
    float boatSpeed = 70.0f;

    float startX = -200.0f;
    float endX = WIN_W + 200.0f;
    float totalDistance = endX - startX;
    float boatX = startX + fmod(t * boatSpeed, totalDistance);

    glPushMatrix();
    glTranslatef(boatX, cy + boatBob, 0.0f);
    glRotatef(boatTilt, 0.0f, 0.0f, 1.0f);

    float hullW = 80.0f;
    float hullH = 30.0f;

    glNormal3f(0, 0, 1);
    glColor3f(0.45f, 0.25f, 0.05f);
    glBegin(GL_POLYGON);
    for (int i = 0; i <= 40; ++i) {
        float a = PI + (float)i / 40.0f * PI;
        glVertex2f(cosf(a) * hullW, sinf(a) * hullH);
    }
    glEnd();

    glColor3f(0.55f, 0.32f, 0.10f);
    myFilledRect(-hullW, 0.0f, hullW * 2.0f, 10.0f);

    float mastH = 130.0f;
    glColor3f(0.35f, 0.20f, 0.05f);
    myFilledRect(-3.0f, 0.0f, 6.0f, mastH);

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(3.0f, 5.0f);
    glVertex2f(3.0f, mastH - 10.0f);
    glVertex2f(65.0f, 15.0f);
    glEnd();

    glColor3f(0.95f, 0.2f, 0.2f);
    glBegin(GL_TRIANGLES);
    glVertex2f(5.0f, 20.0f);
    glVertex2f(5.0f, 55.0f);
    glVertex2f(40.0f, 25.0f);
    glEnd();

    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(0.0f, mastH);
    glVertex2f(18.0f, mastH - 4.0f);
    glVertex2f(0.0f, mastH - 8.0f);
    glEnd();
    glPopMatrix();
}

void drawPalmTree(float baseX, float baseY, float t, bool smallTree = false) {
    glPushMatrix();
    glTranslatef(baseX, baseY, 0.0f);

    float trunkH = smallTree ? 180.0f : 250.0f;

    if (!smallTree) {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, treeTrunkVertices.data());
        glColorPointer(3, GL_FLOAT, 0, treeTrunkColors.data());
        glNormalPointer(GL_FLOAT, 0, treeTrunkNormals.data());

        glDrawArrays(GL_TRIANGLES, 0, treeTrunkVertices.size() / 3);

        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    else {
        float trunkW = 40.0f;
        glColor3f(0.54f, 0.32f, 0.12f);
        glNormal3f(0, 0, 1);
        glBegin(GL_QUADS);
        for (int i = 0; i < 10; ++i) {
            float y0 = trunkH * i / 10.0f;
            float y1 = trunkH * (i + 1) / 10.0f;
            float w0 = trunkW * (1.0f - i * 0.05f);
            float w1 = trunkW * (1.0f - (i + 1) * 0.05f);
            float offsetX0 = powf((float)i / 10.0f, 2.0f) * 18.0f;
            float offsetX1 = powf((float)(i + 1) / 10.0f, 2.0f) * 18.0f;
            glVertex2f(-w0 * 0.5f + offsetX0, y0);
            glVertex2f(w0 * 0.5f + offsetX0, y0);
            glVertex2f(w1 * 0.5f + offsetX1, y1);
            glVertex2f(-w1 * 0.5f + offsetX1, y1);
        }
        glEnd();
    }

    float topY = trunkH;
    float topOffsetX = powf(1.0f, 2.0f) * 18.0f;
    float leafLength = smallTree ? 100.0f : 150.0f;
    float swayAmplitude = smallTree ? 5.0f : 6.0f;
    glColor3f(0.08f, 0.55f, 0.18f);

    glNormal3f(0, 0, 1);
    for (int f = 0; f < 6; ++f) {
        glPushMatrix();
        float angle = 360.0f / 6.0f * f;
        glTranslatef(topOffsetX, topY, 0.0f);
        glRotatef(angle, 0.0f, 0.0f, 1.0f);
        float sway = sinf(t * 1.8f + f) * swayAmplitude;
        int leafSegments = 18;
        for (int s = 0; s < leafSegments; ++s) {
            float u = (float)s / (leafSegments - 1);
            float stemX = leafLength * u;
            float stemY = sinf(u * PI) * 10.0f;
            int leaflets = 6;
            for (int l = 0; l < leaflets; ++l) {
                float leafletY = (l - leaflets / 2) * 6.0f * (1.0f - u);
                float leafletLength = 24.0f * (1.0f - u) + 10.0f;
                float leafletHeight = 8.0f;
                glBegin(GL_QUADS);
                glVertex2f(stemX, stemY + leafletY + sway * u);
                glVertex2f(stemX + leafletLength, stemY + leafletY + sway * u);
                glVertex2f(stemX + leafletLength, stemY + leafletY + leafletHeight + sway * u);
                glVertex2f(stemX, stemY + leafletY + leafletHeight + sway * u);
                glEnd();
            }
        }
        glPopMatrix();
    }

    glColor3f(0.2f, 0.12f, 0.02f);
    myFilledCircle(topOffsetX + 10.0f, topY - 8.0f, 9.0f, 24);
    myFilledCircle(topOffsetX - 10.0f, topY - 14.0f, 8.0f, 24);
    myFilledCircle(topOffsetX + 26.0f, topY - 18.0f, 7.0f, 24);

    glPopMatrix();
}

void drawUmbrella(float cx, float cy, float r, float leanAngle = -12.0f) {
    r *= 1.25f;
    float poleHeight = r * 2.2f;
    float poleWidth = r * 0.12f;
    float canopyRadius = r * 1.3f;
    float canopyHeight = r * 0.9f;
    float canopyBottomY_rel = poleHeight;
    float canopyApexY_rel = poleHeight + canopyHeight;
    float knobRadius = r * 0.08f;
    int numPanels = 8;

    glPushMatrix();
    glTranslatef(cx, cy - poleHeight, 0);
    glRotatef(leanAngle, 0, 0, 1);

    glColor3f(0.45f, 0.28f, 0.05f);
    myFilledRect(-poleWidth * 0.5f, 0, poleWidth, poleHeight);

    float step = 2.0f * PI / numPanels;
    glNormal3f(0, 0, 1);
    for (int i = 0; i < numPanels; ++i) {
        if (i % 2 == 0) glColor3f(1.0f, 0.25f, 0.25f);
        else glColor3f(1.0f, 0.9f, 0.2f);
        float angle1 = i * step;
        float angle2 = (i + 1) * step;
        glBegin(GL_TRIANGLES);
        glVertex2f(0, canopyApexY_rel);
        glVertex2f(cosf(angle1) * canopyRadius, canopyBottomY_rel);
        glVertex2f(cosf(angle2) * canopyRadius, canopyBottomY_rel);
        glEnd();
    }

    glBegin(GL_POLYGON);
    glColor3f(1.0f, 0.3f, 0.3f);
    int segs = 60;
    for (int i = 0; i <= segs; ++i) {
        float a = PI + (float)i / segs * PI;
        float x = cosf(a) * canopyRadius;
        float y = canopyBottomY_rel;
        glVertex2f(x, y);
    }
    glEnd();

    glColor3f(1.0f, 0.9f, 0.2f);
    myFilledCircle(0, canopyApexY_rel + knobRadius * 0.6f, knobRadius, 24);

    glPopMatrix();
}

void drawSand(float topY) {
    if (isNightMode) glColor3f(0.6f, 0.55f, 0.4f);
    else glColor3f(0.96f, 0.88f, 0.63f);
    myFilledRect(0.0f, 0.0f, WIN_W, topY);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    glColor4f(0.75f, 0.64f, 0.42f, 0.3f);
    static bool seeded = false;
    if (!seeded) { srand(12345); seeded = true; }
    for (int i = 0; i < 1500; ++i) {
        float rx = (float)(rand() % WIN_W);
        float ry = (float)(rand() % (int)(topY - 10));
        glVertex2f(rx, ry);
    }
    glEnd();
    glDisable(GL_BLEND);
}

void drawVolleyball(float x, float y, float radius) {
    glColor3f(1.0f, 1.0f, 1.0f);
    myFilledCircle(x, y, radius);

    glColor3f(0.3f, 0.3f, 0.3f);
    glLineWidth(2.0f);
    glPushMatrix();
    glTranslatef(x, y, 0);

    glNormal3f(0, 0, 1);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 16; ++i) {
        float a = -PI / 2 + (float)i / 16 * PI;
        glVertex2f(cosf(a) * radius * 0.8f, sinf(a) * radius);
    }
    glEnd();

    for (int j = 0; j < 2; ++j) {
        glPushMatrix();
        glRotatef(120.0f * (j + 1), 0, 0, 1);
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= 16; ++i) {
            float a = -PI / 2 + (float)i / 16 * PI;
            glVertex2f(cosf(a) * radius * 0.8f, sinf(a) * radius);
        }
        glEnd();
        glPopMatrix();
    }
    glPopMatrix();
}

void drawGirlHair() {
    glColor3f(0.3f, 0.2f, 0.1f);
    myFilledEllipse(0, 10, 12, 8);
    myFilledEllipse(-6, 8, 3, 6);
    myFilledEllipse(6, 8, 3, 6);

    glNormal3f(0, 0, 1);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(8, 5);
    glVertex2f(14, 8);
    glVertex2f(16, 12);
    glVertex2f(14, 16);
    glVertex2f(10, 18);
    glVertex2f(8, 14);
    glVertex2f(6, 10);
    glEnd();
}

void drawBoyHair() {
    glColor3f(0.2f, 0.15f, 0.1f);
    myFilledEllipse(0, 10, 12, 7);
    myFilledEllipse(-5, 8, 3, 5);
    myFilledEllipse(5, 8, 3, 5);
    myFilledEllipse(0, 6, 10, 3);
}

void drawGirlPlayer(float x, float y, float ballX, float ballY, float prevAngleR, float prevAngleL, float breatheScale) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(1.0f + (1.0f - breatheScale) * 0.05f, breatheScale, 1.0f);

    glColor3f(0.95f, 0.8f, 0.7f);
    myFilledRect(-8, -40, 6, 40);
    myFilledRect(2, -40, 6, 40);

    glColor3f(0.9f, 0.2f, 0.4f);
    myFilledRect(-12, 0, 24, 40);

    float angleRight = atan2(ballY - (y + 30), ballX - (x + 14)) * 180 / PI;
    float angleLeft = atan2(ballY - (y + 30), ballX - (x - 14)) * 180 / PI;
    float armR = lerp(prevAngleR, angleRight, 0.2f);
    float armL = lerp(prevAngleL, angleLeft, 0.2f);

    glColor3f(0.95f, 0.8f, 0.7f);
    glPushMatrix();
    glTranslatef(14, 30, 0);
    glRotatef(armR, 0, 0, 1);
    myFilledRect(-4, 0, 8, 30);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-14, 30, 0);
    glRotatef(armL, 0, 0, 1);
    myFilledRect(-4, 0, 8, 30);
    glPopMatrix();

    glColor3f(0.95f, 0.8f, 0.7f);
    myFilledRect(-5, 40, 10, 6);
    myFilledCircle(0, 52, 10);

    glPushMatrix();
    glTranslatef(0, 52, 0);
    drawGirlHair();
    glPopMatrix();

    glPopMatrix();
}

void drawBoyPlayer(float x, float y, float ballX, float ballY, float prevAngleR, float prevAngleL, float breatheScale) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(1.0f + (1.0f - breatheScale) * 0.05f, breatheScale, 1.0f);

    glColor3f(0.9f, 0.75f, 0.65f);
    myFilledRect(-10, -45, 6, 45);
    myFilledRect(4, -45, 6, 45);

    glColor3f(0.2f, 0.6f, 0.9f);
    myFilledRect(-12, 0, 24, 20);

    glColor3f(0.9f, 0.75f, 0.65f);
    myFilledRect(-12, 20, 24, 30);

    float angleRight = atan2(ballY - (y + 38), ballX - (x + 14)) * 180 / PI;
    float angleLeft = atan2(ballY - (y + 38), ballX - (x - 14)) * 180 / PI;
    float armR = lerp(prevAngleR, angleRight, 0.2f);
    float armL = lerp(prevAngleL, angleLeft, 0.2f);

    glColor3f(0.9f, 0.75f, 0.65f);
    glPushMatrix();
    glTranslatef(14, 38, 0);
    glRotatef(armR, 0, 0, 1);
    myFilledRect(-4, 0, 8, 30);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-14, 38, 0);
    glRotatef(armL, 0, 0, 1);
    myFilledRect(-4, 0, 8, 30);
    glPopMatrix();

    glColor3f(0.9f, 0.75f, 0.65f);
    myFilledRect(-5, 50, 10, 6);
    myFilledCircle(0, 62, 10);

    glPushMatrix();
    glTranslatef(0, 62, 0);
    drawBoyHair();
    glPopMatrix();

    glPopMatrix();
}

void drawVolleyballGame(float baseY, float t) {
    // --- UPDATED GAME SPEED ---
    static float prevGirlR = 0, prevGirlL = 0, prevBoyR = 0, prevBoyL = 0;

    // FASTER CYCLE: 2.2 seconds (vs 4.0 previously)
    float cycleTime = 2.2f;

    float phase = fmod(t, cycleTime) / cycleTime;
    float ballX, ballY;

    if (phase < 0.5f) {
        float p = phase / 0.5f;
        ballX = 150 + (250 - 150) * p;
        ballY = baseY + 40 + sinf(p * PI) * 60;
    }
    else {
        float p = (phase - 0.5f) / 0.5f;
        ballX = 250 + (150 - 250) * p;
        ballY = baseY + 20 + sinf(p * PI) * 40;
    }

    // --- FASTER BREATHING ---
    // Multiplier increased to 10.0f (was 5.0f) for higher energy
    float breathe = 1.0f + 0.03f * sinf(t * 10.0f);

    // --- JUMPING LOGIC (Same as before, but faster due to cycleTime) ---
    float girlJump = 0.0f;
    float boyJump = 0.0f;
    float distToGirl = fabs(ballX - 150.0f);
    float distToBoy = fabs(ballX - 250.0f);
    float jumpRange = 50.0f;

    if (distToGirl < jumpRange) {
        girlJump = (jumpRange - distToGirl) * 0.8f;
        breathe = 1.05f;
    }

    if (distToBoy < jumpRange) {
        boyJump = (jumpRange - distToBoy) * 0.8f;
        breathe = 1.05f;
    }

    drawGirlPlayer(150, baseY + girlJump, ballX, ballY, prevGirlR, prevGirlL, breathe);
    drawBoyPlayer(250, baseY + boyJump, ballX, ballY, prevBoyR, prevBoyL, breathe);

    prevGirlR = atan2(ballY - (baseY + 30), ballX - (150 + 14)) * 180 / PI;
    prevGirlL = atan2(ballY - (baseY + 30), ballX - (150 - 14)) * 180 / PI;
    prevBoyR = atan2(ballY - (baseY + 38), ballX - (250 + 14)) * 180 / PI;
    prevBoyL = atan2(ballY - (baseY + 38), ballX - (250 - 14)) * 180 / PI;

    drawVolleyball(ballX, ballY, 12);
}

// ----------------- CALLBACKS -----------------

void mouseMotion(int x, int y) {
    if (!showCredits) {
        umbX_global = (float)x * globalZoom;
    }
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    if (showCredits) {
        showCredits = false;
        glutPostRedisplay();
        glClearColor(0.6f, 0.92f, 1.0f, 1.0f);
    }
    else {
        switch (key) {
        case 'n': case 'N':
            isNightMode = !isNightMode;
            if (isNightMode) {
                glEnable(GL_COLOR_MATERIAL);
                glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
                sStarNextSpawnTime = secs() + 2.0f;
            }
            else {
                glDisable(GL_LIGHTING);
            }
            break;
        case '+':
            globalZoom -= 0.1f;
            if (globalZoom < 0.2f) globalZoom = 0.2f;
            break;
        case '-':
            globalZoom += 0.1f;
            break;
        case 27:
            exit(0);
            break;
        }
    }
}

// ----------------- Main Display -----------------
void display() {
    if (showCredits) {
        drawCredits();
        return;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float cx = WIN_W / 2.0f;
    float cy = WIN_H / 2.0f;
    gluOrtho2D(cx - (cx * globalZoom), cx + (cx * globalZoom),
        cy - (cy * globalZoom), cy + (cy * globalZoom));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (isNightMode) glClearColor(0.05f, 0.05f, 0.2f, 1.0f);
    else glClearColor(0.6f, 0.92f, 1.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float t = secs();

    drawStars();
    drawShootingStar(t);

    drawCelestialBody(180.0f, 650.0f, 50.0f, t);

    drawCloud(200.0f, 600.0f, 1.0f, t);
    drawCloud(500.0f, 650.0f, 0.8f, t);
    drawCloud(850.0f, 620.0f, 1.2f, t);
    drawOceanBase(0.0f, WIN_W, WIN_H * 0.5f, 0.0f, t);

    drawSailBoat(0.0f, WIN_H * 0.5f + 30.0f, t);

    drawSand(WIN_H * 0.35f);
    drawVolleyballGame(WIN_H * 0.35f, t);

    drawPalmTree(800.0f, WIN_H * 0.35f, t, false);
    drawPalmTree(650.0f, WIN_H * 0.35f, t, true);

    drawUmbrella(umbX_global, WIN_H * 0.35f, 50.0f);

    glutSwapBuffers();
}

void init() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    initPalmTreeGeometry();
    initStars();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("FINAL PROJECT");

    init();

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMotionFunc(mouseMotion);

    glutMainLoop();
    return 0;
}