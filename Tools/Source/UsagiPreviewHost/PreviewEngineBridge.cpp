/****************************************************************************
//  Usagi engine bridge for the native preview host.
****************************************************************************/
#include "PreviewEngineBridge.h"

#include "Engine/Common/Common.h"
#include "Engine/Core/_win/WinUtil.h"
#include "Engine/Core/Modules/ModuleManager.h"
#include "Engine/Game/GameInterface.h"
#include "Engine/Graphics/Device/Display.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Textures/RenderTarget.h"
#include "Engine/HID/Input.h"
#include "Engine/Maths/AABB.h"
#include "Engine/Particles/ParticleEffect.h"
#include "Engine/Particles/ParticleEffectHndl.h"
#include "Engine/Particles/Scripted/ScriptEmitter.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/Camera/StandardCamera.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Graphics/Lights/DirLight.h"
#include "Engine/Graphics/Lights/LightMgr.h"
#include OS_HEADER(Engine/HID, Input_ps.h)

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>

namespace usg
{
bool InitEngine(const char** dllModules, uint32 uModuleCount);
bool GameInit();
void GameLoop();
void GameCleanup();
void GameMessage(const uint32 messageID, const void* const pParameters);
GFXDevice* GetGFXDevice();
}

namespace
{
static const float kDefaultDeltaTime = 1.0f / 60.0f;
class PreviewGame;
static PreviewGame* g_previewGame = nullptr;

bool FileExists(const char* path)
{
    const DWORD attributes = GetFileAttributesA(path);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

void CopyResourceName(const char* path, char* out, int outSize)
{
    if (outSize <= 0)
    {
        return;
    }

    out[0] = '\0';
    if (path == nullptr || path[0] == '\0')
    {
        return;
    }

    const char* nameStart = path;
    for (const char* cursor = path; *cursor != '\0'; ++cursor)
    {
        if (*cursor == '\\' || *cursor == '/')
        {
            nameStart = cursor + 1;
        }
    }

    std::snprintf(out, outSize, "%s", nameStart);

    for (char* cursor = out; *cursor != '\0'; ++cursor)
    {
        if (*cursor == '.')
        {
            *cursor = '\0';
            break;
        }
    }
}

bool ValidateCompiledParticleFile(const char* resourceName, const char* extension, char* error, int errorSize)
{
    char relativePath[MAX_PATH] = {};
    std::snprintf(relativePath, sizeof(relativePath), "Particle/%s.%s", resourceName, extension);

    if (FileExists(relativePath))
    {
        return true;
    }

    std::snprintf(error, errorSize, "Compiled particle resource not found: %s", relativePath);
    return false;
}

void NormalizeSlashes(char* path)
{
    for (char* cursor = path; *cursor != '\0'; ++cursor)
    {
        if (*cursor == '\\')
        {
            *cursor = '/';
        }
    }
}

bool EndsWithInsensitive(const char* value, const char* suffix)
{
    const size_t valueLen = std::strlen(value);
    const size_t suffixLen = std::strlen(suffix);
    if (valueLen < suffixLen)
    {
        return false;
    }

    const char* valueSuffix = value + valueLen - suffixLen;
    for (size_t i = 0; i < suffixLen; ++i)
    {
        if (std::tolower(static_cast<unsigned char>(valueSuffix[i])) != std::tolower(static_cast<unsigned char>(suffix[i])))
        {
            return false;
        }
    }

    return true;
}

void TrimAscii(char* value)
{
    char* start = value;
    while (*start != '\0' && std::isspace(static_cast<unsigned char>(*start)))
    {
        ++start;
    }

    char* end = start + std::strlen(start);
    while (end > start && std::isspace(static_cast<unsigned char>(end[-1])))
    {
        --end;
    }

    *end = '\0';
    if (start != value)
    {
        std::memmove(value, start, std::strlen(start) + 1);
    }
}

bool ExtractModelResourceName(const char* path, char* out, int outSize)
{
    if (path == nullptr || path[0] == '\0' || outSize <= 0)
    {
        return false;
    }

    std::snprintf(out, outSize, "%s", path);
    NormalizeSlashes(out);

    const char* marker = std::strstr(out, "Data/Models/");
    if (marker != nullptr)
    {
        std::memmove(out, marker + std::strlen("Data/Models/"), std::strlen(marker + std::strlen("Data/Models/")) + 1);
    }

    marker = std::strstr(out, "Models/");
    if (marker == out)
    {
        std::memmove(out, out + std::strlen("Models/"), std::strlen(out + std::strlen("Models/")) + 1);
    }

    TrimAscii(out);
    return out[0] != '\0';
}

bool ParseEntityModelName(const char* path, char* out, int outSize, char* error, int errorSize)
{
    FILE* file = nullptr;
    fopen_s(&file, path, "rb");
    if (file == nullptr)
    {
        std::snprintf(error, errorSize, "Could not open entity YAML: %s", path);
        return false;
    }

    bool inModelComponent = false;
    char line[1024] = {};
    while (std::fgets(line, sizeof(line), file) != nullptr)
    {
        char* cursor = line;
        while (*cursor == ' ' || *cursor == '\t')
        {
            ++cursor;
        }

        if (std::strncmp(cursor, "ModelComponent:", 15) == 0)
        {
            inModelComponent = true;
            continue;
        }

        if (inModelComponent && cursor == line && cursor[0] != '\0' && cursor[0] != '\r' && cursor[0] != '\n')
        {
            inModelComponent = false;
        }

        if (inModelComponent)
        {
            char* name = std::strstr(cursor, "name:");
            if (name != nullptr)
            {
                name += 5;
                std::snprintf(out, outSize, "%s", name);
                TrimAscii(out);
                NormalizeSlashes(out);
                fclose(file);
                return out[0] != '\0';
            }
        }
    }

    fclose(file);
    std::snprintf(error, errorSize, "Entity YAML does not contain a direct ModelComponent.name: %s", path);
    return false;
}

bool ValidateCompiledModelFile(const char* modelName, char* error, int errorSize)
{
    char relativePath[MAX_PATH] = {};
    std::snprintf(relativePath, sizeof(relativePath), "Models/%s", modelName);

    if (FileExists(relativePath))
    {
        return true;
    }

    std::snprintf(error, errorSize, "Compiled model resource not found: %s", relativePath);
    return false;
}

class PreviewGame final : public usg::GameInterface
{
public:
    PreviewGame()
        : m_viewContext(nullptr)
        , m_keyLight(nullptr)
        , m_previewModel(nullptr)
        , m_sceneInitialized(false)
        , m_postFxInitialized(false)
        , m_singleEmitterAllocated(false)
        , m_pendingDeltaTime(kDefaultDeltaTime)
    {
        m_activeEffectMatrix.LoadIdentity();
    }

    void Init(usg::GFXDevice* pDevice, usg::ResourceMgr* pResMgr) override
    {
        UNREFERENCED_PARAMETER(pResMgr);

        usg::Input::Init();
        InitScene(pDevice);
        m_bIsRunning = true;
    }

    void Cleanup(usg::GFXDevice* pDevice) override
    {
        if (pDevice != nullptr)
        {
            pDevice->WaitIdle();
        }

        ClearParticle(pDevice);
        ClearModel(pDevice);

        if (m_sceneInitialized)
        {
            if (m_keyLight != nullptr)
            {
                m_scene.GetLightMgr().RemoveDirLight(m_keyLight);
                m_keyLight = nullptr;
            }

            if (m_viewContext != nullptr)
            {
                m_scene.DeleteViewContext(m_viewContext);
                m_viewContext = nullptr;
            }

            m_scene.Cleanup(pDevice);
            m_sceneInitialized = false;
        }

        if (m_postFxInitialized)
        {
            m_postFx.Cleanup(pDevice);
            m_postFxInitialized = false;
        }
    }

    void Update(usg::GFXDevice* pDevice) override
    {
        if (!m_sceneInitialized)
        {
            return;
        }

        m_scene.TransformUpdate(m_pendingDeltaTime);
        if (m_previewModel != nullptr)
        {
            m_previewModel->GPUUpdate(pDevice);
        }
        m_scene.Update(pDevice);
        m_postFx.Update(&m_scene, m_pendingDeltaTime);
        m_postFx.UpdateGPU(pDevice);
        m_pendingDeltaTime = kDefaultDeltaTime;
    }

    void Draw(usg::GFXDevice* pDevice) override
    {
        if (pDevice == nullptr)
        {
            return;
        }

        usg::Display* display = pDevice->GetDisplay(0);
        if (display == nullptr)
        {
            return;
        }

        pDevice->Begin();
        usg::GFXContext* context = pDevice->GetImmediateCtxt();
        context->Begin(true);

        if (m_sceneInitialized)
        {
            context->ApplyDefaults();
            m_scene.PreDraw(context);

            m_postFx.BeginScene(context, usg::PostFXSys::TRANSFER_FLAGS_CLEAR);
            m_postFx.SetActiveViewContext(m_viewContext);
            m_viewContext->PreDraw(context, usg::VIEW_CENTRAL);
            m_viewContext->DrawScene(context);
            m_postFx.SetActiveViewContext(nullptr);

            context->Transfer(m_postFx.GetFinalRT(), display);
            m_postFx.EndScene();
        }

        context->RenderToDisplay(display, usg::RenderTarget::RT_FLAG_COLOR_0 | usg::RenderTarget::RT_FLAG_DEPTH);
        display->Present();
        context->End();
        pDevice->End();
    }

    void OnMessage(usg::GFXDevice* const pDevice, const uint32 messageID, const void* const pParameters) override
    {
        UNREFERENCED_PARAMETER(pParameters);

        if (pDevice == nullptr)
        {
            return;
        }

        switch (messageID)
        {
        case 'ONSZ':
            pDevice->WaitIdle();
            break;
        case 'WSZE':
            if (usg::Display* display = pDevice->GetDisplay(0))
            {
                display->Resize(pDevice);
                if (m_postFxInitialized)
                {
                    m_postFx.UpdateRTSize(pDevice, display);
                    uint32 width = 0;
                    uint32 height = 0;
                    display->GetDisplayDimensions(width, height, false);
                    SetCameraPosition(0.0f, 0.0f, 25.0f, 0.0f, 0.0f, 0.0f, width, height);
                }
            }
            break;
        case 'WMIN':
            if (usg::Display* display = pDevice->GetDisplay(0))
            {
                display->Minimized(pDevice);
            }
            break;
        default:
            break;
        }
    }

    bool LoadEntity(usg::GFXDevice* pDevice, const char* path, char* error, int errorSize)
    {
        if (!m_sceneInitialized)
        {
            std::snprintf(error, errorSize, "Preview scene is not initialized.");
            return false;
        }

        char modelName[MAX_PATH] = {};
        if (EndsWithInsensitive(path, ".yml") || EndsWithInsensitive(path, ".yaml"))
        {
            if (!ParseEntityModelName(path, modelName, sizeof(modelName), error, errorSize))
            {
                return false;
            }
        }
        else if (!ExtractModelResourceName(path, modelName, sizeof(modelName)))
        {
            std::snprintf(error, errorSize, "Entity preview requires an entity YAML or model resource path.");
            return false;
        }

        if (!ValidateCompiledModelFile(modelName, error, errorSize))
        {
            return false;
        }

        ClearParticle(pDevice);
        ClearModel(pDevice);

        m_previewModel = vnew(usg::ALLOC_OBJECT) usg::Model();
        if (!m_previewModel->Load(pDevice, &m_scene, usg::ResourceMgr::Inst(), modelName, false, true, true, true))
        {
            vdelete m_previewModel;
            m_previewModel = nullptr;
            std::snprintf(error, errorSize, "Failed to load model resource: %s", modelName);
            return false;
        }

        usg::Matrix4x4 modelMatrix;
        modelMatrix.LoadIdentity();
        m_previewModel->SetTransform(modelMatrix);
        m_previewModel->GPUUpdate(pDevice);

        const usg::Sphere& bounds = m_previewModel->GetResource()->GetBounds();
        const float radius = std::max(bounds.GetRadius(), 1.0f);
        const usg::Vector3f center = bounds.GetPos();
        SetCameraPosition(
            center.x,
            center.y + radius * 0.35f,
            center.z + radius * 3.5f,
            center.x,
            center.y,
            center.z);
        return true;
    }

    bool LoadParticle(usg::GFXDevice* pDevice, const char* emitterPath, const char* effectPath, char* error, int errorSize)
    {
        if (!m_sceneInitialized)
        {
            std::snprintf(error, errorSize, "Preview scene is not initialized.");
            return false;
        }

        char emitterName[MAX_PATH] = {};
        char effectName[MAX_PATH] = {};
        CopyResourceName(emitterPath, emitterName, sizeof(emitterName));
        CopyResourceName(effectPath, effectName, sizeof(effectName));

        if (effectName[0] != '\0')
        {
            if (!ValidateCompiledParticleFile(effectName, "pfx", error, errorSize))
            {
                return false;
            }

            ClearModel(pDevice);
            ClearParticle(pDevice);

            usg::Matrix4x4 matrix;
            matrix.LoadIdentity();
            m_scene.CreateScriptedEffect(m_activeEffect, matrix, effectName);
            m_activeEffectMatrix = matrix;
            return true;
        }

        if (emitterName[0] == '\0')
        {
            std::snprintf(error, errorSize, "Particle load requires an emitter or effect path.");
            return false;
        }

        if (!ValidateCompiledParticleFile(emitterName, "pem", error, errorSize))
        {
            return false;
        }

        ClearParticle(pDevice);
        ClearModel(pDevice);

        usg::Matrix4x4 matrix;
        matrix.LoadIdentity();
        m_singleEmitterEffect.Init(pDevice, &m_scene, matrix);
        m_singleEmitter.Alloc(pDevice, &m_scene.GetParticleMgr(), emitterName, true);
        m_singleEmitter.SetInstanceData(matrix, 1.0f, 0.0f, 0);
        m_singleEmitterEffect.AddEmitter(pDevice, &m_singleEmitter);
        m_singleEmitterAllocated = true;
        m_activeEffectMatrix = matrix;
        return true;
    }

    void SetDeltaTime(float deltaTime)
    {
        if (deltaTime > 0.0f)
        {
            m_pendingDeltaTime = deltaTime;
        }
    }

    void SetCameraPosition(
        float x,
        float y,
        float z,
        float targetX,
        float targetY,
        float targetZ,
        uint32 width = 0,
        uint32 height = 0)
    {
        if (!m_postFxInitialized)
        {
            return;
        }

        if (width == 0 || height == 0)
        {
            width = m_postFx.GetFinalTargetWidth(false);
            height = m_postFx.GetFinalTargetHeight(false);
        }

        const float aspect = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
        usg::Matrix4x4 cameraMatrix;
        cameraMatrix.LookAt(usg::Vector3f(x, y, z), usg::Vector3f(targetX, targetY, targetZ), usg::Vector3f::Y_AXIS);
        m_camera.SetUp(cameraMatrix, aspect, 60.0f, 1.0f, 500.0f);
    }

private:
    void InitScene(usg::GFXDevice* pDevice)
    {
        if (pDevice == nullptr || m_sceneInitialized)
        {
            return;
        }

        usg::Display* display = pDevice->GetDisplay(0);
        uint32 width = 640;
        uint32 height = 480;
        if (display != nullptr)
        {
            display->GetDisplayDimensions(width, height, false);
        }
        if (width == 0)
        {
            width = 640;
        }
        if (height == 0)
        {
            height = 480;
        }

        m_postFx.Init(pDevice, usg::ResourceMgr::Inst(), width, height, usg::PostFXSys::EFFECT_OFFSCREEN_TARGET);
        m_postFxInitialized = true;

        usg::AABB worldBounds;
        worldBounds.SetCentreRadii(usg::Vector3f(0.0f, 0.0f, 0.0f), usg::Vector3f(512.0f, 512.0f, 512.0f));
        m_scene.Init(pDevice, usg::ResourceMgr::Inst(), worldBounds, nullptr);
        m_viewContext = m_scene.CreateViewContext(pDevice);
        m_viewContext->Init(pDevice, usg::ResourceMgr::Inst(), &m_postFx, 0, usg::RenderMask::RENDER_MASK_ALL);
        m_viewContext->SetCamera(&m_camera);
        m_keyLight = m_scene.GetLightMgr().AddDirectionalLight(pDevice, false, "PreviewKeyLight");
        m_keyLight->SetDirection(usg::Vector4f(-0.4f, -0.8f, -0.35f, 0.0f));
        m_keyLight->SetDiffuse(usg::Color(1.0f, 1.0f, 1.0f, 1.0f));
        m_keyLight->SetAmbient(usg::Color(0.25f, 0.25f, 0.25f, 1.0f));
        m_keyLight->SetSpecularColor(usg::Color(0.8f, 0.8f, 0.8f, 1.0f));
        SetCameraPosition(0.0f, 0.0f, 25.0f, 0.0f, 0.0f, 0.0f, width, height);
        m_sceneInitialized = true;
    }

    void ClearParticle(usg::GFXDevice* pDevice)
    {
        if (pDevice != nullptr)
        {
            pDevice->WaitIdle();
        }

        m_activeEffect.Kill(true);

        if (m_singleEmitterAllocated)
        {
            m_singleEmitterEffect.Kill(true);
            m_singleEmitter.Cleanup(pDevice);
            m_singleEmitterAllocated = false;
        }
    }

    void ClearModel(usg::GFXDevice* pDevice)
    {
        if (m_previewModel == nullptr)
        {
            return;
        }

        if (pDevice != nullptr)
        {
            pDevice->WaitIdle();
        }

        m_previewModel->ForceRemoveFromScene();
        m_previewModel->Cleanup(pDevice);
        vdelete m_previewModel;
        m_previewModel = nullptr;
    }

    usg::Scene m_scene;
    usg::PostFXSys m_postFx;
    usg::ViewContext* m_viewContext;
    usg::DirLight* m_keyLight;
    usg::StandardCamera m_camera;
    usg::Model* m_previewModel;
    usg::ParticleEffectHndl m_activeEffect;
    usg::ParticleEffect m_singleEmitterEffect;
    usg::ScriptEmitter m_singleEmitter;
    usg::Matrix4x4 m_activeEffectMatrix;
    bool m_sceneInitialized;
    bool m_postFxInitialized;
    bool m_singleEmitterAllocated;
    float m_pendingDeltaTime;
};
}

usg::GameInterface* usg::CreateGame()
{
    g_previewGame = vnew(usg::ALLOC_OBJECT) PreviewGame();
    return g_previewGame;
}

const char* usg::GetGameName()
{
    return "Usagi Preview Host";
}

PreviewEngineBridge::PreviewEngineBridge()
    : m_initialized(false)
{
}

PreviewEngineBridge::~PreviewEngineBridge()
{
    Shutdown();
}

bool PreviewEngineBridge::Initialize(HINSTANCE instance, HWND hwnd, const char* romfilesPath, char* error, int errorSize)
{
    if (m_initialized)
    {
        return true;
    }

    if (hwnd == nullptr || !IsWindow(hwnd))
    {
        std::snprintf(error, errorSize, "Preview engine received an invalid HWND.");
        return false;
    }

    WINUTIL::Init(instance);
    WINUTIL::SetWindow(hwnd);
    usg::Input::GetPlatform().RegisterHwnd(0, hwnd);

    if (romfilesPath != nullptr && romfilesPath[0] != '\0' && !SetCurrentDirectoryA(romfilesPath))
    {
        std::snprintf(error, errorSize, "Failed to set preview working directory: %s", romfilesPath);
        return false;
    }

    if (!usg::InitEngine(nullptr, 0))
    {
        std::snprintf(error, errorSize, "Usagi engine initialization failed.");
        return false;
    }

    if (!usg::GameInit())
    {
        std::snprintf(error, errorSize, "Usagi preview game initialization failed.");
        usg::GameCleanup();
        return false;
    }

    usg::ModuleManager::Inst()->PostInit(usg::GetGFXDevice());
    m_initialized = true;
    return true;
}

bool PreviewEngineBridge::LoadEntity(const char* path, char* error, int errorSize)
{
    if (!m_initialized || g_previewGame == nullptr)
    {
        std::snprintf(error, errorSize, "Preview engine is not initialized.");
        return false;
    }

    return g_previewGame->LoadEntity(usg::GetGFXDevice(), path, error, errorSize);
}

bool PreviewEngineBridge::LoadParticle(const char* emitterPath, const char* effectPath, char* error, int errorSize)
{
    if (!m_initialized || g_previewGame == nullptr)
    {
        std::snprintf(error, errorSize, "Preview engine is not initialized.");
        return false;
    }

    return g_previewGame->LoadParticle(usg::GetGFXDevice(), emitterPath, effectPath, error, errorSize);
}

void PreviewEngineBridge::Tick(float deltaTime)
{
    if (m_initialized)
    {
        if (g_previewGame != nullptr)
        {
            g_previewGame->SetDeltaTime(deltaTime);
        }

        usg::GameLoop();
    }
}

void PreviewEngineBridge::SetCameraPosition(float x, float y, float z, float targetX, float targetY, float targetZ)
{
    if (m_initialized && g_previewGame != nullptr)
    {
        g_previewGame->SetCameraPosition(x, y, z, targetX, targetY, targetZ);
    }
}

void PreviewEngineBridge::Resize()
{
    if (m_initialized)
    {
        usg::GameMessage('ONSZ', nullptr);
        usg::GameMessage('WSZE', nullptr);
    }
}

void PreviewEngineBridge::Shutdown()
{
    if (m_initialized)
    {
        usg::GameCleanup();
        g_previewGame = nullptr;
        m_initialized = false;
    }
}
