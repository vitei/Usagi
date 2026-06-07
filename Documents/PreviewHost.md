# Usagi Preview Host

## Purpose

`UsagiPreviewHost.exe` is the native process that future Avalonia tools will use
for embedded engine previews. It owns a child Win32 window, speaks JSON-lines IPC
to the managed tools, and now bootstraps the real Usagi engine against the
attached preview HWND.

## Existing Pattern Survey

ParticleEditor hosts a native Win32 window in `Tools/Source/ParticleEditor/main/_win/WinMain.cpp`, registers that window with `Input::GetPlatform().RegisterHwnd`, then enters `GameMain`. The tool implementation is a `GameInterface` subclass in `ParticleEditor.cpp`.

The shared engine loop in `Engine/Game/GameMain.cpp` creates the game object through `CreateGame`, calls `PreGFXInit`, initializes the graphics display from `WINUTIL::GetWindow`, and drives `Update` and `Draw` while `GameInterface::IsRunning` remains true.

The Avalonia tool shell already has a protocol/client shape under
`Tools/Source/UsagiTools/src/Usagi.ToolCore/Preview`: it starts
`Tools/bin/UsagiPreviewHost.exe`, redirects stdin/stdout, sends
newline-delimited JSON commands, and waits for a `ready` response.

## Current Scaffold

The current native host lives in `Tools/Source/UsagiPreviewHost` and builds from
`project/UsagiPreviewHost.vcxproj` as an x64 Win32 executable linked against the
engine libraries.

Implemented:

- Hidden Win32 preview window creation.
- Non-blocking polling of redirected stdin pipes.
- JSON-lines command parsing for protocol version 1.
- JSON responses compatible with `PreviewProtocol.cs`.
- `init` protocol validation and `ready` response.
- `attachWindow` reparenting of the native host window into the supplied parent HWND.
- Engine bootstrap through `PreviewEngineBridge` after `attachWindow`.
- A minimal `GameInterface` implementation that clears/presents the attached
  display on `tick`.
- Resize forwarding to the engine display.
- Stubbed `loadEntity`, `loadParticle`, `pick`, and camera command handling.
- Graceful shutdown on `shutdown` or window close.
- `Tools/Tests/PreviewHost/Run.ps1` smoke coverage for `init`, `attachWindow`,
  engine initialization, one `tick`, and `shutdown`.

Not implemented:

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

The host validates the HWND, calls `SetParent`, switches its window to
`WS_CHILD | WS_VISIBLE`, resizes it, initializes the engine against that child
window, and emits diagnostics for both attach and engine initialization.

## Next Engine Step

The next slice should load one real previewable resource through the engine host.
The recommended order is:

1. Move particle preview state into the preview `GameInterface`.
2. Wire `loadParticle` to load a `.pem`/`.pfx` pair and instantiate the same
   runtime particle classes used by `ParticleEditor`.
3. Add a visual smoke path that ticks long enough to prove the hosted window is
   not blank.
4. After particles work, add entity/resource loading and picking.

Engine initialization currently requires a valid `nameDataHash.bin` under the
runtime `_romfiles\win` directory, matching normal game/editor launch behavior.
