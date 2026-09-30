#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "math3d.hpp"

#include <windows.h>
#include <windowsx.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <string>


enum {
    IDC_POINT_FIELD = 101,
    IDC_ANGLE_FIELD = 102,
    IDC_CALC_BUTTON = 103,
    IDC_RESET_BUTTON = 104,
    IDC_RESULT_TEXT = 105,
    IDC_AXIS_TEXT = 106,
    IDC_AXIS_X = 108,
    IDC_AXIS_Y = 109,
    IDC_AXIS_Z = 110
};

constexpr int kPanelWidth = 360;
constexpr wchar_t kMainClass[] = L"PointRotationMain";
constexpr wchar_t kGlClass[] = L"PointRotationGL";

struct App { 
    HWND hwnd = nullptr;
    HWND glHwnd = nullptr;
    HWND pointEdit = nullptr;
    HWND angleEdit = nullptr;
    HWND resultLabel = nullptr;
    HWND axisLabel = nullptr;
    HWND axisBtnX = nullptr;
    HWND axisBtnY = nullptr;
    HWND axisBtnZ = nullptr;
    HDC glDc = nullptr;
    HGLRC glRc = nullptr;
    HBRUSH panelBrush = nullptr;
    HBRUSH editBrush = nullptr;
    HFONT titleFont = nullptr;
    HFONT uiFont = nullptr;
    HFONT resultFont = nullptr;

    GLuint fontBase = 0;

    Vec3 point{1.20f, 0.70f, 0.40f};
    Axis axis = Axis::None;
    float angleDeg = 45.0f;

    float camYaw = 0.70f;
    float camPitch = 0.42f;
    float camDist = 13.0f;
    bool orbiting = false;
    bool mouseMoved = false;
    POINT lastMouse{};

    bool animating = false;
    float animAngle = 0.0f;  // current glRotatef angle
    float animTarget = 0.0f;
    bool hasResult = false;
    Vec3 result{0, 0, 0};

    int width = 1280;
    int height = 800;
};

App g;

Vec3 cameraEye() { //cam eye pos on calculation
    const float cp = std::cos(g.camPitch);
    return {g.camDist * cp * std::sin(g.camYaw), g.camDist * std::sin(g.camPitch),
            g.camDist * cp * std::cos(g.camYaw)};
}

std::wstring toWide(const std::string& s) { return std::wstring(s.begin(), s.end()); }

void setLabel(HWND hwnd, const std::wstring& text) { SetWindowTextW(hwnd, text.c_str()); }

void setupCamera(int w, int h) { //setup camera and drag func
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(42.0, static_cast<double>(w) / h, 0.1, 80.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    const Vec3 eye = cameraEye();
    gluLookAt(eye.x, eye.y, eye.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void glPrint(const Vec3& pos, const char* text) {
    glDisable(GL_DEPTH_TEST);
    glRasterPos3f(pos.x, pos.y, pos.z);
    GLboolean valid = GL_FALSE;
    glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &valid);
    if (valid) {
        glListBase(g.fontBase);
        glCallLists(static_cast<GLsizei>(std::strlen(text)), GL_UNSIGNED_BYTE, text);
    }
    glEnable(GL_DEPTH_TEST);
}

void drawLine(const Vec3& a, const Vec3& b) { //draw line for stracture of cube
    glBegin(GL_LINES);
    glVertex3f(a.x, a.y, a.z);
    glVertex3f(b.x, b.y, b.z);
    glEnd();
}

void drawWorldAxes() { //draw axis visually
    const float len = 1.85f;
    const float thickX = (g.axis == Axis::X) ? 4.0f : 2.0f;
    const float thickY = (g.axis == Axis::Y) ? 4.0f : 2.0f;
    const float thickZ = (g.axis == Axis::Z) ? 4.0f : 2.0f;

    glColor3f(0.95f, 0.25f, 0.25f);
    glLineWidth(thickX);
    drawLine({-len, 0, 0}, {len, 0, 0});
    glPrint({len * 1.06f, 0.06f, 0}, "X");

    glColor3f(0.25f, 0.85f, 0.35f);
    glLineWidth(thickY);
    drawLine({0, -len, 0}, {0, len, 0});
    glPrint({0.06f, len * 1.06f, 0}, "Y");

    glColor3f(0.30f, 0.55f, 1.0f);
    glLineWidth(thickZ);
    drawLine({0, 0, -len}, {0, 0, len});
    glPrint({0.06f, 0.06f, len * 1.06f}, "Z");
}

void drawPointMarker(const Vec3& p, float r, float gcol, float b) { //draw point visually
    const float m = 0.08f;
    glColor3f(r, gcol, b);
    glPointSize(14.0f);
    glBegin(GL_POINTS);
    glVertex3f(p.x, p.y, p.z);
    glEnd();
    glLineWidth(2.0f);
    drawLine({p.x - m, p.y, p.z}, {p.x + m, p.y, p.z});
    drawLine({p.x, p.y - m, p.z}, {p.x, p.y + m, p.z});
    drawLine({p.x, p.y, p.z - m}, {p.x, p.y, p.z + m});
}

void renderScene() { //render scene (cube, point and new point)
    if (!g.glRc) {
        return;
    }
    wglMakeCurrent(g.glDc, g.glRc);

    RECT rc{};
    GetClientRect(g.glHwnd, &rc);
    const int w = rc.right - rc.left;
    const int h = rc.bottom - rc.top;

    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.031f, 0.047f, 0.094f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    setupCamera(w, h);
    drawWorldAxes();

    glColor3f(0.35f, 0.85f, 1.0f);
    glLineWidth(2.0f);
    drawLine({0, 0, 0}, g.point);
    drawPointMarker(g.point, 0.20f, 0.90f, 1.0f);
    glPrint(g.point + Vec3{0.12f, 0.14f, 0}, "P");

    if (g.hasResult || g.animating) {
        const Vec3 live = rotateAroundAxis(g.point, g.axis, g.animAngle);
        glColor3f(1.0f, 0.82f, 0.20f);
        glLineWidth(2.0f);
        drawLine({0, 0, 0}, live);
        drawPointMarker(live, 1.0f, 0.82f, 0.20f);
        glPrint(live + Vec3{0.12f, 0.14f, 0}, "P'");
    }

    drawPointMarker({0, 0, 0}, 1.0f, 1.0f, 1.0f);

    SwapBuffers(g.glDc);
}

void selectAxis(Axis axis) { //select axis
    g.axis = axis;
    wchar_t buf[64];
    swprintf(buf, 64, L"Selected axis:  %s", toWide(axisName(g.axis)).c_str());
    setLabel(g.axisLabel, buf);
}

bool parseFields(Vec3& point, float& angle, std::wstring& error) { //recieve fields from insertion vals 
    wchar_t pbuf[128]{};
    wchar_t abuf[64]{};
    GetWindowTextW(g.pointEdit, pbuf, 128);
    GetWindowTextW(g.angleEdit, abuf, 64);

    float x = 0, y = 0, z = 0;
    if (swscanf(pbuf, L"%f , %f , %f", &x, &y, &z) != 3 && swscanf(pbuf, L"%f %f %f", &x, &y, &z) != 3) {
        error = L"Point must look like:  1.2, 0.7, 0.4";
        return false;
    }
    float a = 0;
    if (swscanf(abuf, L"%f", &a) != 1) { 
        error = L"Angle must be a number in degrees, for example 45";
        return false;
    }
    point = {x, y, z};
    angle = a;
    return true;
}

void showResult(const Vec3& p) { //show result of calc
    wchar_t buf[160];
    swprintf(buf, 160, L"P'  =  (%.3f,  %.3f,  %.3f)", p.x, p.y, p.z);
    setLabel(g.resultLabel, buf);
    std::printf("P  = (%.6f, %.6f, %.6f)\n", g.point.x, g.point.y, g.point.z);
    std::printf("Rotate %.3f deg around %s\n", g.angleDeg, axisName(g.axis));
    std::printf("P' = (%.6f, %.6f, %.6f)\n\n", p.x, p.y, p.z);
    std::fflush(stdout);
}

void onCalculate() { //calc animation
    std::wstring error;
    Vec3 point;
    float angle = 0;
    if (!parseFields(point, angle, error)) {
        MessageBoxW(g.hwnd, error.c_str(), L"Invalid input", MB_OK | MB_ICONWARNING);
        return;
    }
    if (g.axis == Axis::None) {
        MessageBoxW(g.hwnd, L"Press the X, Y or Z button first.", L"Choose an axis",
                    MB_OK | MB_ICONINFORMATION);
        return;
    }

    g.point = point;
    g.angleDeg = angle;
    g.animTarget = angle;
    g.animAngle = 0.0f;
    g.animating = true;
    g.hasResult = false;
    g.result = rotateAroundAxis(g.point, g.axis, g.angleDeg);
    setLabel(g.resultLabel, L"Calculating...");
}

void onReset() { //reset val to default
    g.animating = false;
    g.animAngle = 0.0f;
    g.hasResult = false;
    g.axis = Axis::None;
    g.camYaw = 0.70f;
    g.camPitch = 0.42f;
    g.camDist = 13.0f;
    setLabel(g.pointEdit, L"1.2, 0.7, 0.4");
    setLabel(g.angleEdit, L"45");
    setLabel(g.resultLabel, L"P'  =  (waiting for Calculate Point)");
    setLabel(g.axisLabel, L"Selected axis:  (press X, Y or Z)");
}

void updateAnimation() { //update animation by speed and dir
    if (!g.animating) {
        return;
    }
    const float speed = 1.8f;  
    const float dir = g.animTarget >= 0 ? 1.0f : -1.0f;
    g.animAngle += speed * dir;
    const bool done = (dir > 0 && g.animAngle >= g.animTarget) || (dir < 0 && g.animAngle <= g.animTarget);
    if (done) {
        g.animAngle = g.animTarget;
        g.animating = false;
        g.hasResult = true;
        showResult(g.result);
    }
}

bool initGl(HWND hwnd) { //init gl
    g.glDc = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;
    const int format = ChoosePixelFormat(g.glDc, &pfd);
    if (!format || !SetPixelFormat(g.glDc, format, &pfd)) {
        return false;
    }
    g.glRc = wglCreateContext(g.glDc);
    if (!g.glRc || !wglMakeCurrent(g.glDc, g.glRc)) {
        return false;
    }

    g.fontBase = glGenLists(256);
    SelectObject(g.glDc, GetStockObject(SYSTEM_FONT));
    wglUseFontBitmapsW(g.glDc, 0, 256, g.fontBase);
    glEnable(GL_POINT_SMOOTH);
    return true;
}

void shutdownGl() { //shutdown gl 
    wglMakeCurrent(nullptr, nullptr);
    if (g.glRc) {
        wglDeleteContext(g.glRc);
        g.glRc = nullptr;
    }
    if (g.glDc && g.glHwnd) {
        ReleaseDC(g.glHwnd, g.glDc);
        g.glDc = nullptr;
    }
}

HWND addLabel(HWND parent, const wchar_t* text, int x, int y, int w, int h, int id = 0, HFONT font = nullptr) { //label for text
    HWND hwnd = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE, x, y, w, h, parent,
                              reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandle(nullptr), nullptr);
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(font ? font : g.uiFont), TRUE);
    return hwnd;
}

HWND addEdit(HWND parent, const wchar_t* text, int x, int y, int w, int h, int id) { //edit box
    HWND hwnd = CreateWindowExW(0, L"EDIT", text, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER, x, y,
                                w, h, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandle(nullptr),
                                nullptr);
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g.uiFont), TRUE);
    return hwnd;
}

HWND addButton(HWND parent, const wchar_t* text, int x, int y, int w, int h, int id, DWORD extraStyle = BS_PUSHBUTTON) { //button 
    HWND hwnd = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | extraStyle, x, y, w, h, parent,
                              reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandle(nullptr), nullptr);
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(g.uiFont), TRUE);
    return hwnd;
}

void createPanel(HWND parent) { //ui panel 
    g.titleFont = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0,
                              L"Segoe UI");
    g.uiFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0,
                           L"Segoe UI");
    g.resultFont = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0,
                               L"Segoe UI");

    addLabel(parent, L"Rotation Matrix Point Calculator", 16, 20, 336, 40, 0, g.titleFont);

    addLabel(parent, L"1. Point  P   (x, y, z)", 24, 100, 320, 22);
    g.pointEdit = addEdit(parent, L"1.2, 0.7, 0.4", 24, 126, 312, 32, IDC_POINT_FIELD);

    addLabel(parent, L"2. Angle  (degrees)", 24, 176, 320, 22);
    g.angleEdit = addEdit(parent, L"45", 24, 202, 312, 32, IDC_ANGLE_FIELD);

    addLabel(parent, L"3. Choose an axis", 24, 252, 320, 22);
    g.axisBtnX = addButton(parent, L"X", 24, 278, 96, 40, IDC_AXIS_X);
    g.axisBtnY = addButton(parent, L"Y", 132, 278, 96, 40, IDC_AXIS_Y);
    g.axisBtnZ = addButton(parent, L"Z", 240, 278, 96, 40, IDC_AXIS_Z);
    g.axisLabel = addLabel(parent, L"Selected axis:  (press X, Y or Z)", 24, 326, 320, 24, IDC_AXIS_TEXT);

    addButton(parent, L"Calculate Point", 24, 364, 312, 44, IDC_CALC_BUTTON, BS_DEFPUSHBUTTON);
    addButton(parent, L"Reset", 24, 418, 312, 34, IDC_RESET_BUTTON);

    addLabel(parent, L"Calculated point", 24, 470, 320, 22);
    g.resultLabel = addLabel(parent, L"P'  =  (waiting for Calculate Point)", 24, 496, 312, 50, IDC_RESULT_TEXT,
                             g.resultFont);
}

LRESULT CALLBACK GlProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) { 
    switch (msg) {
        case WM_LBUTTONDOWN:
            g.orbiting = true;
            g.mouseMoved = false;
            g.lastMouse = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            SetCapture(hwnd);
            return 0;
        case WM_MOUSEMOVE:
            if (g.orbiting) {
                const int x = GET_X_LPARAM(lParam);
                const int y = GET_Y_LPARAM(lParam);
                const int dx = x - g.lastMouse.x;
                const int dy = y - g.lastMouse.y;
                if (std::abs(dx) + std::abs(dy) > 3) {
                    g.mouseMoved = true;
                }
                if (g.mouseMoved) {
                    g.camYaw += dx * 0.01f;
                    g.camPitch = clampf(g.camPitch - dy * 0.01f, -1.2f, 1.2f);
                    g.lastMouse = {x, y};
                }
            }
            return 0; 
        case WM_LBUTTONUP:
            g.orbiting = false;
            ReleaseCapture();
            return 0;
        case WM_MOUSEWHEEL:
            g.camDist = clampf(g.camDist - GET_WHEEL_DELTA_WPARAM(wParam) / 120.0f * 0.45f, 4.0f, 28.0f);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            renderScene();
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) { 
    switch (msg) {
        case WM_CREATE:
            createPanel(hwnd);
            g.glHwnd = CreateWindowW(kGlClass, L"", WS_CHILD | WS_VISIBLE, kPanelWidth, 0, 920, 800, hwnd, nullptr,
                                     GetModuleHandle(nullptr), nullptr);
            if (!initGl(g.glHwnd)) {
                MessageBoxW(hwnd, L"Could not start OpenGL.", L"Error", MB_ICONERROR);
            }
            SetTimer(hwnd, 1, 16, nullptr);
            return 0;
        case WM_SIZE: {
            g.width = LOWORD(lParam);
            g.height = HIWORD(lParam);
            if (g.glHwnd) {
                MoveWindow(g.glHwnd, kPanelWidth, 0, std::max(1, g.width - kPanelWidth), std::max(1, g.height), TRUE);
            }
            return 0;
        }
        case WM_TIMER:
            updateAnimation();
            renderScene();
            return 0;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_CALC_BUTTON:
                    onCalculate();
                    break;
                case IDC_RESET_BUTTON:
                    onReset();
                    break;
                case IDC_AXIS_X:
                    selectAxis(Axis::X);
                    break;
                case IDC_AXIS_Y:
                    selectAxis(Axis::Y);
                    break;
                case IDC_AXIS_Z:
                    selectAxis(Axis::Z);
                    break;
            }
            return 0;
        case WM_CTLCOLORSTATIC: {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetTextColor(hdc, RGB(230, 236, 245));
            SetBkColor(hdc, RGB(12, 18, 32));
            return reinterpret_cast<LRESULT>(g.panelBrush);
        }
        case WM_CTLCOLOREDIT: {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetTextColor(hdc, RGB(20, 24, 36));
            SetBkColor(hdc, RGB(236, 242, 250));
            return reinterpret_cast<LRESULT>(g.editBrush);
        }
        case WM_ERASEBKGND: {
            RECT rc;
            GetClientRect(hwnd, &rc);
            rc.right = kPanelWidth;
            FillRect(reinterpret_cast<HDC>(wParam), &rc, g.panelBrush);
            return 1;
        }
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            shutdownGl();
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int wmain() { //main func 
    g.panelBrush = CreateSolidBrush(RGB(12, 18, 32));
    g.editBrush = CreateSolidBrush(RGB(236, 242, 250));

    WNDCLASSW mainWc{};
    mainWc.lpfnWndProc = MainProc;
    mainWc.hInstance = GetModuleHandle(nullptr);
    mainWc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    mainWc.hbrBackground = g.panelBrush;
    mainWc.lpszClassName = kMainClass;
    RegisterClassW(&mainWc);

    WNDCLASSW glWc{};
    glWc.lpfnWndProc = GlProc;
    glWc.hInstance = mainWc.hInstance;
    glWc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    glWc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    glWc.lpszClassName = kGlClass;
    glWc.style = CS_OWNDC;
    RegisterClassW(&glWc);

    RECT wr{0, 0, 1280, 800};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    g.hwnd = CreateWindowW(kMainClass, L"Rotation Matrix Point Calculator", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                           CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr,
                           mainWc.hInstance, nullptr);

    HWND console = GetConsoleWindow();
    if (console) {
        DWORD pid = 0;
        GetWindowThreadProcessId(console, &pid);
        if (pid == GetCurrentProcessId()) {
            ShowWindow(console, SW_HIDE);
        }
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(g.panelBrush);
    DeleteObject(g.editBrush);
    DeleteObject(g.titleFont);
    DeleteObject(g.uiFont);
    DeleteObject(g.resultFont);
    return static_cast<int>(msg.wParam);
}
