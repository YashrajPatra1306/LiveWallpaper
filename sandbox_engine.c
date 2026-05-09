#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

#define MAX_OBJECTS 500
#define MAX_PARTICLES 2000
#define TIMER_ID 1
#define TIMER_INTERVAL 16

typedef enum { TOOL_DRAW, TOOL_MOVE, TOOL_DELETE, TOOL_PARTICLES, TOOL_PHYSICS } ToolMode;
typedef enum { SHAPE_CIRCLE, SHAPE_RECT, SHAPE_TRIANGLE, SHAPE_LINE } ShapeType;

typedef struct {
    float x, y;
    float vx, vy;
    float radius;
    float width, height;
    ShapeType shape;
    COLORREF color;
    int active;
    float mass;
    float restitution;
} GameObject;

typedef struct {
    float x, y;
    float vx, vy;
    float life;
    COLORREF color;
    float size;
    int active;
} Particle;

static GameObject objects[MAX_OBJECTS];
static Particle particles[MAX_PARTICLES];
static int objectCount = 0;
static int particleCount = 0;
static ToolMode currentTool = TOOL_DRAW;
static ShapeType currentShape = SHAPE_CIRCLE;
static COLORREFcurrentColor = RGB(100, 150, 255);
static float currentSize = 30.0f;
static float gravity = 500.0f;
static int showPhysics = 1;
static int lowPowerMode = 0;
static HWND hwndCanvas;
static HDC hdcBackBuffer;
static HBITMAP hbmBackBuffer;
static int isDragging = 0;
static int dragIndex = -1;
static float dragOffsetX, dragOffsetY;
static float fps = 0.0f;
static int frameCount = 0;
static DWORD lastFpsTime = 0;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK CanvasProc(HWND, UINT, WPARAM, LPARAM);
void InitObjects();
void UpdatePhysics(float dt);
void Render(HDC hdc);
void SpawnParticles(float x, float y, int count);
int PickObject(float x, float y);
void SaveScene(const char* filename);
void LoadScene(const char* filename);
void ShowAboutDialog(HWND hwnd);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc = {0};
    HWND hwnd;
    MSG msg;
    
    InitCommonControls();
    
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "SandboxEngine";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Failed to register window class", "Error", MB_ICONERROR);
        return 1;
    }
    
    hwnd = CreateWindowEx(
        0,
        "SandboxEngine",
        "Sandbox Rendering Engine - Native Windows",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800,
        NULL, NULL, hInstance, NULL
    );
    
    if (!hwnd) {
        MessageBox(NULL, "Failed to create window", "Error", MB_ICONERROR);
        return 1;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    srand((unsigned int)time(NULL));
    InitObjects();
    
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hwndToolbar, hwndStatus, hwndPanel;
    RECT rc;
    
    switch (msg) {
        case WM_CREATE: {
            hwndPanel = CreateWindowEx(0, "STATIC", "", 
                WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
                0, 0, 200, 0, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            
            hwndToolbar = CreateWindowEx(0, "STATIC", "Tools",
                WS_CHILD | WS_VISIBLE,
                5, 5, 190, 280, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            
            CreateWindow("BUTTON", "Draw", BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE,
                10, 30, 80, 25, hwndToolbar, (HMENU)101, NULL, NULL);
            CreateWindow("BUTTON", "Move", BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE,
                95, 30, 80, 25, hwndToolbar, (HMENU)102, NULL, NULL);
            CreateWindow("BUTTON", "Delete", BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE,
                10, 60, 80, 25, hwndToolbar, (HMENU)103, NULL, NULL);
            CreateWindow("BUTTON", "Particles", BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE,
                95, 60, 80, 25, hwndToolbar, (HMENU)104, NULL, NULL);
            CreateWindow("BUTTON", "Physics", BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE,
                10, 90, 165, 25, hwndToolbar, (HMENU)105, NULL, NULL);
            
            CreateWindow("STATIC", "Shape:", WS_CHILD | WS_VISIBLE,
                10, 125, 170, 20, hwndToolbar, NULL, NULL, NULL);
            CreateWindow("BUTTON", "Circle", BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE,
                10, 145, 80, 20, hwndToolbar, (HMENU)201, NULL, NULL);
            CreateWindow("BUTTON", "Rect", BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE,
                95, 145, 80, 20, hwndToolbar, (HMENU)202, NULL, NULL);
            CreateWindow("BUTTON", "Triangle", BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE,
                10, 170, 80, 20, hwndToolbar, (HMENU)203, NULL, NULL);
            CreateWindow("BUTTON", "Line", BS_RADIOBUTTON | WS_CHILD | WS_VISIBLE,
                95, 170, 80, 20, hwndToolbar, (HMENU)204, NULL, NULL);
            
            CreateWindow("STATIC", "Options:", WS_CHILD | WS_VISIBLE,
                10, 200, 170, 20, hwndToolbar, NULL, NULL, NULL);
            CreateWindow("BUTTON", "Low Power", BS_CHECKBOX | WS_CHILD | WS_VISIBLE,
                10, 220, 165, 20, hwndToolbar, (HMENU)301, NULL, NULL);
            CreateWindow("BUTTON", "Clear All", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
                10, 245, 80, 25, hwndToolbar, (HMENU)302, NULL, NULL);
            CreateWindow("BUTTON", "Save", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
                95, 245, 80, 25, hwndToolbar, (HMENU)303, NULL, NULL);
            CreateWindow("BUTTON", "Load", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
                10, 275, 80, 25, hwndToolbar, (HMENU)304, NULL, NULL);
            CreateWindow("BUTTON", "About", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
                95, 275, 80, 25, hwndToolbar, (HMENU)305, NULL, NULL);
            
            hwndCanvas = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "",
                WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
                210, 35, 0, 0, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            
            SetWindowLongPtr(hwndCanvas, GWLP_WNDPROC, (LONG_PTR)CanvasProc);
            
            hwndStatus = CreateWindowEx(0, "STATIC", "Ready",
                WS_CHILD | WS_VISIBLE | SS_SUNKEN,
                0, 0, 0, 0, hwnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            
            GetClientRect(hwnd, &rc);
            SendMessage(hwnd, WM_SIZE, 0, 0);
            
            SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
            return 0;
        }
        
        case WM_SIZE: {
            GetClientRect(hwnd, &rc);
            MoveWindow(hwndPanel, 0, 0, 205, rc.bottom, TRUE);
            MoveWindow(hwndToolbar, 5, 5, 195, 310, TRUE);
            MoveWindow(hwndCanvas, 210, 35, rc.right - 215, rc.bottom - 70, TRUE);
            MoveWindow(hwndStatus, 0, rc.bottom - 25, rc.right, 25, TRUE);
            return 0;
        }
        
        case WM_TIMER:
            if (wParam == TIMER_ID) {
                frameCount++;
                DWORD now = GetTickCount();
                if (now - lastFpsTime >= 1000) {
                    fps = (float)frameCount * 1000.0f / (now - lastFpsTime);
                    frameCount = 0;
                    lastFpsTime = now;
                }
                
                float dt = 0.016f;
                if (lowPowerMode) dt = 0.032f;
                
                UpdatePhysics(dt);
                InvalidateRect(hwndCanvas, NULL, FALSE);
                
                char status[256];
                sprintf(status, "FPS: %.1f | Objects: %d/%d | Particles: %d/%d | Tool: %s",
                    fps, objectCount, MAX_OBJECTS, particleCount, MAX_PARTICLES,
                    currentTool == TOOL_DRAW ? "Draw" :
                    currentTool == TOOL_MOVE ? "Move" :
                    currentTool == TOOL_DELETE ? "Delete" :
                    currentTool == TOOL_PARTICLES ? "Particles" : "Physics");
                SetWindowText(hwndStatus, status);
            }
            return 0;
        
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id >= 101 && id <= 105) {
                CheckRadioButton(hwndToolbar, 101, 105, id);
                currentTool = (ToolMode)(id - 101);
            } else if (id >= 201 && id <= 204) {
                CheckRadioButton(hwndToolbar, 201, 204, id);
                currentShape = (ShapeType)(id - 201);
            } else if (id == 301) {
                lowPowerMode = IsDlgButtonChecked(hwndToolbar, 301);
            } else if (id == 302) {
                InitObjects();
                particleCount = 0;
            } else if (id == 303) {
                OPENFILENAME ofn = {0};
                char filename[MAX_PATH] = "scene.txt";
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = filename;
                ofn.nMaxFile = MAX_PATH;
                ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrDefExt = "txt";
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
                if (GetSaveFileName(&ofn)) {
                    SaveScene(filename);
                }
            } else if (id == 304) {
                OPENFILENAME ofn = {0};
                char filename[MAX_PATH] = "";
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = filename;
                ofn.nMaxFile = MAX_PATH;
                ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrDefExt = "txt";
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                if (GetOpenFileName(&ofn)) {
                    LoadScene(filename);
                }
            } else if (id == 305) {
                ShowAboutDialog(hwnd);
            }
            return 0;
        }
        
        case WM_DESTROY:
            KillTimer(hwnd, TIMER_ID);
            if (hdcBackBuffer) {
                DeleteDC(hdcBackBuffer);
                DeleteObject(hbmBackBuffer);
            }
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK CanvasProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    HDC hdc;
    POINT pt;
    float x, y;
    RECT rc;
    
    switch (msg) {
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            
            GetClientRect(hwnd, &rc);
            
            if (!hdcBackBuffer || rc.right != 0) {
                if (hdcBackBuffer) {
                    DeleteDC(hdcBackBuffer);
                    DeleteObject(hbmBackBuffer);
                }
                hdcBackBuffer = CreateCompatibleDC(hdc);
                hbmBackBuffer = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
                SelectObject(hdcBackBuffer, hbmBackBuffer);
            }
            
            SetBrushOrgEx(hdcBackBuffer, 0, 0, NULL);
            FillRect(hdcBackBuffer, &rc, (HBRUSH)(COLOR_WHITE + 1));
            
            Render(hdcBackBuffer);
            
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcBackBuffer, 0, 0, SRCCOPY);
            EndPaint(hwnd, &ps);
            return 0;
        
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MOUSEMOVE: {
            GetClientRect(hwnd, &rc);
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            x = (float)pt.x;
            y = (float)pt.y;
            
            if (msg == WM_LBUTTONDOWN) {
                if (currentTool == TOOL_MOVE) {
                    dragIndex = PickObject(x, y);
                    if (dragIndex >= 0) {
                        isDragging = 1;
                        dragOffsetX = x - objects[dragIndex].x;
                        dragOffsetY = y - objects[dragIndex].y;
                        objects[dragIndex].vx = 0;
                        objects[dragIndex].vy = 0;
                    }
                } else if (currentTool == TOOL_DRAW) {
                    if (objectCount < MAX_OBJECTS) {
                        GameObject* obj = &objects[objectCount++];
                        obj->x = x;
                        obj->y = y;
                        obj->vx = 0;
                        obj->vy = 0;
                        obj->shape = currentShape;
                        obj->color = currentColor;
                        obj->active = 1;
                        obj->restitution = 0.7f;
                        
                        if (currentShape == SHAPE_CIRCLE) {
                            obj->radius = currentSize;
                            obj->mass = currentSize * currentSize * 0.01f;
                        } else if (currentShape == SHAPE_RECT) {
                            obj->width = currentSize * 2;
                            obj->height = currentSize * 1.5f;
                            obj->mass = obj->width * obj->height * 0.001f;
                        } else if (currentShape == SHAPE_TRIANGLE) {
                            obj->width = currentSize * 2;
                            obj->height = currentSize * 1.7f;
                            obj->mass = obj->width * obj->height * 0.0005f;
                        } else if (currentShape == SHAPE_LINE) {
                            obj->width = currentSize * 3;
                            obj->height = 3.0f;
                            obj->mass = 0.5f;
                        }
                    }
                } else if (currentTool == TOOL_DELETE) {
                    int idx = PickObject(x, y);
                    if (idx >= 0) {
                        objects[idx].active = 0;
                    }
                } else if (currentTool == TOOL_PARTICLES) {
                    SpawnParticles(x, y, 10);
                } else if (currentTool == TOOL_PHYSICS) {
                    for (int i = 0; i < objectCount; i++) {
                        if (objects[i].active) {
                            float dx = objects[i].x - x;
                            float dy = objects[i].y - y;
                            float dist = sqrtf(dx * dx + dy * dy);
                            if (dist < 200.0f && dist > 0.01f) {
                                float force = 5000.0f / (dist * dist + 100.0f);
                                objects[i].vx += (dx / dist) * force;
                                objects[i].vy += (dy / dist) * force;
                            }
                        }
                    }
                }
            } else if (msg == WM_LBUTTONUP) {
                isDragging = 0;
                dragIndex = -1;
            } else if (msg == WM_MOUSEMOVE && (wParam & MK_LBUTTON)) {
                if (isDragging && dragIndex >= 0) {
                    objects[dragIndex].x = x - dragOffsetX;
                    objects[dragIndex].y = y - dragOffsetY;
                    objects[dragIndex].vx = 0;
                    objects[dragIndex].vy = 0;
                } else if (currentTool == TOOL_PARTICLES && (wParam & MK_LBUTTON)) {
                    SpawnParticles(x, y, 5);
                }
            }
            return 0;
        }
        
        case WM_RBUTTONDOWN: {
            CHOOSECOLOR cc = {0};
            static COLORREF acrCustColors[16];
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hwnd;
            cc.rgbResult = currentColor;
            cc.lpCustColors = acrCustColors;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            if (ChooseColor(&cc)) {
                currentColor = cc.rgbResult;
            }
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void InitObjects() {
    objectCount = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        objects[i].active = 0;
    }
}

void UpdatePhysics(float dt) {
    if (!showPhysics) return;
    
    for (int i = 0; i < objectCount; i++) {
        if (!objects[i].active) continue;
        
        objects[i].vy += gravity * dt;
        objects[i].x += objects[i].vx * dt;
        objects[i].y += objects[i].vy * dt;
        
        RECT rc;
        GetClientRect(hwndCanvas, &rc);
        float right = (float)rc.right;
        float bottom = (float)rc.bottom;
        
        if (objects[i].shape == SHAPE_CIRCLE) {
            if (objects[i].x - objects[i].radius < 0) {
                objects[i].x = objects[i].radius;
                objects[i].vx *= -objects[i].restitution;
            }
            if (objects[i].x + objects[i].radius > right) {
                objects[i].x = right - objects[i].radius;
                objects[i].vx *= -objects[i].restitution;
            }
            if (objects[i].y - objects[i].radius < 0) {
                objects[i].y = objects[i].radius;
                objects[i].vy *= -objects[i].restitution;
            }
            if (objects[i].y + objects[i].radius > bottom) {
                objects[i].y = bottom - objects[i].radius;
                objects[i].vy *= -objects[i].restitution;
            }
        } else {
            float w = objects[i].width / 2;
            float h = objects[i].height / 2;
            if (objects[i].x - w < 0) {
                objects[i].x = w;
                objects[i].vx *= -objects[i].restitution;
            }
            if (objects[i].x + w > right) {
                objects[i].x = right - w;
                objects[i].vx *= -objects[i].restitution;
            }
            if (objects[i].y - h < 0) {
                objects[i].y = h;
                objects[i].vy *= -objects[i].restitution;
            }
            if (objects[i].y + h > bottom) {
                objects[i].y = bottom - h;
                objects[i].vy *= -objects[i].restitution;
            }
        }
        
        for (int j = i + 1; j < objectCount; j++) {
            if (!objects[j].active) continue;
            
            float dx = objects[j].x - objects[i].x;
            float dy = objects[j].y - objects[i].y;
            float dist = sqrtf(dx * dx + dy * dy);
            float minDist = 0;
            
            if (objects[i].shape == SHAPE_CIRCLE && objects[j].shape == SHAPE_CIRCLE) {
                minDist = objects[i].radius + objects[j].radius;
            } else {
                minDist = (objects[i].width + objects[j].width) / 4;
            }
            
            if (dist < minDist && dist > 0.01f) {
                float overlap = minDist - dist;
                float nx = dx / dist;
                float ny = dy / dist;
                
                float totalMass = objects[i].mass + objects[j].mass;
                objects[i].x -= nx * overlap * (objects[j].mass / totalMass);
                objects[i].y -= ny * overlap * (objects[j].mass / totalMass);
                objects[j].x += nx * overlap * (objects[i].mass / totalMass);
                objects[j].y += ny * overlap * (objects[i].mass / totalMass);
                
                float dvx = objects[i].vx - objects[j].vx;
                float dvy = objects[i].vy - objects[j].vy;
                float dvn = dvx * nx + dvy * ny;
                
                if (dvn > 0) {
                    float restitution = (objects[i].restitution + objects[j].restitution) / 2;
                    float impulse = (2 * dvn) / totalMass * restitution;
                    objects[i].vx -= impulse * objects[j].mass * nx;
                    objects[i].vy -= impulse * objects[j].mass * ny;
                    objects[j].vx += impulse * objects[i].mass * nx;
                    objects[j].vy += impulse * objects[i].mass * ny;
                }
            }
        }
        
        objects[i].vx *= 0.995f;
        objects[i].vy *= 0.995f;
    }
    
    for (int i = 0; i < particleCount; i++) {
        if (!particles[i].active) continue;
        
        particles[i].life -= dt;
        if (particles[i].life <= 0) {
            particles[i].active = 0;
            continue;
        }
        
        particles[i].x += particles[i].vx * dt;
        particles[i].y += particles[i].vy * dt;
        particles[i].vy += gravity * 0.5f * dt;
        
        RECT rc;
        GetClientRect(hwndCanvas, &rc);
        if (particles[i].x < 0 || particles[i].x > rc.right ||
            particles[i].y < 0 || particles[i].y > rc.bottom) {
            particles[i].active = 0;
        }
    }
}

void Render(HDC hdc) {
    for (int i = 0; i < objectCount; i++) {
        if (!objects[i].active) continue;
        
        HBRUSH brush = CreateSolidBrush(objects[i].color);
        HPEN pen = CreatePen(PS_SOLID, 1, objects[i].color);
        SelectObject(hdc, brush);
        SelectObject(hdc, pen);
        
        if (objects[i].shape == SHAPE_CIRCLE) {
            Ellipse(hdc, 
                (int)(objects[i].x - objects[i].radius),
                (int)(objects[i].y - objects[i].radius),
                (int)(objects[i].x + objects[i].radius),
                (int)(objects[i].y + objects[i].radius));
        } else if (objects[i].shape == SHAPE_RECT) {
            Rectangle(hdc,
                (int)(objects[i].x - objects[i].width / 2),
                (int)(objects[i].y - objects[i].height / 2),
                (int)(objects[i].x + objects[i].width / 2),
                (int)(objects[i].y + objects[i].height / 2));
        } else if (objects[i].shape == SHAPE_TRIANGLE) {
            POINT pts[3];
            pts[0].x = (int)objects[i].x;
            pts[0].y = (int)(objects[i].y - objects[i].height / 2);
            pts[1].x = (int)(objects[i].x - objects[i].width / 2);
            pts[1].y = (int)(objects[i].y + objects[i].height / 2);
            pts[2].x = (int)(objects[i].x + objects[i].width / 2);
            pts[2].y = (int)(objects[i].y + objects[i].height / 2);
            Polygon(hdc, pts, 3);
        } else if (objects[i].shape == SHAPE_LINE) {
            MoveToEx(hdc, (int)(objects[i].x - objects[i].width / 2), (int)objects[i].y, NULL);
            LineTo(hdc, (int)(objects[i].x + objects[i].width / 2), (int)objects[i].y);
        }
        
        DeleteObject(brush);
        DeleteObject(pen);
    }
    
    for (int i = 0; i < particleCount; i++) {
        if (!particles[i].active) continue;
        
        HBRUSH brush = CreateSolidBrush(particles[i].color);
        SelectObject(hdc, brush);
        Ellipse(hdc,
            (int)(particles[i].x - particles[i].size),
            (int)(particles[i].y - particles[i].size),
            (int)(particles[i].x + particles[i].size),
            (int)(particles[i].y + particles[i].size));
        DeleteObject(brush);
    }
}

void SpawnParticles(float x, float y, int count) {
    for (int i = 0; i < count && particleCount < MAX_PARTICLES; i++) {
        Particle* p = &particles[particleCount++];
        p->x = x;
        p->y = y;
        p->vx = ((float)rand() / RAND_MAX - 0.5f) * 200;
        p->vy = ((float)rand() / RAND_MAX - 0.5f) * 200;
        p->life = 1.0f + ((float)rand() / RAND_MAX) * 2.0f;
        p->color = currentColor;
        p->size = 3.0f + ((float)rand() / RAND_MAX) * 4.0f;
        p->active = 1;
    }
}

int PickObject(float x, float y) {
    for (int i = objectCount - 1; i >= 0; i--) {
        if (!objects[i].active) continue;
        
        float dx = x - objects[i].x;
        float dy = y - objects[i].y;
        
        if (objects[i].shape == SHAPE_CIRCLE) {
            if (dx * dx + dy * dy <= objects[i].radius * objects[i].radius) {
                return i;
            }
        } else {
            if (fabsf(dx) <= objects[i].width / 2 && fabsf(dy) <= objects[i].height / 2) {
                return i;
            }
        }
    }
    return -1;
}

void SaveScene(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return;
    
    fprintf(f, "%d\n", objectCount);
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].active) {
            fprintf(f, "%d %f %f %f %f %f %f %d %lu %f %f\n",
                objects[i].shape,
                objects[i].x, objects[i].y,
                objects[i].vx, objects[i].vy,
                objects[i].radius,
                objects[i].width, objects[i].height,
                objects[i].color,
                objects[i].mass, objects[i].restitution);
        }
    }
    fclose(f);
}

void LoadScene(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return;
    
    InitObjects();
    int count;
    if (fscanf(f, "%d", &count) != 1) {
        fclose(f);
        return;
    }
    
    for (int i = 0; i < count && objectCount < MAX_OBJECTS; i++) {
        GameObject* obj = &objects[objectCount++];
        fscanf(f, "%d %f %f %f %f %f %f %f %lu %f %f",
            &obj->shape,
            &obj->x, &obj->y,
            &obj->vx, &obj->vy,
            &obj->radius,
            &obj->width, &obj->height,
            &obj->color,
            &obj->mass, &obj->restitution);
        obj->active = 1;
    }
    fclose(f);
}

void ShowAboutDialog(HWND hwnd) {
    MessageBox(hwnd,
        "Sandbox Rendering Engine\n\n"
        "A native Windows application optimized for low-tier hardware.\n\n"
        "Features:\n"
        "- Multiple shape types (Circle, Rect, Triangle, Line)\n"
        "- Full physics simulation with collisions\n"
        "- Particle system\n"
        "- Save/Load scenes\n"
        "- Low power mode for better performance\n\n"
        "Controls:\n"
        "- Left Click: Use selected tool\n"
        "- Right Click: Choose color\n"
        "- Tools: Draw, Move, Delete, Particles, Physics",
        "About", MB_OK | MB_ICONINFORMATION);
}
