using UnityEngine;
using UnityEditor;
using System.IO;
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using JsonConvert = Newtonsoft.Json.JsonConvert;

public class LevelImporter
{
    // Toggle this if your exporter writes rotation in radians
    private static bool rotationIsRadians = true;

    // Simple mapping: level type -> prefab path in Resources (or null to use primitives)
    // Extend this dictionary to point to project prefabs (place prefabs under Resources or adjust loader).
    private static readonly Dictionary<string, string> prefabMap = new Dictionary<string, string>()
    {
        { "cube", null },    // null = use built-in PrimitiveType.Cube
        { "plane", null },
        { "sphere", null },
        { "capsule", null },
        // Example mapping to a prefab in Resources folder:
        // { "MyCustomActor", "Prefabs/MyCustomActorPrefab" }
    };

    // JSON models
    class Level { public int version; public List<SerializedObject> objects; }
    class SerializedObject
    {
        public string type;
        public float[] position;
        public float[] rotation;
        public float[] scale;
        public List<ComponentRecord> components;
    }
    class ComponentRecord
    {
        public string type;
        public JObject props; // use JToken/JObject for robust parsing
    }

    [MenuItem("Tools/Import Level...")]
    public static void ImportLevelMenu()
    {
        string path = EditorUtility.OpenFilePanel("Open .level file", Application.dataPath, "level");
        if (string.IsNullOrEmpty(path)) return;
        ImportLevel(path);
    }

    public static void ImportLevel(string path)
    {
        string json = File.ReadAllText(path);
        Level lvl = null;
        try
        {
            lvl = JsonConvert.DeserializeObject<Level>(json);
        }
        catch (System.Exception ex)
        {
            Debug.LogError("Level import failed: " + ex.Message);
            return;
        }

        if (lvl?.objects == null)
        {
            Debug.LogWarning("No objects found in level.");
            return;
        }

        GameObject root = new GameObject(Path.GetFileNameWithoutExtension(path));
        Undo.RegisterCreatedObjectUndo(root, "Import Level");

        foreach (var o in lvl.objects)
        {
            Vector3 pos = ToVec(o.position, Vector3.zero);
            Vector3 rotArr = ToVec(o.rotation, Vector3.zero);
            if (rotationIsRadians) rotArr *= Mathf.Rad2Deg;
            Vector3 scl = ToVec(o.scale, Vector3.one);

            string typeKey = (o.type ?? "Cube").ToLowerInvariant();
            GameObject go = null;

            // Try prefab mapping first
            if (prefabMap.TryGetValue(typeKey, out string prefabPath) && !string.IsNullOrEmpty(prefabPath))
            {
                var prefab = Resources.Load<GameObject>(prefabPath);
                if (prefab != null) go = (GameObject)PrefabUtility.InstantiatePrefab(prefab);
            }

            // Fallback to primitives when no prefab
            if (go == null)
            {
                switch (typeKey)
                {
                    case "plane": go = GameObject.CreatePrimitive(PrimitiveType.Plane); break;
                    case "sphere": go = GameObject.CreatePrimitive(PrimitiveType.Sphere); break;
                    case "capsule": go = GameObject.CreatePrimitive(PrimitiveType.Capsule); break;
                    default: go = GameObject.CreatePrimitive(PrimitiveType.Cube); break;
                }
            }

            if (!go) continue;
            Undo.RegisterCreatedObjectUndo(go, "Import Level Object");
            go.transform.SetParent(root.transform);
            go.transform.position = pos;
            go.transform.rotation = Quaternion.Euler(rotArr);
            go.transform.localScale = scl;

            // Apply components
            if (o.components != null)
            {
                foreach (var cr in o.components)
                {
                    string t = cr.type ?? "";
                    if (t == "Physics")
                    {
                        var rb = go.GetComponent<Rigidbody>();
                        if (!rb) rb = go.AddComponent<Rigidbody>();
                        if (cr.props != null && cr.props.TryGetValue("mass", out JToken massToken))
                        {
                            if (massToken != null && float.TryParse(massToken.ToString(), out float mass))
                            {
                                rb.mass = mass;
                            }
                        }
                    }
                    else if (t == "BoxCollider")
                    {
                        var bc = go.GetComponent<BoxCollider>();
                        if (!bc) bc = go.AddComponent<BoxCollider>();
                        if (cr.props != null && cr.props.TryGetValue("halfExtents", out JToken heToken) && heToken is JArray heArr && heArr.Count >= 3)
                        {
                            float hx = heArr[0].ToObject<float>();
                            float hy = heArr[1].ToObject<float>();
                            float hz = heArr[2].ToObject<float>();
                            // JSON stored half-extents: convert to full size for Unity collider.size
                            bc.size = new Vector3(hx * 2f, hy * 2f, hz * 2f);
                        }
                    }
                    else if (t == "MeshComponent")
                    {
                        // Optionally replace primitive mesh with project mesh or assign materials
                    }
                }
            }
        }

        Debug.Log($"Imported {lvl.objects.Count} objects from {Path.GetFileName(path)}");
    }

    static Vector3 ToVec(float[] arr, Vector3 fallback)
    {
        if (arr == null || arr.Length < 3) return fallback;
        return new Vector3(arr[0], arr[1], arr[2]);
    }
}