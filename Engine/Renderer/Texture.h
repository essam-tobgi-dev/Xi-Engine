#pragma once

#include <string>
#include <cstdint>

namespace Xi {

    enum class TextureFilter {
        Nearest,
        Linear
    };

    enum class TextureWrap {
        Repeat,
        ClampToEdge,
        MirroredRepeat
    };

    struct TextureSpec {
        int width = 0;
        int height = 0;
        int channels = 4;
        TextureFilter minFilter = TextureFilter::Linear;
        TextureFilter magFilter = TextureFilter::Linear;
        TextureWrap wrapS = TextureWrap::Repeat;
        TextureWrap wrapT = TextureWrap::Repeat;
        bool generateMipmaps = true;
    };

    class Texture {
    public:
        Texture();
        ~Texture();

        bool LoadFromFile(const std::string& path);
        bool Create(const TextureSpec& spec, const unsigned char* data = nullptr);

        void Bind(uint32_t slot = 0) const;
        void Unbind() const;

        uint32_t GetID() const { return m_TextureID; }
        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        bool IsValid() const { return m_TextureID != 0; }

        const std::string& GetPath() const { return m_Path; }

    private:
        uint32_t m_TextureID = 0;
        int m_Width = 0;
        int m_Height = 0;
        int m_Channels = 0;
        std::string m_Path;
    };

}
