/****************************************************************************
//  Usagi preview host IPC protocol.
//  JSON-lines messages shared with Usagi.ToolCore.Preview.
****************************************************************************/
#pragma once

#ifndef USAGI_PREVIEW_HOST_IPC_PROTOCOL_H
#define USAGI_PREVIEW_HOST_IPC_PROTOCOL_H

#include <cstdint>

static constexpr int IPC_PROTOCOL_VERSION = 1;
static constexpr int IPC_MAX_PATH = 512;

enum class IpcCommandType
{
    Unknown,
    Init,
    Shutdown,
    AttachWindow,
    LoadEntity,
    LoadParticle,
    Tick,
    Pick,
    SetCameraPosition
};

struct IpcInitCommand
{
    int protocolVersion;
    char dataPath[IPC_MAX_PATH];
    char romfilesPath[IPC_MAX_PATH];
};

struct IpcAttachWindowCommand
{
    int64_t hwnd;
    int width;
    int height;
};

struct IpcLoadEntityCommand
{
    char path[IPC_MAX_PATH];
};

struct IpcLoadParticleCommand
{
    char emitterPath[IPC_MAX_PATH];
    char effectPath[IPC_MAX_PATH];
};

struct IpcTickCommand
{
    float deltaTime;
};

struct IpcPickCommand
{
    int x;
    int y;
};

struct IpcSetCameraPositionCommand
{
    float x;
    float y;
    float z;
    float targetX;
    float targetY;
    float targetZ;
};

class IpcParser
{
public:
    static IpcCommandType ParseCommandType(const char* json);
    static bool ParseInit(const char* json, IpcInitCommand& out);
    static bool ParseAttachWindow(const char* json, IpcAttachWindowCommand& out);
    static bool ParseLoadEntity(const char* json, IpcLoadEntityCommand& out);
    static bool ParseLoadParticle(const char* json, IpcLoadParticleCommand& out);
    static bool ParseTick(const char* json, IpcTickCommand& out);
    static bool ParsePick(const char* json, IpcPickCommand& out);
    static bool ParseSetCameraPosition(const char* json, IpcSetCameraPositionCommand& out);

private:
    static const char* FindString(const char* json, const char* key, char* buffer, int bufferSize);
    static bool FindInt(const char* json, const char* key, int& out);
    static bool FindInt64(const char* json, const char* key, int64_t& out);
    static bool FindFloat(const char* json, const char* key, float& out);
};

class IpcResponse
{
public:
    static void Ready(char* buffer, int bufferSize, int protocolVersion, const char* engineVersion);
    static void Error(char* buffer, int bufferSize, const char* message, const char* details = nullptr);
    static void Loaded(char* buffer, int bufferSize, const char* resourceType, const char* path, bool success, const char* error = nullptr);
    static void Picked(char* buffer, int bufferSize, const char* entityId, const char* componentName);
    static void Diagnostic(char* buffer, int bufferSize, const char* level, const char* message, const char* source = nullptr);
};

#endif
