# After cloning — quick setup

1. Clone the repository.
2. Open the solution in Visual Studio 2022.
3. Build `Release` for the desired platform (`x64` recommended).
4. Run `deploy_release.bat` from the repository root:
   - Usage: `deploy_release.bat [path-to-built-exe] [optional-target-release-folder]`
   - If you omit the exe path, the script will try to auto-find a Release exe in the usual output locations.
5. The script copies the exe, shaders (`*.hlsl`), and these folders (if present) into the release folder:
   - `DirectX Game\Images`
   - `DirectX Game\Meshes`
   - `DirectX Game\ImGui` (optional)
   - `DirectX Game\Libs` (optional)
6. The default release destination is: `C:\Users\USER\Dropbox\PC\Documents\GitHub\HO_Lapuz_FinalExam\Release` (repo-root `Release` folder).

## Files that must sit next to the `.exe`
- All `.hlsl` shader files (the engine compiles shaders at runtime): `VertexShader.hlsl`, `PixelShader.hlsl`.
- Runtime assets used by the game (textures, meshes, config) — preserve expected relative paths (e.g., `Images`, `Meshes`).
- Optional: `d3dcompiler_47.dll` if shipping the compiler with the build.
- If using DLL CRT (`/MD`), ensure the Visual C++ Redistributable is available on target machines (or build with `/MT` to avoid it).

## Release build checklist (verify before shipping)
- Project → Properties → General
  - `Platform Toolset` = Visual Studio 2022 (v143)
  - `Configuration Type` = Application (.exe)
- C/C++ → Language
  - `C++ Language Standard` = ISO C++14 (project default)
- C/C++ → Code Generation
  - `Runtime Library` = Multi-threaded DLL (/MD) for Release (or /MT if you choose static CRT)
- Linker → Input
  - Ensure required libs are linked (project uses pragmas for `d3d11.lib`, `d3dcompiler.lib`, `dxgi.lib` in `RenderSystem.cpp`).
- Linker → General
  - `Additional Library Directories` should include third-party Release lib folders (for example `DirectX Game\Libs\reactphysics3d\Release\lib`).

## Unity importer (quick)
- File to add: `Assets/Editor/LevelImporter.cs` (Editor folder required).
- Requirements: Newtonsoft.Json (install via Unity Package Manager or put `Newtonsoft.Json.dll` in `Assets/Plugins`).
- Usage in Editor:
  1. Place `LevelImporter.cs` in `Assets/Editor/`.
  2. Editor → Tools → Import Level... → choose your `.level` file (for example `DirectX Game/Level/TestCases1.level`).
  3. The importer creates a root GameObject and instantiates primitives or mapped prefabs, attaches `Rigidbody`/`BoxCollider` and sets properties (mass, collider size).
- Notes:
  - Rotation: importer expects Euler degrees. If your exported rotations are in radians, enable the radians→degrees conversion (script includes a flag).
  - For custom meshes/prefabs: extend the `prefabMap` in the script to point to prefabs (Resources or a ScriptableObject mapping).

## Unreal importer (quick)
- Place the importer under your Unreal project: `Content/Python/Tools/import_level.py`.
  - Make `Tools` a Python package (optional) by adding an empty `__init__.py` if you plan to import via `Tools.import_level`.
- Enable plugins in the Editor (one-time): Edit → Plugins → enable `Python Editor Script Plugin` and `Editor Scripting Utilities` and restart the Editor.
- In‑Editor usage (fast test): Window → Developer Tools → Python Console, then run:
  import importlib
  import Tools.import_level as il
  importlib.reload(il)
  il.import_level(r"C:\full\path\to\TestCases1.level", rotations_in_radians=True)
- Editor Utility option (optional): create an Editor Utility Widget that runs the Python import call (the widget can expose a file path field + Import button). The importer also supports running via the Editor CLI for automation.
- Portable asset references: use `/Game/...` asset paths (for example `/Game/StarterContent/Shapes/Shape_NarrowCapsule`) so the importer remains project‑relative.

## Running Unreal importer headless / from a script
- Example command (adjust engine and project paths):
  "C:\Program Files\Epic Games\UE_5.xx\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Path\To\YourProject\YourProject.uproject" -run=pythonscript -script="C:/Path/To/YourProject/Content/Python/Tools/run_import.py" "C:/full/path/TestCases1.level"
- `run_import.py` should call `Tools.import_level.import_level(path)`; this launches the Editor and executes the import script.

## Quick troubleshooting
- JSON parse errors: ensure the `.level` file is valid JSON (SceneEditor provides a validator/diagnostic when a file is truncated).
- Missing headers/libs when building Release: verify Additional Include/Library directories and that Release variants of third‑party libs exist (for example ReactPhysics3D Release lib in `Libs\reactphysics3d\Release`).
- If the Unreal importer fails with `ModuleNotFoundError`, ensure `Content/Python/Tools` exists and restart the Editor so the Python module path is refreshed.

## Deployment note
- The `deploy_release.bat` copies the executable, shaders, and asset folders into the release folder. Confirm the `Release` folder contents before distribution.