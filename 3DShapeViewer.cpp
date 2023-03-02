#define _CRT_SECURE_NO_WARNINGS

#include "framework.h"
#include "3DShapeViewer.h"

using namespace std;

EGLint EGL_OPENGL_ES3_BIT_KHR = 0x0040;

// Window dimensions
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

// OpenGL context and window handle
EGLDisplay eglDisplay;
EGLSurface eglSurface;
EGLContext eglContext;
HWND hWnd;

// Vertex shader source code
const char* vertexShaderSource =
"#version 300 es\n"
"layout(location = 0) in vec4 a_position;\n"
"void main() {\n"
"  gl_Position = a_position;\n"
"}\n";

// Fragment shader source code
const char* fragmentShaderSource =
"#version 300 es\n"
"precision mediump float;\n"
"out vec4 fragColor;\n"
"void main() {\n"
"  fragColor = vec4(1.0, 1.0, 0.0, 1.0);\n"
"}\n";

// Vertex data
const GLfloat vertexData[] = {
    0.0f, 0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f
};

// Vertex buffer object and attribute location
GLuint vbo;
GLint positionAttr;

// Initialize OpenGL context and window
void initializeOpenGL()
{
    // Initialize EGL
    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLConfig eglConfig;
    EGLint attribs[] = {
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR, // OpenGL ES 3.0 컨텍스트 요청
    EGL_NONE
    };
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(eglDisplay, &majorVersion, &minorVersion);
    eglChooseConfig(eglDisplay, attribs, &eglConfig, 1, &numConfigs);
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, hWnd, NULL);
    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs);
    eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);

    // Compile and link shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glUseProgram(program);

    // Create and bind vertex buffer object
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    // Get position attribute location and enable vertex attribute array
    positionAttr = glGetAttribLocation(program, "a_position");
    glEnableVertexAttribArray(positionAttr);

    // Set vertex attribute pointer
    glVertexAttribPointer(positionAttr, 3, GL_FLOAT, GL_FALSE, 0, 0);

}

// Render OpenGL scene
void drawOpenGL()
{
    // Clear color buffer
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // Draw triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Swap buffers
    eglSwapBuffers(eglDisplay, eglSurface);
}

// Clean up OpenGL context and window
void cleanupOpenGL()
{
    // Delete vertex buffer object
    glDeleteBuffers(1, &vbo);
    // Delete program and shaders
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    GLuint vertexShader;
    glGetAttachedShaders(program, 1, NULL, &vertexShader);
    GLuint fragmentShader;
    glGetAttachedShaders(program, 1, NULL, &fragmentShader);
    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteProgram(program);
    // Terminate EGL
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglSurface);
    eglTerminate(eglDisplay);
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Register window class
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(wcex);
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.lpszClassName = L"OpenGLWindowClass";
    RegisterClassEx(&wcex);
    // Create window
    hWnd = CreateWindowEx(0, L"OpenGLWindowClass", L"OpenGL ES 3.0 Window", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);

    // Initialize OpenGL
    initializeOpenGL();

    // Enter message loop
    MSG msg = {};
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            drawOpenGL();
        }
    }

    // Clean up OpenGL
    cleanupOpenGL();

    return static_cast<int>(msg.wParam);
}