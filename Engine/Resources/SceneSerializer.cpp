#include "SceneSerializer.h"
#include "../ECS/World.h"
#include "../ECS/Components/Transform.h"
#include "../ECS/Components/MeshRenderer.h"
#include "../ECS/Components/SpriteRenderer.h"
#include "../ECS/Components/Camera.h"
#include "../ECS/Components/Light.h"
#include "../ECS/Components/Collider.h"
#include "../ECS/Components/RigidBody.h"
#include "../ECS/Components/AudioSource.h"
#include "../Core/Log.h"

#include <json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace Xi {

    // Helper functions for GLM serialization
    static json Vec2ToJson(const glm::vec2& v) {
        return json::array({v.x, v.y});
    }

    static json Vec3ToJson(const glm::vec3& v) {
        return json::array({v.x, v.y, v.z});
    }

    static json Vec4ToJson(const glm::vec4& v) {
        return json::array({v.x, v.y, v.z, v.w});
    }

    static glm::vec2 JsonToVec2(const json& j) {
        return glm::vec2(j[0].get<float>(), j[1].get<float>());
    }

    static glm::vec3 JsonToVec3(const json& j) {
        return glm::vec3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
    }

    static glm::vec4 JsonToVec4(const json& j) {
        return glm::vec4(j[0].get<float>(), j[1].get<float>(), j[2].get<float>(), j[3].get<float>());
    }

    SceneSerializer::SceneSerializer(World& world)
        : m_World(world) {}

    bool SceneSerializer::Save(const std::string& filepath) {
        json scene;
        scene["version"] = "1.0";
        scene["name"] = "Scene";
        scene["entities"] = json::array();

        for (const auto& [entity, info] : m_World.GetEntities()) {
            json entityJson;
            entityJson["id"] = entity;
            entityJson["name"] = info.name;
            entityJson["active"] = info.active;
            entityJson["parent"] = info.parent;
            entityJson["components"] = json::object();

            // Transform
            if (m_World.HasComponent<Transform>(entity)) {
                const Transform& t = m_World.GetComponent<Transform>(entity);
                entityJson["components"]["Transform"] = {
                    {"position", Vec3ToJson(t.position)},
                    {"rotation", Vec3ToJson(t.rotation)},
                    {"scale", Vec3ToJson(t.scale)}
                };
            }

            // Camera
            if (m_World.HasComponent<CameraComponent>(entity)) {
                const CameraComponent& c = m_World.GetComponent<CameraComponent>(entity);
                entityJson["components"]["Camera"] = {
                    {"isMain", c.isMain},
                    {"priority", c.priority},
                    {"projectionType", static_cast<int>(c.camera.GetProjectionType())},
                    {"fov", c.camera.GetFOV()},
                    {"nearClip", c.camera.GetNearClip()},
                    {"farClip", c.camera.GetFarClip()},
                    {"orthoSize", c.camera.GetOrthographicSize()}
                };
            }

            // Light
            if (m_World.HasComponent<Light>(entity)) {
                const Light& l = m_World.GetComponent<Light>(entity);
                entityJson["components"]["Light"] = {
                    {"type", static_cast<int>(l.type)},
                    {"color", Vec3ToJson(l.color)},
                    {"intensity", l.intensity},
                    {"range", l.range},
                    {"innerAngle", l.innerAngle},
                    {"outerAngle", l.outerAngle},
                    {"castShadows", l.castShadows}
                };
            }

            // Collider
            if (m_World.HasComponent<Collider>(entity)) {
                const Collider& c = m_World.GetComponent<Collider>(entity);
                entityJson["components"]["Collider"] = {
                    {"type", static_cast<int>(c.type)},
                    {"center", Vec3ToJson(c.center)},
                    {"size", Vec3ToJson(c.size)},
                    {"radius", c.radius},
                    {"height", c.height},
                    {"isTrigger", c.isTrigger},
                    {"layer", c.layer},
                    {"mask", c.mask}
                };
            }

            // RigidBody
            if (m_World.HasComponent<RigidBody>(entity)) {
                const RigidBody& rb = m_World.GetComponent<RigidBody>(entity);
                entityJson["components"]["RigidBody"] = {
                    {"type", static_cast<int>(rb.type)},
                    {"mass", rb.mass},
                    {"drag", rb.drag},
                    {"angularDrag", rb.angularDrag},
                    {"useGravity", rb.useGravity},
                    {"friction", rb.friction},
                    {"bounciness", rb.bounciness}
                };
            }

            // AudioSource
            if (m_World.HasComponent<AudioSource>(entity)) {
                const AudioSource& as = m_World.GetComponent<AudioSource>(entity);
                entityJson["components"]["AudioSource"] = {
                    {"clipPath", as.clipPath},
                    {"volume", as.volume},
                    {"pitch", as.pitch},
                    {"minDistance", as.minDistance},
                    {"maxDistance", as.maxDistance},
                    {"playOnAwake", as.playOnAwake},
                    {"loop", as.loop},
                    {"is3D", as.is3D}
                };
            }

            scene["entities"].push_back(entityJson);
        }

        std::ofstream file(filepath);
        if (!file.is_open()) {
            XI_LOG_ERROR("Failed to save scene: " + filepath);
            return false;
        }

        file << scene.dump(2);
        XI_LOG_INFO("Scene saved: " + filepath);
        return true;
    }

    bool SceneSerializer::Load(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            XI_LOG_ERROR("Failed to load scene: " + filepath);
            return false;
        }

        json scene;
        try {
            file >> scene;
        } catch (const json::parse_error& e) {
            XI_LOG_ERROR("JSON parse error: " + std::string(e.what()));
            return false;
        }

        m_World.Clear();

        // First pass: create all entities
        std::unordered_map<Entity, Entity> entityMap; // Old ID -> New ID

        for (const auto& entityJson : scene["entities"]) {
            Entity oldId = entityJson["id"].get<Entity>();
            std::string name = entityJson["name"].get<std::string>();
            Entity newId = m_World.CreateEntity(name);

            m_World.SetEntityActive(newId, entityJson["active"].get<bool>());
            entityMap[oldId] = newId;
        }

        // Second pass: set up hierarchy and components
        for (const auto& entityJson : scene["entities"]) {
            Entity oldId = entityJson["id"].get<Entity>();
            Entity entity = entityMap[oldId];

            // Parent
            if (entityJson.contains("parent") && !entityJson["parent"].is_null()) {
                Entity oldParent = entityJson["parent"].get<Entity>();
                if (oldParent != INVALID_ENTITY && entityMap.count(oldParent)) {
                    m_World.SetParent(entity, entityMap[oldParent]);
                }
            }

            const auto& components = entityJson["components"];

            // Transform
            if (components.contains("Transform")) {
                const auto& t = components["Transform"];
                Transform& transform = m_World.AddComponent<Transform>(entity);
                transform.position = JsonToVec3(t["position"]);
                transform.rotation = JsonToVec3(t["rotation"]);
                transform.scale = JsonToVec3(t["scale"]);
            }

            // Camera
            if (components.contains("Camera")) {
                const auto& c = components["Camera"];
                CameraComponent& cam = m_World.AddComponent<CameraComponent>(entity);
                cam.isMain = c["isMain"].get<bool>();
                cam.priority = c["priority"].get<int>();
                cam.camera.SetProjectionType(static_cast<ProjectionType>(c["projectionType"].get<int>()));
                cam.camera.SetFOV(c["fov"].get<float>());
                cam.camera.SetNearClip(c["nearClip"].get<float>());
                cam.camera.SetFarClip(c["farClip"].get<float>());
                cam.camera.SetOrthographicSize(c["orthoSize"].get<float>());
            }

            // Light
            if (components.contains("Light")) {
                const auto& l = components["Light"];
                Light& light = m_World.AddComponent<Light>(entity);
                light.type = static_cast<LightType>(l["type"].get<int>());
                light.color = JsonToVec3(l["color"]);
                light.intensity = l["intensity"].get<float>();
                light.range = l["range"].get<float>();
                light.innerAngle = l["innerAngle"].get<float>();
                light.outerAngle = l["outerAngle"].get<float>();
                light.castShadows = l["castShadows"].get<bool>();
            }

            // Collider
            if (components.contains("Collider")) {
                const auto& c = components["Collider"];
                Collider& col = m_World.AddComponent<Collider>(entity);
                col.type = static_cast<ColliderType>(c["type"].get<int>());
                col.center = JsonToVec3(c["center"]);
                col.size = JsonToVec3(c["size"]);
                col.radius = c["radius"].get<float>();
                col.height = c["height"].get<float>();
                col.isTrigger = c["isTrigger"].get<bool>();
                col.layer = c["layer"].get<uint32_t>();
                col.mask = c["mask"].get<uint32_t>();
            }

            // RigidBody
            if (components.contains("RigidBody")) {
                const auto& r = components["RigidBody"];
                RigidBody& rb = m_World.AddComponent<RigidBody>(entity);
                rb.type = static_cast<RigidBodyType>(r["type"].get<int>());
                rb.mass = r["mass"].get<float>();
                rb.drag = r["drag"].get<float>();
                rb.angularDrag = r["angularDrag"].get<float>();
                rb.useGravity = r["useGravity"].get<bool>();
                rb.friction = r["friction"].get<float>();
                rb.bounciness = r["bounciness"].get<float>();
            }

            // AudioSource
            if (components.contains("AudioSource")) {
                const auto& a = components["AudioSource"];
                AudioSource& as = m_World.AddComponent<AudioSource>(entity);
                as.clipPath = a["clipPath"].get<std::string>();
                as.volume = a["volume"].get<float>();
                as.pitch = a["pitch"].get<float>();
                as.minDistance = a["minDistance"].get<float>();
                as.maxDistance = a["maxDistance"].get<float>();
                as.playOnAwake = a["playOnAwake"].get<bool>();
                as.loop = a["loop"].get<bool>();
                as.is3D = a["is3D"].get<bool>();
            }
        }

        XI_LOG_INFO("Scene loaded: " + filepath);
        return true;
    }

}
