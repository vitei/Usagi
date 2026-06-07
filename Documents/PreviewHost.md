# Usagi Preview Host

## Purpose

`UsagiPreviewHost.exe` is the native process that future Avalonia tools will use for embedded engine previews. The first checked-in version is intentionally a small Win32 scaffold, not an engine renderer. It establishes the process boundary, window-hosting contract, and JSON-lines IPC shape while leaving scene and particle rendering behind explicit TODO responses.

## Existing Pattern Survey

ParticleEditor hosts a native Win32 window in `Tools/Source/ParticleEditor/main/_win/WinMain.cpp`, registers that window with `Input::GetPlatform().RegisterHwnd`, then enters `GameMain`. The tool implementation is a `GameInterface` subclass in `ParticleEditor.cpp`.

The shared engine loop in `Engine/Game/GameMain.cpp` creates the game object through `CreateGame`, calls `PreGFXInit`, initializes the graphics display from `WINUTIL::GetWindow`, and drives `Update` and `Draw` while `GameInterface::IsRunning` remains true.

The historical Avalonia tool shell branches have a protocol/client shape under `Tools/Source/UsagiTools/src/Usagi.ToolCore/Preview`: they start `Tools/bin/UsagiPreviewHost.exe`, redirect stdin/stdout, send newline-delimited JSON commands, and wait for a `ready` response. This branch keeps that contract documented without adding managed `UsagiTools` projects.

## Current Scaffold

The current native host lives in `Tools/Source/UsagiPreviewHost` and builds from `project/UsagiPreviewHost.vcxproj` as a standalone x64 Win32 executable.

Implemented:

- Hidden Win32 preview window creation.
- Non-blocking polling of redirected stdin pipes.
- JSON-lines command parsing for protocol version 1.
- JSON responses compatible with `PreviewProtocol.cs`.
- `init` protocol validation and `ready` response.
- `attachWindow` reparenting of the native host window into the supplied parent HWND.
- Commands other than `init` and `shutdown` are rejected until protocol negotiation succeeds.
- Stubbed `loadEntity`, `loadParticle`, `tick`, `pick`, and camera command handling.
- Graceful shutdown on `shutdown` or window close.
- `Tools/Tests/PreviewHost/Run.ps1` build/headless smoke coverage for protocol startup, stub responses, picking, and shutdown.
- `Tools/Tests/PreviewHost/Run.ps1 -PreflightOnly` reports build, romfiles,
  model, particle, shader, and texture prerequisites without launching the host.

Not implemented:

- Engine `GameMain` integration.
- Graphics device creation against the Avalonia-hosted child window.
- Entity or particle resource loading.
- Camera state storage beyond accepting the command.
- Picking.

## Protocol

Commands are UTF-8 compatible JSON objects delimited by `\n`. The first command should be:

```json
{"type":"init","protocolVersion":1,"dataPath":"...","romfilesPath":"..."}
```

The host responds:

```json
{"type":"ready","protocolVersion":1,"engineVersion":"UsagiPreviewHost scaffold"}
```

After the Avalonia surface has a native handle, the client sends:

```json
{"type":"attachWindow","hwnd":123456,"width":800,"height":600}
```

The scaffold validates the HWND, calls `SetParent`, switches its window to `WS_CHILD | WS_VISIBLE`, resizes it, and emits a diagnostic response.

## Next Engine Step

The next slice should decide how to bridge the ParticleEditor/GameInterface pattern with an externally owned child window. The likely path is:

1. Extend or wrap the Win32 entry point so `WINUTIL::SetWindow` points at the attached child window before `GFX::Initialise` initializes the display.
2. Move the current IPC loop into a lightweight `GameInterface` implementation that processes commands in `Update`.
3. Add a minimal render path that clears the hosted window before loading entities or particles.
4. Only then wire `loadParticle` to the same particle systems used by ParticleEditor.

Keeping the first scaffold standalone avoids taking on renderer lifetime, OpenGL/Vulkan context ownership, and engine reset behavior before the Avalonia embedding contract is proven.

## Smoke Test Preflight

Run the preflight when setting up a clean checkout or reviewing preview changes:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools\Tests\PreviewHost\Run.ps1 -PreflightOnly
```

The report lists every prerequisite the full smoke needs, including generated
native projects, MSBuild, `UsagiPreviewHost.exe`, `_romfiles\win\nameDataHash.bin`,
source particle data, shader effects, texture data, `Ayataka.exe`, and the
PBRSample model source. Full smoke mode still fails on missing required
prerequisites, but it now prints the complete checklist first instead of failing
one missing file at a time.
