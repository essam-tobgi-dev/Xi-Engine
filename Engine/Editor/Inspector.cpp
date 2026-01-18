#include "Inspector.h"
#include "../ECS/World.h"
#include "../ECS/Components/Transform.h"
#include "../ECS/Components/MeshRenderer.h"
#include "../ECS/Components/SpriteRenderer.h"
#include "../ECS/Components/Camera.h"
#include "../ECS/Components/Light.h"
#include "../ECS/Components/Collider.h"
#include "../ECS/Components/RigidBody.h"
#include "../ECS/Components/AudioSource.h"
#include "../Renderer/Material.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Xi {

    Inspector::Inspector() = default;
    Inspector::~Inspector() = default;

    void Inspector::Draw(World& world, Entity entity) {
        ImGui::Begin("Inspector");

        if (entity == INVALID_ENTITY) {
            ImGui::Text("No entity selected");
            ImGui::End();
            return;
        }

        // Entity name
        static char nameBuffer[256];
        strncpy_s(nameBuffer, world.GetEntityName(entity).c_str(), sizeof(nameBuffer) - 1);

        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
            world.SetEntityName(entity, nameBuffer);
        }

        // Active toggle
        bool active = world.IsEntityActive(entity);
        if (ImGui::Checkbox("Active", &active)) {
            world.SetEntityActive(entity, active);
        }

        ImGui::Separator();

        // Draw components
        DrawTransform(world, entity);
        DrawMeshRenderer(world, entity);
        DrawSpriteRenderer(world, entity);
        DrawCamera(world, entity);
        DrawLight(world, entity);
        DrawCollider(world, entity);
        DrawRigidBody(world, entity);
        DrawAudioSource(world, entity);

        ImGui::Separator();

        // Add component button
        DrawAddComponentMenu(world, entity);

        ImGui::End();
    }

    void Inspector::DrawTransform(World& world, Entity entity) {
        if (!world.HasComponent<Transform>(entity)) return;

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            Transform& t = world.GetComponent<Transform>(entity);

            ImGui::DragFloat3("Position", glm::value_ptr(t.position), 0.1f);
            ImGui::DragFloat3("Rotation", glm::value_ptr(t.rotation), 1.0f);
            ImGui::DragFloat3("Scale", glm::value_ptr(t.scale), 0.1f, 0.001f, 1000.0f);

            // Remove button
            if (ImGui::Button("Remove##Transform")) {
                world.RemoveComponent<Transform>(entity);
            }
        }
    }

    void Inspector::DrawMeshRenderer(World& world, Entity entity) {
        if (!world.HasComponent<MeshRenderer>(entity)) return;

        if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
            MeshRenderer& mr = world.GetComponent<MeshRenderer>(entity);

            ImGui::Checkbox("Visible", &mr.visible);
            ImGui::Checkbox("Cast Shadows", &mr.castShadows);
            ImGui::Checkbox("Receive Shadows", &mr.receiveShadows);

            if (mr.material) {
                ImGui::Text("Material Properties:");
                ImGui::ColorEdit4("Albedo", &mr.material->albedoColor.x);
                ImGui::SliderFloat("Metallic", &mr.material->metallic, 0.0f, 1.0f);
                ImGui::SliderFloat("Roughness", &mr.material->roughness, 0.0f, 1.0f);
                ImGui::SliderFloat("AO", &mr.material->ao, 0.0f, 1.0f);
                ImGui::ColorEdit3("Emissive", &mr.material->emissive.x);
            }

            if (ImGui::Button("Remove##MeshRenderer")) {
                world.RemoveComponent<MeshRenderer>(entity);
            }
        }
    }

    void Inspector::DrawSpriteRenderer(World& world, Entity entity) {
        if (!world.HasComponent<SpriteRenderer>(entity)) return;

        if (ImGui::CollapsingHeader("Sprite Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
            SpriteRenderer& sr = world.GetComponent<SpriteRenderer>(entity);

            ImGui::Checkbox("Visible##Sprite", &sr.visible);
            ImGui::ColorEdit4("Color##Sprite", glm::value_ptr(sr.color));
            ImGui::DragFloat2("Tiling", glm::value_ptr(sr.tiling), 0.1f);
            ImGui::DragFloat2("Offset", glm::value_ptr(sr.offset), 0.1f);
            ImGui::DragInt("Sorting Order", &sr.sortingOrder);
            ImGui::Checkbox("Flip X", &sr.flipX);
            ImGui::Checkbox("Flip Y", &sr.flipY);

            if (ImGui::Button("Remove##SpriteRenderer")) {
                world.RemoveComponent<SpriteRenderer>(entity);
            }
        }
    }

    void Inspector::DrawCamera(World& world, Entity entity) {
        if (!world.HasComponent<CameraComponent>(entity)) return;

        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
            CameraComponent& cc = world.GetComponent<CameraComponent>(entity);

            ImGui::Checkbox("Main Camera", &cc.isMain);
            ImGui::DragInt("Priority", &cc.priority);
            ImGui::ColorEdit4("Clear Color", cc.clearColor);
            ImGui::Checkbox("Clear Depth", &cc.clearDepth);

            const char* projTypes[] = { "Perspective", "Orthographic" };
            int currentProj = static_cast<int>(cc.camera.GetProjectionType());
            if (ImGui::Combo("Projection", &currentProj, projTypes, 2)) {
                cc.camera.SetProjectionType(static_cast<ProjectionType>(currentProj));
            }

            if (cc.camera.GetProjectionType() == ProjectionType::Perspective) {
                float fov = cc.camera.GetFOV();
                if (ImGui::SliderFloat("FOV", &fov, 1.0f, 179.0f)) {
                    cc.camera.SetFOV(fov);
                }
            } else {
                float size = cc.camera.GetOrthographicSize();
                if (ImGui::DragFloat("Size", &size, 0.1f, 0.1f, 1000.0f)) {
                    cc.camera.SetOrthographicSize(size);
                }
            }

            float nearClip = cc.camera.GetNearClip();
            float farClip = cc.camera.GetFarClip();
            if (ImGui::DragFloat("Near Clip", &nearClip, 0.01f, 0.001f, farClip)) {
                cc.camera.SetNearClip(nearClip);
            }
            if (ImGui::DragFloat("Far Clip", &farClip, 1.0f, nearClip, 100000.0f)) {
                cc.camera.SetFarClip(farClip);
            }

            if (ImGui::Button("Remove##Camera")) {
                world.RemoveComponent<CameraComponent>(entity);
            }
        }
    }

    void Inspector::DrawLight(World& world, Entity entity) {
        if (!world.HasComponent<Light>(entity)) return;

        if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            Light& light = world.GetComponent<Light>(entity);

            const char* lightTypes[] = { "Directional", "Point", "Spot" };
            int currentType = static_cast<int>(light.type);
            if (ImGui::Combo("Type", &currentType, lightTypes, 3)) {
                light.type = static_cast<LightType>(currentType);
            }

            ImGui::ColorEdit3("Color##Light", glm::value_ptr(light.color));
            ImGui::DragFloat("Intensity", &light.intensity, 0.1f, 0.0f, 100.0f);

            if (light.type != LightType::Directional) {
                ImGui::DragFloat("Range", &light.range, 0.1f, 0.1f, 1000.0f);
            }

            if (light.type == LightType::Spot) {
                ImGui::SliderFloat("Inner Angle", &light.innerAngle, 0.0f, light.outerAngle);
                ImGui::SliderFloat("Outer Angle", &light.outerAngle, light.innerAngle, 90.0f);
            }

            ImGui::Checkbox("Cast Shadows##Light", &light.castShadows);

            if (ImGui::Button("Remove##Light")) {
                world.RemoveComponent<Light>(entity);
            }
        }
    }

    void Inspector::DrawCollider(World& world, Entity entity) {
        if (!world.HasComponent<Collider>(entity)) return;

        if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen)) {
            Collider& col = world.GetComponent<Collider>(entity);

            const char* colliderTypes[] = { "Box", "Sphere", "Capsule" };
            int currentType = static_cast<int>(col.type);
            if (ImGui::Combo("Type##Collider", &currentType, colliderTypes, 3)) {
                col.type = static_cast<ColliderType>(currentType);
            }

            ImGui::DragFloat3("Center", glm::value_ptr(col.center), 0.1f);

            if (col.type == ColliderType::Box) {
                ImGui::DragFloat3("Size##Box", glm::value_ptr(col.size), 0.1f, 0.001f);
            } else if (col.type == ColliderType::Sphere) {
                ImGui::DragFloat("Radius##Sphere", &col.radius, 0.1f, 0.001f);
            } else if (col.type == ColliderType::Capsule) {
                ImGui::DragFloat("Radius##Capsule", &col.radius, 0.1f, 0.001f);
                ImGui::DragFloat("Height", &col.height, 0.1f, 0.001f);
            }

            ImGui::Checkbox("Is Trigger", &col.isTrigger);

            if (ImGui::Button("Remove##Collider")) {
                world.RemoveComponent<Collider>(entity);
            }
        }
    }

    void Inspector::DrawRigidBody(World& world, Entity entity) {
        if (!world.HasComponent<RigidBody>(entity)) return;

        if (ImGui::CollapsingHeader("Rigid Body", ImGuiTreeNodeFlags_DefaultOpen)) {
            RigidBody& rb = world.GetComponent<RigidBody>(entity);

            const char* bodyTypes[] = { "Static", "Kinematic", "Dynamic" };
            int currentType = static_cast<int>(rb.type);
            if (ImGui::Combo("Type##RigidBody", &currentType, bodyTypes, 3)) {
                rb.type = static_cast<RigidBodyType>(currentType);
            }

            if (rb.type == RigidBodyType::Dynamic) {
                ImGui::DragFloat("Mass", &rb.mass, 0.1f, 0.001f, 10000.0f);
                ImGui::DragFloat("Drag", &rb.drag, 0.01f, 0.0f, 10.0f);
                ImGui::DragFloat("Angular Drag", &rb.angularDrag, 0.01f, 0.0f, 10.0f);
                ImGui::Checkbox("Use Gravity", &rb.useGravity);
            }

            ImGui::DragFloat("Friction", &rb.friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Bounciness", &rb.bounciness, 0.01f, 0.0f, 1.0f);

            if (ImGui::TreeNode("Constraints")) {
                ImGui::Checkbox("Freeze Position X", &rb.freezePositionX);
                ImGui::Checkbox("Freeze Position Y", &rb.freezePositionY);
                ImGui::Checkbox("Freeze Position Z", &rb.freezePositionZ);
                ImGui::Checkbox("Freeze Rotation X", &rb.freezeRotationX);
                ImGui::Checkbox("Freeze Rotation Y", &rb.freezeRotationY);
                ImGui::Checkbox("Freeze Rotation Z", &rb.freezeRotationZ);
                ImGui::TreePop();
            }

            if (ImGui::Button("Remove##RigidBody")) {
                world.RemoveComponent<RigidBody>(entity);
            }
        }
    }

    void Inspector::DrawAudioSource(World& world, Entity entity) {
        if (!world.HasComponent<AudioSource>(entity)) return;

        if (ImGui::CollapsingHeader("Audio Source", ImGuiTreeNodeFlags_DefaultOpen)) {
            AudioSource& as = world.GetComponent<AudioSource>(entity);

            // Clip path input
            static char pathBuffer[256];
            strncpy_s(pathBuffer, as.clipPath.c_str(), sizeof(pathBuffer) - 1);
            if (ImGui::InputText("Clip Path", pathBuffer, sizeof(pathBuffer))) {
                as.clipPath = pathBuffer;
            }

            ImGui::SliderFloat("Volume", &as.volume, 0.0f, 1.0f);
            ImGui::SliderFloat("Pitch", &as.pitch, 0.1f, 3.0f);
            ImGui::DragFloat("Min Distance", &as.minDistance, 0.1f, 0.1f, as.maxDistance);
            ImGui::DragFloat("Max Distance", &as.maxDistance, 1.0f, as.minDistance, 10000.0f);

            ImGui::Checkbox("Play On Awake", &as.playOnAwake);
            ImGui::Checkbox("Loop", &as.loop);
            ImGui::Checkbox("3D Sound", &as.is3D);
            ImGui::Checkbox("Mute", &as.mute);

            ImGui::Text("Is Playing: %s", as.isPlaying ? "Yes" : "No");

            if (ImGui::Button("Remove##AudioSource")) {
                world.RemoveComponent<AudioSource>(entity);
            }
        }
    }

    void Inspector::DrawAddComponentMenu(World& world, Entity entity) {
        if (ImGui::Button("Add Component")) {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup")) {
            if (!world.HasComponent<Transform>(entity)) {
                if (ImGui::MenuItem("Transform")) {
                    world.AddComponent<Transform>(entity);
                }
            }
            if (!world.HasComponent<MeshRenderer>(entity)) {
                if (ImGui::MenuItem("Mesh Renderer")) {
                    world.AddComponent<MeshRenderer>(entity);
                }
            }
            if (!world.HasComponent<SpriteRenderer>(entity)) {
                if (ImGui::MenuItem("Sprite Renderer")) {
                    world.AddComponent<SpriteRenderer>(entity);
                }
            }
            if (!world.HasComponent<CameraComponent>(entity)) {
                if (ImGui::MenuItem("Camera")) {
                    world.AddComponent<CameraComponent>(entity);
                }
            }
            if (!world.HasComponent<Light>(entity)) {
                if (ImGui::MenuItem("Light")) {
                    world.AddComponent<Light>(entity);
                }
            }
            if (!world.HasComponent<Collider>(entity)) {
                if (ImGui::MenuItem("Collider")) {
                    world.AddComponent<Collider>(entity);
                }
            }
            if (!world.HasComponent<RigidBody>(entity)) {
                if (ImGui::MenuItem("Rigid Body")) {
                    world.AddComponent<RigidBody>(entity);
                }
            }
            if (!world.HasComponent<AudioSource>(entity)) {
                if (ImGui::MenuItem("Audio Source")) {
                    world.AddComponent<AudioSource>(entity);
                }
            }
            ImGui::EndPopup();
        }
    }

}
