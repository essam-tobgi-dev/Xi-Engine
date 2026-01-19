#pragma once

#include <cstdint>

namespace Xi {

    struct FramebufferSpec {
        uint32_t width = 1280;
        uint32_t height = 720;
        uint32_t samples = 1;
    };

    class Framebuffer {
    public:
        Framebuffer(const FramebufferSpec& spec);
        ~Framebuffer();

        void Bind();
        void Unbind();

        void Resize(uint32_t width, uint32_t height);

        uint32_t GetColorAttachment() const { return m_ColorAttachment; }
        uint32_t GetDepthAttachment() const { return m_DepthAttachment; }

        uint32_t GetWidth() const { return m_Spec.width; }
        uint32_t GetHeight() const { return m_Spec.height; }

        const FramebufferSpec& GetSpec() const { return m_Spec; }

    private:
        void Invalidate();

        uint32_t m_FramebufferID = 0;
        uint32_t m_ColorAttachment = 0;
        uint32_t m_DepthAttachment = 0;
        FramebufferSpec m_Spec;
    };

}
