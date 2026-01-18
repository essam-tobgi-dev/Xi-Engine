#include "GameApplication.h"
#include "../Engine/Core/Log.h"
#include "../Engine/Core/Input.h"
#include "../Engine/ECS/World.h"
#include "../Engine/ECS/Components/Transform.h"
#include "../Engine/ECS/Components/MeshRenderer.h"
#include "../Engine/ECS/Components/Camera.h"
#include "../Engine/ECS/Components/Light.h"
#include "../Engine/ECS/Components/Collider.h"
#include "../Engine/ECS/Components/RigidBody.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/Renderer/Primitives.h"
#include "../Engine/Renderer/Material.h"
#include "../Engine/Resources/ResourceManager.h"
#include "../Engine/Editor/EditorUI.h"

namespace Xi {

    GameApplication::GameApplication()
        : Application(WindowProps{"Xi Engine", 1600, 900, true, false}) {
    }

    GameApplication::~GameApplication() = default;

    void GameApplication::OnInit() {
        XI_LOG_INFO("Game initializing...");
        CreateDemoScene();
    }

    void GameApplication::CreateDemoScene() {
        World& world = GetWorld();
        Renderer& renderer = GetRenderer();

        // Register primitive meshes
        ResourceManager& rm = ResourceManager::Get();
        rm.RegisterMesh("Cube", Primitives::CreateCube());
        rm.RegisterMesh("Sphere", Primitives::CreateSphere());
        rm.RegisterMesh("Plane", Primitives::CreatePlane(20.0f));
        rm.RegisterMesh("Cylinder", Primitives::CreateCylinder());

        // Create materials
        auto defaultMaterial = rm.CreateMaterial("Default");
        defaultMaterial->SetShader(renderer.GetDefaultShader());
        defaultMaterial->albedoColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
        defaultMaterial->roughness = 0.5f;
        defaultMaterial->metallic = 0.0f;

        auto redMaterial = rm.CreateMaterial("Red");
        redMaterial->SetShader(renderer.GetDefaultShader());
        redMaterial->albedoColor = glm::vec4(0.9f, 0.2f, 0.2f, 1.0f);
        redMaterial->roughness = 0.3f;
        redMaterial->metallic = 0.0f;

        auto blueMaterial = rm.CreateMaterial("Blue");
        blueMaterial->SetShader(renderer.GetDefaultShader());
        blueMaterial->albedoColor = glm::vec4(0.2f, 0.4f, 0.9f, 1.0f);
        blueMaterial->roughness = 0.5f;
        blueMaterial->metallic = 0.5f;

        auto greenMaterial = rm.CreateMaterial("Green");
        greenMaterial->SetShader(renderer.GetDefaultShader());
        greenMaterial->albedoColor = glm::vec4(0.2f, 0.8f, 0.3f, 1.0f);
        greenMaterial->roughness = 0.7f;
        greenMaterial->metallic = 0.0f;

        // Create ground plane
        Entity ground = world.CreateEntity("Ground");
        Transform& groundTransform = world.AddComponent<Transform>(ground);
        groundTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);

        MeshRenderer& groundMesh = world.AddComponent<MeshRenderer>(ground);
        groundMesh.mesh = rm.GetMesh("Plane");
        groundMesh.material = defaultMaterial;

        Collider& groundCollider = world.AddComponent<Collider>(ground);
        groundCollider.type = ColliderType::Box;
        groundCollider.size = glm::vec3(20.0f, 0.1f, 20.0f);
        groundCollider.center = glm::vec3(0.0f, -0.05f, 0.0f);

        // Create cubes
        for (int i = 0; i < 3; i++) {
            Entity cube = world.CreateEntity("Cube " + std::to_string(i + 1));
            Transform& t = world.AddComponent<Transform>(cube);
            t.position = glm::vec3(-3.0f + i * 3.0f, 0.5f, 0.0f);
            t.scale = glm::vec3(1.0f);

            MeshRenderer& mr = world.AddComponent<MeshRenderer>(cube);
            mr.mesh = rm.GetMesh("Cube");
            mr.material = (i == 0) ? redMaterial : ((i == 1) ? blueMaterial : greenMaterial);

            Collider& col = world.AddComponent<Collider>(cube);
            col.type = ColliderType::Box;
            col.size = glm::vec3(1.0f);
        }

        // Create a sphere
        Entity sphere = world.CreateEntity("Sphere");
        Transform& sphereTransform = world.AddComponent<Transform>(sphere);
        sphereTransform.position = glm::vec3(0.0f, 2.0f, 3.0f);

        MeshRenderer& sphereMesh = world.AddComponent<MeshRenderer>(sphere);
        sphereMesh.mesh = rm.GetMesh("Sphere");
        sphereMesh.material = blueMaterial;

        Collider& sphereCol = world.AddComponent<Collider>(sphere);
        sphereCol.type = ColliderType::Sphere;
        sphereCol.radius = 0.5f;

        RigidBody& sphereRb = world.AddComponent<RigidBody>(sphere);
        sphereRb.type = RigidBodyType::Dynamic;
        sphereRb.mass = 1.0f;
        sphereRb.bounciness = 0.5f;

        // Create directional light
        Entity dirLight = world.CreateEntity("Directional Light");
        Transform& lightTransform = world.AddComponent<Transform>(dirLight);
        lightTransform.position = glm::vec3(5.0f, 10.0f, 5.0f);
        lightTransform.rotation = glm::vec3(-45.0f, 45.0f, 0.0f);

        Light& light = world.AddComponent<Light>(dirLight);
        light.type = LightType::Directional;
        light.color = glm::vec3(1.0f, 0.95f, 0.9f);
        light.intensity = 2.0f;

        // Create point light
        Entity pointLight = world.CreateEntity("Point Light");
        Transform& pointLightTransform = world.AddComponent<Transform>(pointLight);
        pointLightTransform.position = glm::vec3(0.0f, 3.0f, 0.0f);

        Light& pLight = world.AddComponent<Light>(pointLight);
        pLight.type = LightType::Point;
        pLight.color = glm::vec3(1.0f, 0.8f, 0.6f);
        pLight.intensity = 5.0f;
        pLight.range = 10.0f;

        XI_LOG_INFO("Demo scene created with " + std::to_string(world.GetEntityCount()) + " entities");
    }

    void GameApplication::OnUpdate(float dt) {
        World& world = GetWorld();
        Renderer& renderer = GetRenderer();

        // Collect lights from the scene
        auto* lightPool = world.GetComponentPool<Light>();
        auto* transformPool = world.GetComponentPool<Transform>();

        if (lightPool && transformPool) {
            renderer.ClearLights();
            for (Entity entity : lightPool->GetEntities()) {
                if (!world.HasComponent<Transform>(entity)) continue;

                const Transform& t = world.GetComponent<Transform>(entity);
                const Light& l = world.GetComponent<Light>(entity);

                LightData lightData;
                lightData.type = static_cast<LightData::Type>(static_cast<int>(l.type));
                lightData.position = t.position;
                lightData.direction = t.GetForward();
                lightData.color = l.color;
                lightData.intensity = l.intensity;
                lightData.range = l.range;
                lightData.spotAngle = l.outerAngle;

                renderer.AddLight(lightData);
            }
        }

        // Submit mesh renderers to the render queue
        auto* meshPool = world.GetComponentPool<MeshRenderer>();
        if (meshPool && transformPool) {
            for (Entity entity : meshPool->GetEntities()) {
                if (!world.HasComponent<Transform>(entity)) continue;
                if (!world.IsEntityActive(entity)) continue;

                const Transform& t = world.GetComponent<Transform>(entity);
                const MeshRenderer& mr = world.GetComponent<MeshRenderer>(entity);

                if (mr.visible && mr.mesh && mr.material) {
                    renderer.Submit(mr.mesh, mr.material, t.GetMatrix());
                }
            }
        }

        // Handle escape to quit
        if (Input::IsKeyPressed(KeyCode::Escape)) {
            Quit();
        }
    }

    void GameApplication::OnFixedUpdate(float dt) {
        (void)dt;
        // Fixed timestep physics updates are handled by Application
    }

    void GameApplication::OnRender() {
        // Additional rendering can be done here
    }

    void GameApplication::OnImGui() {
        // Custom ImGui windows can be added here
    }

    void GameApplication::OnShutdown() {
        XI_LOG_INFO("Game shutting down...");
        ResourceManager::Get().Clear();
    }

}
