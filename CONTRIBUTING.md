# Resource cleanup guideline

To ensure deterministic unloading of GPU/OS resources, resource manager classes expose a `releaseAllResources()` method. Owners of the manager (for example `GraphicsEngine`) must call this method before destroying systems that resources depend on (for example `RenderSystem`). This prevents dangling references and ensures proper cleanup order.

Usage:
- Call `textureManager->releaseAllResources()` before deleting the `textureManager` and before destroying `RenderSystem`.
- Call `resourcePtr.reset()` on any outstanding `std::shared_ptr` held elsewhere (UI, game objects) before manager destruction.

This guideline is enforced in the project by calling `releaseAllResources()` in `GraphicsEngine::~GraphicsEngine()` prior to deleting the manager.

## ResourceManager: releaseAllResources

Added `releaseAllResources()` to `ResourceManager` so owners can explicitly free all managed resources before tearing down dependent systems (for example, call from `GraphicsEngine` before destroying `RenderSystem`). This helps deterministic unloading of GPU resources and prevents dangling references.