# Place under <YourProject>/Content/Python/Tools/import_level.py
import unreal
import json
import os
from math import degrees


SWAP_SCALE_FOR_PLANES = True
SWAP_SCALE_FOR_CUBES = False  # set True if you want cubes to use X,Z,Y ordering like planes

def to_vector(arr, default=(0,0,0)):
    if not arr or len(arr) < 3:
        return unreal.Vector(*default)
    return unreal.Vector(float(arr[0]), float(arr[1]), float(arr[2]))

def _safe_set_mobility(component, mobility):
    try:
        component.set_mobility(mobility)
    except Exception:
        try:
            component.set_mobility(unreal.ComponentMobility.MOVABLE)
        except Exception:
            pass

def _safe_enable_physics(component):
    try:
        component.set_simulate_physics(True)
        return True
    except Exception:
        try:
            if hasattr(component, "get_body_instance"):
                bi = component.get_body_instance()
                if bi:
                    try:
                        bi.set_instance_simulate_physics(True)
                        return True
                    except Exception:
                        pass
        except Exception:
            pass
    return False

def _safe_set_mass(component, mass):
    if mass <= 0:
        return False
    try:
        component.set_mass_override_in_kilograms("", float(mass))
        return True
    except Exception:
        pass
    try:
        component.set_mass_override(float(mass))
        return True
    except Exception:
        pass
    try:
        if hasattr(component, "get_body_instance"):
            bi = component.get_body_instance()
            if bi and hasattr(bi, "set_mass"):
                bi.set_mass(float(mass))
                return True
    except Exception:
        pass
    return False

def import_level(path, rotations_in_radians=True):
    """
    Import a JSON .level file into the current open level.

    Behavior:
    - Divide imported scale values by 10 (reduce giant scale).
    - Multiply position (world transform) by 10.
    - Swap position axes to Unreal order (x, z, y).
    - Swap plane scale axes (x, z, y) AFTER dividing by 10.
      If SWAP_SCALE_FOR_CUBES is True, cubes will use the same swap.
    - Capsule type is imported using a project asset from StarterContent (relative path).
    """
    if not os.path.exists(path):
        unreal.log_error("import_level: File not found: {}".format(path))
        return False
    try:
        with open(path, 'r', encoding='utf-8') as f:
            doc = json.load(f)
    except Exception as e:
        unreal.log_error("import_level: JSON parse error: {}".format(e))
        return False

    objs = doc.get('objects') or []
    unreal.log("import_level: Importing {} objects from {}".format(len(objs), path))

    imported = 0
    for o in objs:
        try:
            typ = (o.get('type') or "Cube")
            pos_raw = o.get('position') or [0,0,0]
            rot_raw = o.get('rotation') or [0,0,0]
            scl_raw = o.get('scale') or [1,1,1]

            # Rotation: convert radians->degrees if requested
            if rotations_in_radians:
                try:
                    rot = [degrees(float(r)) for r in rot_raw]
                except Exception:
                    rot = [0.0, 0.0, 0.0]
            else:
                try:
                    rot = [float(r) for r in rot_raw]
                except Exception:
                    rot = [0.0, 0.0, 0.0]


            try:
                sx, sy, sz = float(scl_raw[0]), float(scl_raw[1]), float(scl_raw[2])
            except Exception:
                sx, sy, sz = 1.0, 1.0, 1.0
            sx /= 10.0; sy /= 10.0; sz /= 10.0


            lower_typ = (typ or "").lower()
            if lower_typ == "plane" and SWAP_SCALE_FOR_PLANES:
                scl = unreal.Vector(sx, sz, sy)
            elif lower_typ == "cube" and SWAP_SCALE_FOR_CUBES:
                scl = unreal.Vector(sx, sz, sy)
            else:
                scl = unreal.Vector(sx, sz, sy)

            # Position: parse, multiply by 10, then swap Y/Z -> (x, z, y)
            try:
                px, py, pz = float(pos_raw[0]), float(pos_raw[1]), float(pos_raw[2])
            except Exception:
                px, py, pz = 0.0, 0.0, 0.0
            px *= 10.0; py *= 10.0; pz *= 10.0
            pos = unreal.Vector(px, pz, py)  # swap y<->z for engine ordering

            # Choose mesh: map "Capsule" -> StarterContent capsule asset (project-relative)
            mesh_path = '/Game/StarterContent/Shapes/Shape_NarrowCapsule' if lower_typ == 'capsule' else '/Engine/BasicShapes/Cube'
            if lower_typ == 'plane':
                mesh_path = '/Engine/BasicShapes/Plane'
            elif lower_typ == 'sphere':
                mesh_path = '/Engine/BasicShapes/Sphere'
            elif lower_typ == 'capsule':
                mesh_path = '/Game/StarterContent/Shapes/Shape_NarrowCapsule'

            mesh_asset = unreal.EditorAssetLibrary.load_asset(mesh_path)
            if not mesh_asset:
                unreal.log_warning("import_level: Mesh not found for type {}, path {}. Falling back to Cube".format(typ, mesh_path))
                mesh_asset = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Cube')

            # Spawn actor at computed position (apply rotation/scale after spawn)
            try:
                actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, pos)
            except Exception as ex:
                unreal.log_error("import_level: Failed to spawn actor: {}".format(ex))
                continue

            smc = getattr(actor, "static_mesh_component", None)
            if smc and mesh_asset:
                try:
                    smc.set_static_mesh(mesh_asset)
                except Exception as ex:
                    unreal.log_warning("import_level: set_static_mesh failed: {}".format(ex))

            # Apply rotation (provide teleport_physics flag on engines that require it)
            try:
                actor.set_actor_rotation(unreal.Rotator(rot[0], rot[1], rot[2]), False)
            except TypeError:
                try:
                    actor.set_actor_rotation(unreal.Rotator(rot[0], rot[1], rot[2]))
                except Exception as ex:
                    unreal.log_warning("import_level: set_actor_rotation fallback failed: {}".format(ex))
            except Exception as ex:
                unreal.log_warning("import_level: set_actor_rotation failed: {}".format(ex))


            try:
                actor.set_actor_scale3d(scl)
            except Exception as ex:
                unreal.log_warning("import_level: set_actor_scale3d failed: {}".format(ex))


            for cr in o.get('components') or []:
                try:
                    ctype = cr.get('type')
                    props = cr.get('props') or {}
                    if ctype == 'Physics':
                        if smc:
                            dynamic_flag = props.get('dynamic', props.get('simulate', True))
                            try:
                                dynamic_flag = bool(dynamic_flag)
                            except Exception:
                                dynamic_flag = True

                            if not dynamic_flag:
                                # Set mobility to STATIC and ensure physics simulation is disabled
                                _safe_set_mobility(smc, unreal.ComponentMobility.STATIC)
                                try:
                                    smc.set_simulate_physics(False)
                                except Exception:
                                    try:
                                        if hasattr(smc, "get_body_instance"):
                                            bi = smc.get_body_instance()
                                            if bi:
                                                try:
                                                    bi.set_instance_simulate_physics(False)
                                                except Exception:
                                                    pass
                                    except Exception:
                                        pass
                                unreal.log("import_level: Physics 'dynamic' flag is False for actor {}, set to STATIC".format(actor.get_name()))
                            else:
                                _safe_set_mobility(smc, unreal.ComponentMobility.MOVABLE)
                                enabled = _safe_enable_physics(smc)
                                if enabled:
                                    try:
                                        mass = float(props.get('mass', 0.0))
                                    except Exception:
                                        mass = 0.0
                                    if mass > 0:
                                        try:
                                            _safe_set_mass(smc, mass)
                                        except Exception:
                                            unreal.log_warning("import_level: mass override attempt raised unexpected error; skipping")
                                else:
                                    unreal.log_warning("import_level: could not enable physics on component for actor {}".format(actor.get_name()))
                    elif ctype == 'BoxCollider':
                        he = props.get('halfExtents')
                        if isinstance(he, (list, tuple)) and len(he) >= 3:
                            try:
                                box_comp = unreal.EditorUtilities.add_component(actor, unreal.BoxComponent, "BoxCollider")
                                box_comp.set_box_extent(unreal.Vector(he[0], he[1], he[2]))
                                box_comp.register_component()
                            except Exception as ex:
                                unreal.log_warning("import_level: failed to add BoxComponent: {}".format(ex))
                    elif ctype == 'MeshComponent':
                        pass
                except Exception as ex:
                    unreal.log_warning("import_level: component handling error: {}".format(ex))

            imported += 1

        except Exception as e:
            unreal.log_warning("import_level: object import failed, continuing. Error: {}".format(e))
            continue

    unreal.log("import_level: Done importing file: {} ({} successful)".format(path, imported))
    return True