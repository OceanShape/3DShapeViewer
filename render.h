#pragma once

#include <GLES3/gl3.h>
#include <EGL/egl.h>

// OpenGL ES 3.0 렌더링 루프 함수
void RenderLoop(HWND hWnd, EGLDisplay eglDisplay, EGLSurface eglSurface) {
    // OpenGL ES 3.0 초기화
    // ...

    // 렌더링 루프
    while (true) {
        // 윈도우가 업데이트되었는지 확인
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // OpenGL ES 3.0 렌더링
            glClear(GL_COLOR_BUFFER_BIT);

            // ...

            eglSwapBuffers(eglDisplay, eglSurface);
        }
    }
}
