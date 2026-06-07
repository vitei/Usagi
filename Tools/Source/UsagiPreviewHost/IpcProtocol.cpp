/****************************************************************************
//  Usagi preview host IPC protocol implementation.
****************************************************************************/
#include "IpcProtocol.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

static const char* SkipWhitespace(const char* p)
{
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
    {
        ++p;
    }

    return p;
}

static const char* FindKey(const char* json, const char* key)
{
    char pattern[128];
    std::snprintf(pattern, sizeof(pattern), "\"%s\"", key);

    const char* found = std::strstr(json, pattern);
    if (found == nullptr)
    {
        return nullptr;
    }

    found += std::strlen(pattern);
    found = SkipWhitespace(found);
    if (*found != ':')
    {
        return nullptr;
    }

    return SkipWhitespace(found + 1);
}

const char* IpcParser::FindString(const char* json, const char* key, char* buffer, int bufferSize)
{
    if (bufferSize <= 0)
    {
        return nullptr;
    }

    const char* value = FindKey(json, key);
    if (value == nullptr)
    {
        return nullptr;
    }

    if (std::strncmp(value, "null", 4) == 0)
    {
        buffer[0] = '\0';
        return buffer;
    }

    if (*value != '"')
    {
        return nullptr;
    }

    ++value;

    int i = 0;
    while (*value != '\0' && *value != '"' && i < bufferSize - 1)
    {
        if (*value == '\\' && value[1] != '\0')
        {
            ++value;
            switch (*value)
            {
            case 'n':
                buffer[i++] = '\n';
                break;
            case 'r':
                buffer[i++] = '\r';
                break;
            case 't':
                buffer[i++] = '\t';
                break;
            case '"':
            case '\\':
                buffer[i++] = *value;
                break;
            default:
                buffer[i++] = *value;
                break;
            }
        }
        else
        {
            buffer[i++] = *value;
        }

        ++value;
    }

    buffer[i] = '\0';
    return buffer;
}

bool IpcParser::FindInt(const char* json, const char* key, int& out)
{
    const char* value = FindKey(json, key);
    if (value == nullptr)
    {
        return false;
    }

    char* end = nullptr;
    const long result = std::strtol(value, &end, 10);
    if (end == value)
    {
        return false;
    }

    out = static_cast<int>(result);
    return true;
}

bool IpcParser::FindInt64(const char* json, const char* key, int64_t& out)
{
    const char* value = FindKey(json, key);
    if (value == nullptr)
    {
        return false;
    }

    char* end = nullptr;
    const long long result = _strtoi64(value, &end, 10);
    if (end == value)
    {
        return false;
    }

    out = static_cast<int64_t>(result);
    return true;
}

bool IpcParser::FindFloat(const char* json, const char* key, float& out)
{
    const char* value = FindKey(json, key);
    if (value == nullptr)
    {
        return false;
    }

    char* end = nullptr;
    const float result = std::strtof(value, &end);
    if (end == value)
    {
        return false;
    }

    out = result;
    return true;
}

IpcCommandType IpcParser::ParseCommandType(const char* json)
{
    char type[64];
    if (FindString(json, "type", type, sizeof(type)) == nullptr)
    {
        return IpcCommandType::Unknown;
    }

    if (std::strcmp(type, "init") == 0) return IpcCommandType::Init;
    if (std::strcmp(type, "shutdown") == 0) return IpcCommandType::Shutdown;
    if (std::strcmp(type, "attachWindow") == 0) return IpcCommandType::AttachWindow;
    if (std::strcmp(type, "loadEntity") == 0) return IpcCommandType::LoadEntity;
    if (std::strcmp(type, "loadParticle") == 0) return IpcCommandType::LoadParticle;
    if (std::strcmp(type, "tick") == 0) return IpcCommandType::Tick;
    if (std::strcmp(type, "pick") == 0) return IpcCommandType::Pick;
    if (std::strcmp(type, "setCameraPosition") == 0) return IpcCommandType::SetCameraPosition;

    return IpcCommandType::Unknown;
}

bool IpcParser::ParseInit(const char* json, IpcInitCommand& out)
{
    std::memset(&out, 0, sizeof(out));
    if (!FindInt(json, "protocolVersion", out.protocolVersion))
    {
        return false;
    }

    FindString(json, "dataPath", out.dataPath, sizeof(out.dataPath));
    FindString(json, "romfilesPath", out.romfilesPath, sizeof(out.romfilesPath));
    return true;
}

bool IpcParser::ParseAttachWindow(const char* json, IpcAttachWindowCommand& out)
{
    std::memset(&out, 0, sizeof(out));
    if (!FindInt64(json, "hwnd", out.hwnd)) return false;
    if (!FindInt(json, "width", out.width)) out.width = 1;
    if (!FindInt(json, "height", out.height)) out.height = 1;
    return true;
}

bool IpcParser::ParseLoadEntity(const char* json, IpcLoadEntityCommand& out)
{
    std::memset(&out, 0, sizeof(out));
    return FindString(json, "path", out.path, sizeof(out.path)) != nullptr;
}

bool IpcParser::ParseLoadParticle(const char* json, IpcLoadParticleCommand& out)
{
    std::memset(&out, 0, sizeof(out));
    if (FindString(json, "emitterPath", out.emitterPath, sizeof(out.emitterPath)) == nullptr)
    {
        return false;
    }

    FindString(json, "effectPath", out.effectPath, sizeof(out.effectPath));
    return true;
}

bool IpcParser::ParseTick(const char* json, IpcTickCommand& out)
{
    std::memset(&out, 0, sizeof(out));
    out.deltaTime = 1.0f / 60.0f;
    FindFloat(json, "deltaTime", out.deltaTime);
    return true;
}

bool IpcParser::ParsePick(const char* json, IpcPickCommand& out)
{
    std::memset(&out, 0, sizeof(out));
    return FindInt(json, "x", out.x) && FindInt(json, "y", out.y);
}

bool IpcParser::ParseSetCameraPosition(const char* json, IpcSetCameraPositionCommand& out)
{
    std::memset(&out, 0, sizeof(out));
    return FindFloat(json, "x", out.x)
        && FindFloat(json, "y", out.y)
        && FindFloat(json, "z", out.z)
        && FindFloat(json, "targetX", out.targetX)
        && FindFloat(json, "targetY", out.targetY)
        && FindFloat(json, "targetZ", out.targetZ);
}

static void EscapeJsonString(const char* input, char* output, int outputSize)
{
    int j = 0;
    const char* safeInput = input != nullptr ? input : "";

    for (int i = 0; safeInput[i] != '\0' && j < outputSize - 1; ++i)
    {
        const char c = safeInput[i];
        const char* replacement = nullptr;
        char escaped[3] = {};

        switch (c)
        {
        case '"': replacement = "\\\""; break;
        case '\\': replacement = "\\\\"; break;
        case '\n': replacement = "\\n"; break;
        case '\r': replacement = "\\r"; break;
        case '\t': replacement = "\\t"; break;
        default:
            escaped[0] = c;
            replacement = escaped;
            break;
        }

        for (int k = 0; replacement[k] != '\0' && j < outputSize - 1; ++k)
        {
            output[j++] = replacement[k];
        }
    }

    output[j] = '\0';
}

void IpcResponse::Ready(char* buffer, int bufferSize, int protocolVersion, const char* engineVersion)
{
    char escapedVersion[128];
    EscapeJsonString(engineVersion, escapedVersion, sizeof(escapedVersion));

    std::snprintf(buffer, bufferSize,
        "{\"type\":\"ready\",\"protocolVersion\":%d,\"engineVersion\":\"%s\"}",
        protocolVersion, escapedVersion);
}

void IpcResponse::Error(char* buffer, int bufferSize, const char* message, const char* details)
{
    char escapedMessage[512];
    EscapeJsonString(message, escapedMessage, sizeof(escapedMessage));

    if (details != nullptr)
    {
        char escapedDetails[512];
        EscapeJsonString(details, escapedDetails, sizeof(escapedDetails));
        std::snprintf(buffer, bufferSize,
            "{\"type\":\"error\",\"message\":\"%s\",\"details\":\"%s\"}",
            escapedMessage, escapedDetails);
    }
    else
    {
        std::snprintf(buffer, bufferSize,
            "{\"type\":\"error\",\"message\":\"%s\"}",
            escapedMessage);
    }
}

void IpcResponse::Loaded(char* buffer, int bufferSize, const char* resourceType, const char* path, bool success, const char* error)
{
    char escapedType[64];
    char escapedPath[IPC_MAX_PATH];
    EscapeJsonString(resourceType, escapedType, sizeof(escapedType));
    EscapeJsonString(path, escapedPath, sizeof(escapedPath));

    if (error != nullptr)
    {
        char escapedError[512];
        EscapeJsonString(error, escapedError, sizeof(escapedError));
        std::snprintf(buffer, bufferSize,
            "{\"type\":\"loaded\",\"resourceType\":\"%s\",\"path\":\"%s\",\"success\":%s,\"error\":\"%s\"}",
            escapedType, escapedPath, success ? "true" : "false", escapedError);
    }
    else
    {
        std::snprintf(buffer, bufferSize,
            "{\"type\":\"loaded\",\"resourceType\":\"%s\",\"path\":\"%s\",\"success\":%s}",
            escapedType, escapedPath, success ? "true" : "false");
    }
}

void IpcResponse::Picked(char* buffer, int bufferSize, const char* entityId, const char* componentName)
{
    if (entityId == nullptr)
    {
        std::snprintf(buffer, bufferSize, "{\"type\":\"picked\",\"entityId\":null}");
        return;
    }

    char escapedId[256];
    EscapeJsonString(entityId, escapedId, sizeof(escapedId));

    if (componentName != nullptr)
    {
        char escapedComponent[256];
        EscapeJsonString(componentName, escapedComponent, sizeof(escapedComponent));
        std::snprintf(buffer, bufferSize,
            "{\"type\":\"picked\",\"entityId\":\"%s\",\"componentName\":\"%s\"}",
            escapedId, escapedComponent);
    }
    else
    {
        std::snprintf(buffer, bufferSize,
            "{\"type\":\"picked\",\"entityId\":\"%s\",\"componentName\":null}",
            escapedId);
    }
}

void IpcResponse::Diagnostic(char* buffer, int bufferSize, const char* level, const char* message, const char* source)
{
    char escapedLevel[32];
    char escapedMessage[1024];
    EscapeJsonString(level, escapedLevel, sizeof(escapedLevel));
    EscapeJsonString(message, escapedMessage, sizeof(escapedMessage));

    if (source != nullptr)
    {
        char escapedSource[256];
        EscapeJsonString(source, escapedSource, sizeof(escapedSource));
        std::snprintf(buffer, bufferSize,
            "{\"type\":\"diagnostic\",\"level\":\"%s\",\"message\":\"%s\",\"source\":\"%s\"}",
            escapedLevel, escapedMessage, escapedSource);
    }
    else
    {
        std::snprintf(buffer, bufferSize,
            "{\"type\":\"diagnostic\",\"level\":\"%s\",\"message\":\"%s\"}",
            escapedLevel, escapedMessage);
    }
}
