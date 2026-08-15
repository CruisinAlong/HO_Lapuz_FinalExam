# After cloning — quick setup

1. Clone the repository.
2. The main executable is in HO_Lapuz_FinalExam\x64\Debug\DirectX Game.exe (Release was bugged and was unable to build)

## Files that must sit next to the `.exe`
- All `.hlsl` shader files (the engine compiles shaders at runtime): `VertexShader.hlsl`, `PixelShader.hlsl`.
- Runtime assets used by the game (textures, meshes, config) — preserve expected relative paths (e.g., `Images`, `Meshes`).

## Unity importer 
- File to add: `Assets/Editor/LevelImporter.cs` (Editor folder required).
- Requirements: Newtonsoft.Json (install via Unity Package Manager or put `Newtonsoft.Json.dll` in `Assets/Plugins`).
- Usage in Editor:
  1. Place `LevelImporter.cs` in `Assets/Editor/`.
  2. Editor → Tools → Import Level... → choose your `.level` file (for example `DirectX Game/Level/TestCases1.level`).
  3. The importer creates a root GameObject and instantiates primitives or mapped prefabs, attaches `Rigidbody`/`BoxCollider` and sets properties (mass, collider size).
- Notes:
  - Rotation: importer expects Euler degrees. If your exported rotations are in radians, enable the radians→degrees conversion (script includes a flag).
  - For custom meshes/prefabs: extend the `prefabMap` in the script to point to prefabs (Resources or a ScriptableObject mapping).

## Unreal importer 
- Place the importer under your Unreal project: `Content/Python/Tools/import_level.py`.
  - Make `Tools` a Python package (optional) by adding an empty `__init__.py` if you plan to import via `Tools.import_level`.
- Enable plugins in the Editor (one-time): Edit → Plugins → enable `Python Editor Script Plugin` and `Editor Scripting Utilities` and restart the Editor.
- In‑Editor usage (fast test): Window → Developer Tools → Python Console, then run:
  import importlib
  import Tools.import_level as il
  importlib.reload(il)
  il.import_level(r"C:\full\path\to\.level file", rotations_in_radians=True)