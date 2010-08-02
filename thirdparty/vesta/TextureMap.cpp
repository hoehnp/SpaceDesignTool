/*
 * $Revision: 389 $ $Date: 2010-07-26 19:34:58 -0700 (Mon, 26 Jul 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "TextureMap.h"
#include "TextureMapLoader.h"
#include "Debug.h"
#include <GL/glew.h>
#include <cassert>

using namespace vesta;
using namespace std;


static GLenum
ToGlFormat(TextureMap::ImageFormat format)
{
    switch (format)
    {
    case TextureMap::R8G8B8A8:
        return GL_RGBA;
    case TextureMap::R8G8B8:
        return GL_RGB;
    case TextureMap::B8G8R8A8:
        return GL_BGRA_EXT;
    case TextureMap::B8G8R8:
        return GL_BGR_EXT;

    // Compressed formats
    case TextureMap::DXT1:
        return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    case TextureMap::DXT3:
        return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    case TextureMap::DXT5:
        return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

    // Floating point formats
    case TextureMap::RGB16F:
    case TextureMap::RGB32F:
        return GL_RGB;
    case TextureMap::RGBA16F:
    case TextureMap::RGBA32F:
        return GL_RGBA;
    case TextureMap::R16F:
    case TextureMap::R32F:
        return GL_LUMINANCE;
    case TextureMap::RG16F:
    case TextureMap::RG32F:
        return GL_LUMINANCE_ALPHA;

    case TextureMap::Depth24:
        return GL_DEPTH_COMPONENT;

    default:
        return 0;
    }
}

static GLenum
ToGlInternalFormat(TextureMap::ImageFormat format)
{
    switch (format)
    {
    case TextureMap::R8G8B8A8:
    case TextureMap::B8G8R8A8:
        return GL_RGBA8;
    case TextureMap::R8G8B8:
    case TextureMap::B8G8R8:
        return GL_RGB8;

    // Compressed formats are identical to their internal formats
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        return format;

    // Half-precision (16-bit / channel) floating point formats
    case TextureMap::RGB16F:
        return GL_RGB16F_ARB;
    case TextureMap::RGBA16F:
        return GL_RGBA16F_ARB;
    case TextureMap::R16F:
        return GL_LUMINANCE16F_ARB;
    case TextureMap::RG16F:
        return GL_LUMINANCE_ALPHA16F_ARB;

    // Single precision (32-bit / channel) floating point formats
    case TextureMap::RGB32F:
        return GL_RGB32F_ARB;
    case TextureMap::RGBA32F:
        return GL_RGBA32F_ARB;
    case TextureMap::R32F:
        return GL_LUMINANCE32F_ARB;
    case TextureMap::RG32F:
        return GL_LUMINANCE_ALPHA32F_ARB;

    case TextureMap::Depth24:
        return GL_DEPTH_COMPONENT;

    default:
        assert(0);
        return 0;
    }
}


/** Get the size in bytes of a texel. For compressed formats,
  * return the size of a block.
  */
static unsigned int
BytesPerPixel(TextureMap::ImageFormat format)
{
    switch (format)
    {
    case TextureMap::R8G8B8:
    case TextureMap::B8G8R8:
        return 3;

    case TextureMap::R8G8B8A8:
    case TextureMap::B8G8R8A8:
        return 4;

    case TextureMap::DXT1:
        return 8;

    case TextureMap::DXT3:
    case TextureMap::DXT5:
        return 16;

    default:
        return 0;
    }
}


static GLenum
ToGlWrap(TextureProperties::AddressMode addressMode)
{
    switch (addressMode)
    {
    case TextureProperties::Wrap:
        return GL_REPEAT;
    case TextureProperties::Clamp:
        return GL_CLAMP_TO_EDGE_EXT;
    default:
        return 0;
    }
}


/** Create a default texture properties object.
  *
  * s-coordinate addressing: wrap
  * t-coordinate addressing: wrap
  */
TextureProperties::TextureProperties() :
    addressS(Wrap),
    addressT(Wrap),
    usage(ColorTexture),
    useMipmaps(true)
{
}


/** Create a new texture properties with the specified address mode
  * used for both the s and t coordinates.
  */
TextureProperties::TextureProperties(TextureProperties::AddressMode stAddress) :
    addressS(stAddress),
    addressT(stAddress),
    usage(ColorTexture),
    useMipmaps(true)
{
}


TextureMap::TextureMap(const string& name, TextureMapLoader* loader) :
    m_status(Uninitialized),
    m_id(0),
    m_memoryUsage(0),
    m_loader(loader),
    m_name(name),
    m_lastUsed(0)
{
}


TextureMap::TextureMap(const string& name, TextureMapLoader* loader, const TextureProperties& properties) :
    m_status(Uninitialized),
    m_id(0),
    m_memoryUsage(0),
    m_loader(loader),
    m_name(name),
    m_properties(properties),
    m_lastUsed(0)
{
}


/** Construct a new texture map object that wraps an OpenGL texture handle. This is
  * useful when the texture doesn't need to be created via a texture loader. The
  * TextureMap instance takes ownership of the texture handle and will call GL to
  * delete it in the destructor.
  */
TextureMap::TextureMap(unsigned int glTexId, const TextureProperties& properties) :
    m_status(Ready),
    m_id(glTexId),
    m_memoryUsage(0),
    m_loader(0),
    m_properties(properties),
    m_lastUsed(0)
{
}


/** Construct a new texture map object that wraps an OpenGL texture handle. This is
  * useful when the texture doesn't need to be created via a texture loader. The
  * TextureMap instance takes ownership of the texture handle and will call GL to
  * delete it in the destructor. Texture properties are not modified.
  */
TextureMap::TextureMap(unsigned int glTexId) :
    m_status(Ready),
    m_id(glTexId),
    m_memoryUsage(0),
    m_loader(0),
    m_lastUsed(0)
{
}


TextureMap::~TextureMap()
{
    if (m_id)
    {
        glDeleteTextures(1, &m_id);
    }
}


/** Load the texture map and return true if it's ready to be used for rendering.
  * The texture may not be immediately available if it has an asynchronous
  * loader. The call to makeResident() has no effect if the texture is already
  * loaded.
  */
bool
TextureMap::makeResident()
{
    if (m_loader)
    {
        if (status() == Uninitialized)
        {
            m_loader->makeResident(this);
        }
        m_lastUsed = m_loader->frameCount();
    }

    return isResident();
}


/** Generate a texture map without initializing the texture data.
  */
bool
TextureMap::generate(unsigned int width, unsigned int height, ImageFormat format)
{
    GLenum glImageFormat = ToGlFormat(format);
    if (glImageFormat == 0)
    {
        VESTA_LOG("Bad image format provided to TextureMap::generate()");
        setStatus(LoadingFailed);
        return false;
    }

    GLenum glInternalFormat = ToGlInternalFormat(format);

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToGlWrap(m_properties.addressS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToGlWrap(m_properties.addressT));

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 (GLint) glInternalFormat,
                 width, height,
                 0,
                 glImageFormat,
                 GL_UNSIGNED_BYTE,
                 NULL);

    GLint minFilter = m_properties.useMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);

    setStatus(Ready);

    return true;
}


/** Realize this texture on the GPU using the specified image data.
  * Mipmaps will be generated automatically if the useMipmaps property
  * was set when the texture was constructed.
  */
bool
TextureMap::generate(const unsigned char imageData[],
                     unsigned int imageDataSize,
                     unsigned int width,
                     unsigned int height,
                     ImageFormat format)
{
    GLenum glImageFormat = ToGlFormat(format);
    if (glImageFormat == 0)
    {
        VESTA_LOG("Bad image format provided to TextureMap::generate()");
        setStatus(LoadingFailed);
        return false;
    }

    GLenum glInternalFormat = ToGlInternalFormat(format);

    // Verify that imageData is large enough to hold the entire texture
    if (imageDataSize < width * height * BytesPerPixel(format))
    {
        VESTA_LOG("Incomplete image data provided to TextureMap::generate()");
        setStatus(LoadingFailed);
        return false;
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToGlWrap(m_properties.addressS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToGlWrap(m_properties.addressT));

    m_memoryUsage = width * height * BytesPerPixel(format);

    if (m_properties.useMipmaps)
    {
        if (GLEW_EXT_framebuffer_object)
        {
            // Fast path uses glGenerateMipmap() when driver/hardware supports it.
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         (GLint) glInternalFormat,
                         width, height,
                         0,
                         glImageFormat,
                         GL_UNSIGNED_BYTE,
                         imageData);

            glGenerateMipmapEXT(GL_TEXTURE_2D);
        }
        else
        {
            // Legacy path for hardware that doesn't have ARB_framebuffer_object. Slower than
            // glGenerateMipmap() and doesn't support as many texture features.
            gluBuild2DMipmaps(GL_TEXTURE_2D,
                              (GLint) glInternalFormat,
                              width, height,
                              glImageFormat,
                              GL_UNSIGNED_BYTE,
                              imageData);
        }

        // A complete mipmap chain uses about 1/3 more memory
        m_memoryUsage += m_memoryUsage / 3;
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     (GLint) glInternalFormat,
                     width, height,
                     0,
                     glImageFormat,
                     GL_UNSIGNED_BYTE,
                     imageData);        
    }

    GLint minFilter = m_properties.useMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);

    setStatus(Ready);

    return true;
}


/** Realize this texture on the GPU using the specified compressed image data.
  * Mipmaps will be used if the level count is set to > 1. Note that mipmaps for
  * block compressed textures are not generated automatically; they are only
  * enabled when pregenerated mipmaps are provided.
  */
bool
TextureMap::generateCompressed(const char compressedImageData[],
                               unsigned int imageDataSize,
                               unsigned int width,
                               unsigned int height,
                               ImageFormat format,
                               unsigned int mipLevelCount)
{
    unsigned int mipChainSize = MipmapChainSize(format, width, height, mipLevelCount);
    if (mipChainSize > imageDataSize)
    {
        VESTA_LOG("Incomplete compressed image data provided to TextureMap::generateCompressed()");
        setStatus(LoadingFailed);
        return false;
    }

    if (GLEW_ARB_texture_compression == GL_FALSE)
    {
        VESTA_LOG("Attempted to create compressed texture, but hardware doesn't support the feature.");
        setStatus(LoadingFailed);
        return false;
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    GLenum glInternalFormat = ToGlFormat(format);

    unsigned int mipLevelOffset = 0;

    for (unsigned int level = 0; level < mipLevelCount; ++level)
    {
        unsigned int mipLevelWidth  = max(width >> level, 1u);
        unsigned int mipLevelHeight = max(height >> level, 1u);
        unsigned int mipLevelSize = MipmapLevelSize(format, width, height, level);

        glCompressedTexImage2DARB(GL_TEXTURE_2D,
                                  level,
                                  (GLint) glInternalFormat,
                                  mipLevelWidth, mipLevelHeight,
                                  0,
                                  mipLevelSize,
                                  compressedImageData + mipLevelOffset);

        // Advance to the next mip level
        mipLevelOffset += mipLevelSize;
    }

    if (mipLevelCount <= 1)
    {
        m_properties.useMipmaps = false;
    }
    applyProperties(m_properties);

    setStatus(Ready);
    m_memoryUsage = mipLevelOffset;

    return true;
}


/** Release the graphics memory used by the texture and mark it as uninitialized.
  */
void
TextureMap::evict()
{
    if (m_id)
    {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
    m_memoryUsage = 0;
    setStatus(Uninitialized);
}


static void ApplyTextureProperties(const TextureProperties& properties,
                                   GLenum target)
{
    glTexParameteri(target, GL_TEXTURE_WRAP_S, ToGlWrap(properties.addressS));
    glTexParameteri(target, GL_TEXTURE_WRAP_T, ToGlWrap(properties.addressT));

    GLint minFilter = properties.useMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);

    if (properties.usage == TextureProperties::DepthTexture)
    {
        glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexParameteri(target, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    }
}


// Apply texture filtering and addressing properties
void
TextureMap::applyProperties(const TextureProperties& properties)
{
    if (m_id != 0)
    {
        glBindTexture(GL_TEXTURE_2D, m_id);
        ApplyTextureProperties(properties, GL_TEXTURE_2D);
    }
}


/** Get the size in bytes of a mipmap level with the specified format and dimensions.
  */
unsigned int
TextureMap::MipmapLevelSize(ImageFormat format, unsigned int width, unsigned int height)
{
    unsigned int blockWidth = 1;
    unsigned int blockHeight = 1;
    switch (format)
    {
    case DXT1:
        blockWidth = 4;
        blockHeight = 4;
        break;

    case DXT3:
    case DXT5:
        blockWidth = 4;
        blockHeight = 4;
        break;

    default:
        break;
    }

    unsigned int widthBlocks = (width + blockWidth - 1) / blockWidth;
    unsigned int heightBlocks = (height + blockHeight - 1) / blockHeight;

    return widthBlocks * heightBlocks * BytesPerPixel(format);
}


/** Get the size in bytes of the mipmap level of a texture with the specified base
  * (mipmap level 0) width and height.
  */
unsigned int
TextureMap::MipmapLevelSize(ImageFormat format, unsigned int baseWidth, unsigned int baseHeight, unsigned int level)
{
    unsigned int mipLevelWidth  = max(baseWidth >> level, 1u);
    unsigned int mipLevelHeight = max(baseHeight >> level, 1u);

    return MipmapLevelSize(format, mipLevelWidth, mipLevelHeight);
}


/** Get the size in bytes of a mipmap chain with the specified length and base texture dimensions.
  */
unsigned int
TextureMap::MipmapChainSize(ImageFormat format, unsigned int baseWidth, unsigned int baseHeight, unsigned int levelCount)
{
    unsigned int size = 0;
    for (unsigned int i = 0; i < levelCount; ++i)
    {
        size += MipmapLevelSize(format, baseWidth, baseHeight, i);
    }

    return size;
}


/** Factory method for creating a depth texture.
  * The contents of the depth texture are not initialized and will contain
  * undefined image data.
  *
  * \param width the width of the texture in pixels
  * \param height the height of the texture in pixels
  * \param format a valid depth texture format (currently just Depth24 is allowed)
  *
  * \return either a valid, fully constructed depth texture or NULL if there was
  * an error.
  */
TextureMap*
TextureMap::CreateDepthTexture(unsigned int width, unsigned int height, ImageFormat format)
{
    if (format != Depth24)
    {
        VESTA_WARNING("Invalid depth texture format requested.");
        return NULL;
    }

    GLuint depthTexId = 0;
    glGenTextures(1, &depthTexId);
    if (depthTexId == 0)
    {
        VESTA_WARNING("Failed to create depth texture handle.");
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, depthTexId);

    // Allocate the texture
    glTexImage2D(GL_TEXTURE_2D, 0, ToGlInternalFormat(format), width, height, 0, ToGlFormat(format), GL_UNSIGNED_BYTE, 0);

    // Unbind it
    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum errorCode = glGetError();
    if (errorCode != GL_NO_ERROR)
    {
        const GLubyte* errorMessage = gluErrorString(errorCode);
        if (errorMessage)
        {
            VESTA_WARNING("OpenGL error occurred when creating depth texture: %s", errorMessage);
            glDeleteTextures(1, &depthTexId);
            return NULL;
        }
    }

    // GL_NEAREST is usually the appropriate filtering for depth textures. However,
    // NVIDIA GPUs (and possibly others) perform 'free' 4x percentage close filtering
    // when the filter is set to GL_LINEAR.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    TextureProperties texProps;
    texProps.addressS = TextureProperties::Clamp;
    texProps.addressT = TextureProperties::Clamp;
    texProps.useMipmaps = false;
    texProps.usage = TextureProperties::DepthTexture;

    TextureMap* tex = new TextureMap(depthTexId, texProps);
    if (tex)
    {
        glBindTexture(GL_TEXTURE_2D, tex->id());
        ApplyTextureProperties(texProps, GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    return tex;
}


/** Factory method for creating a cube map with six faces that are size x size texels.
  * The contents of the cube map are not initialized and will contain undefined image
  * data.
  *
  * \param width the width of the texture in pixels
  * \param height the height of the texture in pixels
  * \param format a valid color format
  *
  * \return either a valid, fully constructed cube map or NULL if there was
  * an error.
  */
TextureMap*
TextureMap::CreateCubeMap(unsigned int size, ImageFormat format)
{
    GLuint cubeMapId = 0;
    glGenTextures(1, &cubeMapId);
    if (cubeMapId == 0)
    {
        VESTA_WARNING("Failed to create cube map handle.");
        return NULL;
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, cubeMapId);

    // Set the dimensions for all faces
    for (int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + i, 0, ToGlInternalFormat(format), size, size, 0, ToGlFormat(format), GL_UNSIGNED_BYTE, NULL);
    }

    // Unbind it
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 0);

    GLenum errorCode = glGetError();
    if (errorCode != GL_NO_ERROR)
    {
        const GLubyte* errorMessage = gluErrorString(errorCode);
        if (errorMessage)
        {
            VESTA_WARNING("OpenGL error occurred when creating cube map texture: %s", errorMessage);
            glDeleteTextures(1, &cubeMapId);
            return NULL;
        }
    }

    TextureProperties texProps;
    texProps.addressS = TextureProperties::Clamp;
    texProps.addressT = TextureProperties::Clamp;
    texProps.useMipmaps = false;

    TextureMap* tex = new TextureMap(cubeMapId, texProps);
    if (tex)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, tex->id());
        ApplyTextureProperties(texProps, GL_TEXTURE_CUBE_MAP_ARB);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 0);
    }

    return tex;
}