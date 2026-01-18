#pragma once

#include "../Engine/Core/Application.h"

namespace Xi {

    class GameApplication : public Application {
    public:
        GameApplication();
        virtual ~GameApplication();

    protected:
        void OnInit() override;
        void OnUpdate(float dt) override;
        void OnFixedUpdate(float dt) override;
        void OnRender() override;
        void OnImGui() override;
        void OnShutdown() override;

    private:
        void CreateDemoScene();
    };

}
