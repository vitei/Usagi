/****************************************************************************
//  Minimal native preview host for Avalonia tools.
****************************************************************************/
#pragma once

#ifndef USAGI_PREVIEW_HOST_H
#define USAGI_PREVIEW_HOST_H

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "IpcProtocol.h"

class PreviewHost
{
public:
    PreviewHost();
    ~PreviewHost();

    int Run(HINSTANCE instance, int showCommand);

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

    bool CreatePreviewWindow(HINSTANCE instance);
    void PumpStdIn();
    void ProcessLine(const char* line);
    void HandleInit(const IpcInitCommand& command);
    void HandleAttachWindow(const IpcAttachWindowCommand& command);
    void HandleLoadEntity(const IpcLoadEntityCommand& command);
    void HandleLoadParticle(const IpcLoadParticleCommand& command);
    void HandlePick(const IpcPickCommand& command);
    void HandleShutdown();

    void ResizeHostedWindow(int width, int height);
    void SendResponse(const char* json);
    void SendReady();
    void SendError(const char* message, const char* details = nullptr);
    void SendLoaded(const char* resourceType, const char* path, bool success, const char* error = nullptr);
    void SendDiagnostic(const char* level, const char* message, const char* source = nullptr);

    HWND m_hwnd;
    HWND m_parentHwnd;
    HANDLE m_stdin;
    HANDLE m_stdout;
    DWORD m_stdinType;
    bool m_shutdownRequested;
    bool m_protocolReady;
    char m_inputBuffer[8192];
    int m_inputPos;
};

#endif
