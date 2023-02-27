#pragma once

#include <GLES3/gl3.h>
#include <EGL/egl.h>

// OpenGL ES 3.0 ������ ���� �Լ�
void RenderLoop(HWND hWnd, EGLDisplay eglDisplay, EGLSurface eglSurface) {
    // OpenGL ES 3.0 �ʱ�ȭ
    // ...

    // ������ ����
    while (true) {
        // �����찡 ������Ʈ�Ǿ����� Ȯ��
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // OpenGL ES 3.0 ������
            glClear(GL_COLOR_BUFFER_BIT);

            // ...

            eglSwapBuffers(eglDisplay, eglSurface);
        }
    }
}
