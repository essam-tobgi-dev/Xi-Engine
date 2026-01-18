#include "Texture.h"
#include "../Core/Log.h"

#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Xi {

    static GLenum GetGLFilter(TextureFilter filter) {
        switch (filter) {
            case TextureFilter::Nearest: return GL_NEAREST;
            case TextureFilter::Linear:  return GL_LINEAR;
        }
        return GL_LINEAR;
    }

    static GLenum GetGLWrap(TextureWrap wrap) {
        switch (wrap) {
            case TextureWrap::Repeat:         return GL_REPEAT;
            case TextureWrap::ClampToEdge:    return GL_CLAMP_TO_EDGE;
            case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        }
        return GL_REPEAT;
    }

    Texture::Texture() = default;

    Texture::~Texture() {
        if (m_TextureID) {
            glDeleteTextures(1, &m_TextureID);
        }
    }

    bool Texture::LoadFromFile(const std::string& path) {
        m_Path = path;

        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Channels, 0);

        if (!data) {
            XI_LOG_ERROR("Failed to load texture: " + path);
            return false;
        }

        GLenum internalFormat = GL_RGBA8;
        GLenum dataFormat = GL_RGBA;

        if (m_Channels == 1) {
            internalFormat = GL_R8;
            dataFormat = GL_RED;
        } else if (m_Channels == 3) {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        } else if (m_Channels == 4) {
            internalFormat = GL_RGBA8;
            dataFormat = GL_RGBA;
        }

        glGenTextures(1, &m_TextureID);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        XI_LOG_INFO("Texture loaded: " + path + " (" + std::to_string(m_Width) + "x" + std::to_string(m_Height) + ")");
        return true;
    }

    bool Texture::Create(const TextureSpec& spec, const unsigned char* data) {
        m_Width = spec.width;
        m_Height = spec.height;
        m_Channels = spec.channels;

        GLenum internalFormat = GL_RGBA8;
        GLenum dataFormat = GL_RGBA;

        if (spec.channels == 1) {
            internalFormat = GL_R8;
            dataFormat = GL_RED;
        } else if (spec.channels == 3) {
            internalFormat = GL_RGB8;
            dataFormat = GL_RGB;
        }

        glGenTextures(1, &m_TextureID);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);

        GLenum minFilter = GetGLFilter(spec.minFilter);
        if (spec.generateMipmaps && spec.minFilter == TextureFilter::Linear) {
            minFilter = GL_LINEAR_MIPMAP_LINEAR;
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetGLFilter(spec.magFilter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GetGLWrap(spec.wrapS));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GetGLWrap(spec.wrapT));

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);

        if (spec.generateMipmaps) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        return true;
    }

    void Texture::Bind(uint32_t slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
    }

    void Texture::Unbind() const {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

}
