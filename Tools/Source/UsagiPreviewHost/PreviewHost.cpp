/****************************************************************************
//  Minimal native preview host for Avalonia tools.
****************************************************************************/
#include "PreviewHost.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

static const char* kWindowClassName = "UsagiPreviewHostWindow";
static const char* kEngineVersion = "UsagiPreviewHost engine bootstrap";

PreviewHost::PreviewHost()
    : m_hwnd(nullptr)
    , m_parentHwnd(nullptr)
    , m_stdin(GetStdHandle(STD_INPUT_HANDLE))
    , m_stdout(GetStdHandle(STD_OUTPUT_HANDLE))
    , m_stdinType(FILE_TYPE_UNKNOWN)
    , m_shutdownRequested(false)
    , m_protocolReady(false)
    , m_instance(nullptr)
    , m_inputPos(0)
{
    std::memset(m_inputBuffer, 0, sizeof(m_inputBuffer));
    std::memset(&m_initCommand, 0, sizeof(m_initCommand));
    if (m_stdin != INVALID_HANDLE_VALUE && m_stdin != nullptr)
    {
        m_stdinType = GetFileType(m_stdin);
    }
}

PreviewHost::~PreviewHost()
{
    if (m_hwnd != nullptr)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

int PreviewHost::Run(HINSTANCE instance, int showCommand)
{
    UNREFERENCED_PARAMETER(showCommand);
    m_instance = instance;

    if (!CreatePreviewWindow(instance))
    {
        SendError("Failed to create preview host window");
        return 1;
    }

    SendDiagnostic("info", "Preview host process started");

    MSG message = {};
    while (!m_shutdownRequested)
    {
        while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                m_shutdownRequested = true;
                break;
            }

            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        PumpStdIn();
        Sleep(8);
    }

    m_engine.Shutdown();
    return 0;
}

bool PreviewHost::CreatePreviewWindow(HINSTANCE instance)
{
    WNDCLASSEXA windowClass = {};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = PreviewHost::WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.lpszClassName = kWindowClassName;

    const ATOM registeredClass = RegisterClassExA(&windowClass);
    if (registeredClass == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        return false;
    }

    m_hwnd = CreateWindowExA(
        0,
        kWindowClassName,
        "Usagi Preview Host",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        800,
        600,
        nullptr,
        nullptr,
        instance,
        this);

    if (m_hwnd == nullptr)
    {
        return false;
    }

    ShowWindow(m_hwnd, SW_HIDE);
    return true;
}

LRESULT CALLBACK PreviewHost::WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    PreviewHost* host = reinterpret_cast<PreviewHost*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (message == WM_NCCREATE)
    {
        const CREATESTRUCT* createStruct = reinterpret_cast<const CREATESTRUCT*>(lparam);
        host = reinterpret_cast<PreviewHost*>(createStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(host));
    }

    switch (message)
    {
    case WM_SIZE:
        if (host != nullptr && wparam != SIZE_MINIMIZED)
        {
            host->ResizeHostedWindow(LOWORD(lparam), HIWORD(lparam));
        }
        break;

    case WM_CLOSE:
        if (host != nullptr)
        {
            host->HandleShutdown();
            return 0;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, message, wparam, lparam);
}

void PreviewHost::PumpStdIn()
{
    if (m_stdin == INVALID_HANDLE_VALUE || m_stdin == nullptr)
    {
        return;
    }

    DWORD available = 0;
    if (m_stdinType == FILE_TYPE_PIPE)
    {
        if (!PeekNamedPipe(m_stdin, nullptr, 0, nullptr, &available, nullptr))
        {
            const DWORD error = GetLastError();
            if (error == ERROR_BROKEN_PIPE || error == ERROR_HANDLE_EOF)
            {
                m_shutdownRequested = true;
            }
            return;
        }

        if (available == 0)
        {
            return;
        }
    }
    else
    {
        // The Avalonia client uses redirected pipes. Avoid blocking when launched manually.
        return;
    }

    char readBuffer[512];
    DWORD toRead = std::min<DWORD>(available, static_cast<DWORD>(sizeof(readBuffer)));
    DWORD bytesRead = 0;
    if (!ReadFile(m_stdin, readBuffer, toRead, &bytesRead, nullptr))
    {
        const DWORD error = GetLastError();
        if (error == ERROR_BROKEN_PIPE || error == ERROR_HANDLE_EOF)
        {
            m_shutdownRequested = true;
        }
        return;
    }

    if (bytesRead == 0)
    {
        return;
    }

    for (DWORD i = 0; i < bytesRead; ++i)
    {
        const char ch = readBuffer[i];
        if (ch == '\n' || ch == '\r')
        {
            if (m_inputPos > 0)
            {
                m_inputBuffer[m_inputPos] = '\0';
                ProcessLine(m_inputBuffer);
                m_inputPos = 0;
            }
        }
        else if (m_inputPos < static_cast<int>(sizeof(m_inputBuffer)) - 1)
        {
            m_inputBuffer[m_inputPos++] = ch;
        }
        else
        {
            m_inputPos = 0;
            SendError("IPC command exceeded input buffer");
        }
    }
}

void PreviewHost::ProcessLine(const char* line)
{
    const IpcCommandType commandType = IpcParser::ParseCommandType(line);
    if (!m_protocolReady
        && commandType != IpcCommandType::Init
        && commandType != IpcCommandType::Shutdown)
    {
        SendError("Preview host has not been initialized", line);
        return;
    }

    switch (commandType)
    {
    case IpcCommandType::Init:
    {
        IpcInitCommand command;
        if (IpcParser::ParseInit(line, command))
        {
            HandleInit(command);
        }
        else
        {
            SendError("Invalid init command", line);
        }
        break;
    }

    case IpcCommandType::Shutdown:
        HandleShutdown();
        break;

    case IpcCommandType::AttachWindow:
    {
        IpcAttachWindowCommand command;
        if (IpcParser::ParseAttachWindow(line, command))
        {
            HandleAttachWindow(command);
        }
        else
        {
            SendError("Invalid attachWindow command", line);
        }
        break;
    }

    case IpcCommandType::LoadEntity:
    {
        IpcLoadEntityCommand command;
        if (IpcParser::ParseLoadEntity(line, command))
        {
            HandleLoadEntity(command);
        }
        else
        {
            SendError("Invalid loadEntity command", line);
        }
        break;
    }

    case IpcCommandType::LoadParticle:
    {
        IpcLoadParticleCommand command;
        if (IpcParser::ParseLoadParticle(line, command))
        {
            HandleLoadParticle(command);
        }
        else
        {
            SendError("Invalid loadParticle command", line);
        }
        break;
    }

    case IpcCommandType::Tick:
    {
        IpcTickCommand command;
        if (IpcParser::ParseTick(line, command))
        {
            HandleTick(command);
        }
        else
        {
            SendError("Invalid tick command", line);
        }
        break;
    }

    case IpcCommandType::Pick:
    {
        IpcPickCommand command;
        if (IpcParser::ParsePick(line, command))
        {
            HandlePick(command);
        }
        else
        {
            SendError("Invalid pick command", line);
        }
        break;
    }

    case IpcCommandType::SetCameraPosition:
    {
        IpcSetCameraPositionCommand command;
        if (IpcParser::ParseSetCameraPosition(line, command))
        {
            HandleSetCameraPosition(command);
        }
        else
        {
            SendError("Invalid setCameraPosition command", line);
        }
        break;
    }

    case IpcCommandType::Unknown:
    default:
        SendError("Unknown command type", line);
        break;
    }
}

void PreviewHost::HandleInit(const IpcInitCommand& command)
{
    if (command.protocolVersion != IPC_PROTOCOL_VERSION)
    {
        char message[128];
        std::snprintf(message, sizeof(message), "Protocol version mismatch: expected %d, got %d",
            IPC_PROTOCOL_VERSION, command.protocolVersion);
        SendError(message);
        return;
    }

    m_protocolReady = true;
    m_initCommand = command;
    SendReady();
}

void PreviewHost::HandleAttachWindow(const IpcAttachWindowCommand& command)
{
    m_parentHwnd = reinterpret_cast<HWND>(static_cast<intptr_t>(command.hwnd));

    if (m_parentHwnd == nullptr || !IsWindow(m_parentHwnd))
    {
        SendError("attachWindow received an invalid HWND");
        return;
    }

    SetParent(m_hwnd, m_parentHwnd);
    SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, 0);
    ResizeHostedWindow(command.width, command.height);
    SetWindowPos(m_hwnd, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    ShowWindow(m_hwnd, SW_SHOW);

    char message[128];
    std::snprintf(message, sizeof(message), "Attached native preview window: %dx%d", command.width, command.height);
    SendDiagnostic("info", message);

    char error[512] = {};
    if (m_engine.Initialize(m_instance, m_hwnd, m_initCommand.romfilesPath, error, sizeof(error)))
    {
        SendDiagnostic("info", "Usagi engine initialized for preview host");
    }
    else
    {
        SendError("Failed to initialize Usagi preview engine", error);
    }
}

void PreviewHost::HandleLoadEntity(const IpcLoadEntityCommand& command)
{
    char error[512] = {};
    if (m_engine.LoadEntity(command.path, error, sizeof(error)))
    {
        SendLoaded("entity", command.path, true);
    }
    else
    {
        SendLoaded("entity", command.path, false, error);
    }
}

void PreviewHost::HandleLoadParticle(const IpcLoadParticleCommand& command)
{
    const char* path = command.effectPath[0] != '\0' ? command.effectPath : command.emitterPath;
    char error[512] = {};
    if (m_engine.LoadParticle(command.emitterPath, command.effectPath, error, sizeof(error)))
    {
        SendLoaded("particle", path, true);
    }
    else
    {
        SendLoaded("particle", path, false, error);
    }
}

void PreviewHost::HandleTick(const IpcTickCommand& command)
{
    if (!m_engine.IsInitialized())
    {
        SendDiagnostic("warning", "Tick ignored before preview engine initialization");
        return;
    }

    m_engine.Tick(command.deltaTime);
}

void PreviewHost::HandleSetCameraPosition(const IpcSetCameraPositionCommand& command)
{
    if (!m_engine.IsInitialized())
    {
        SendDiagnostic("warning", "Camera command ignored before preview engine initialization");
        return;
    }

    m_engine.SetCameraPosition(
        command.x,
        command.y,
        command.z,
        command.targetX,
        command.targetY,
        command.targetZ);
}

void PreviewHost::HandlePick(const IpcPickCommand& command)
{
    UNREFERENCED_PARAMETER(command);

    char response[128];
    IpcResponse::Picked(response, sizeof(response), nullptr, nullptr);
    SendResponse(response);
}

void PreviewHost::HandleShutdown()
{
    SendDiagnostic("info", "Shutdown requested");
    m_engine.Shutdown();
    m_shutdownRequested = true;
}

void PreviewHost::ResizeHostedWindow(int width, int height)
{
    if (m_hwnd == nullptr)
    {
        return;
    }

    MoveWindow(m_hwnd, 0, 0, std::max(width, 1), std::max(height, 1), TRUE);
    m_engine.Resize();
}

void PreviewHost::SendResponse(const char* json)
{
    if (m_stdout == INVALID_HANDLE_VALUE || m_stdout == nullptr)
    {
        return;
    }

    DWORD written = 0;
    WriteFile(m_stdout, json, static_cast<DWORD>(std::strlen(json)), &written, nullptr);
    WriteFile(m_stdout, "\n", 1, &written, nullptr);
}

void PreviewHost::SendReady()
{
    char buffer[256];
    IpcResponse::Ready(buffer, sizeof(buffer), IPC_PROTOCOL_VERSION, kEngineVersion);
    SendResponse(buffer);
}

void PreviewHost::SendError(const char* message, const char* details)
{
    char buffer[1024];
    IpcResponse::Error(buffer, sizeof(buffer), message, details);
    SendResponse(buffer);
}

void PreviewHost::SendLoaded(const char* resourceType, const char* path, bool success, const char* error)
{
    char buffer[1024];
    IpcResponse::Loaded(buffer, sizeof(buffer), resourceType, path, success, error);
    SendResponse(buffer);
}

void PreviewHost::SendDiagnostic(const char* level, const char* message, const char* source)
{
    char buffer[1024];
    IpcResponse::Diagnostic(buffer, sizeof(buffer), level, message, source);
    SendResponse(buffer);
}
