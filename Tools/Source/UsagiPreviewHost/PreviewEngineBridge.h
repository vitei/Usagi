/****************************************************************************
//  Usagi engine bridge for the native preview host.
****************************************************************************/
#pragma once

#ifndef USAGI_PREVIEW_ENGINE_BRIDGE_H
#define USAGI_PREVIEW_ENGINE_BRIDGE_H

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

class PreviewEngineBridge
{
public:
    PreviewEngineBridge();
    ~PreviewEngineBridge();

    bool Initialize(HINSTANCE instance, HWND hwnd, const char* romfilesPath, char* error, int errorSize);
    bool LoadEntity(const char* path, char* error, int errorSize);
    bool LoadParticle(const char* emitterPath, const char* effectPath, char* error, int errorSize);
    void Tick(float deltaTime);
    void SetCameraPosition(float x, float y, float z, float targetX, float targetY, float targetZ);
    void Resize();
    void Shutdown();

    bool IsInitialized() const { return m_initialized; }

private:
    bool m_initialized;
};

#endif
