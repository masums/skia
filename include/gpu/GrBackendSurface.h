/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBackendSurface_DEFINED
#define GrBackendSurface_DEFINED

#include "GrTypes.h"
#include "gl/GrGLTypes.h"
#include "mock/GrMockTypes.h"

#ifdef SK_VULKAN
#include "vk/GrVkTypes.h"
#include "../private/GrVkTypesPriv.h"

class GrVkImageLayout;
#endif

#if !SK_SUPPORT_GPU

// SkSurface and SkImage rely on a minimal version of these always being available
class SK_API GrBackendTexture {
public:
    GrBackendTexture() {}

    bool isValid() const { return false; }
};

class SK_API GrBackendRenderTarget {
public:
    GrBackendRenderTarget() {}

    bool isValid() const { return false; }
};
#else

class SK_API GrBackendFormat {
public:
    // Creates an invalid backend format.
    GrBackendFormat() : fValid(false) {}

    static GrBackendFormat MakeGL(GrGLenum format, GrGLenum target) {
        return GrBackendFormat(format, target);
    }

#ifdef SK_VULKAN
    static GrBackendFormat MakeVk(VkFormat format) {
        return GrBackendFormat(format);
    }
#endif

    static GrBackendFormat MakeMock(GrPixelConfig config) {
        return GrBackendFormat(config);
    }

    GrBackend backend() const {return fBackend; }

    // If the backend API is GL, these return a pointer to the format and target. Otherwise
    // it returns nullptr.
    const GrGLenum* getGLFormat() const;
    const GrGLenum* getGLTarget() const;

#ifdef SK_VULKAN
    // If the backend API is Vulkan, this returns a pointer to a VkFormat. Otherwise
    // it returns nullptr
    const VkFormat* getVkFormat() const;
#endif

    // If the backend API is Mock, this returns a pointer to a GrPixelConfig. Otherwise
    // it returns nullptr.
    const GrPixelConfig* getMockFormat() const;

    // Returns true if the backend format has been initialized.
    bool isValid() const { return fValid; }

private:
    GrBackendFormat(GrGLenum format, GrGLenum target);

#ifdef SK_VULKAN
    GrBackendFormat(const VkFormat vkFormat);
#endif

    GrBackendFormat(const GrPixelConfig config);

    GrBackend fBackend;
    bool      fValid;

    union {
        struct {
            GrGLenum fTarget; // GL_TEXTURE_2D, GL_TEXTURE_EXTERNAL or GL_TEXTURE_RECTANGLE
            GrGLenum fFormat; // the sized, internal format of the GL resource
        } fGL;
#ifdef SK_VULKAN
        VkFormat      fVkFormat;
#endif
        GrPixelConfig fMockFormat;
    };
};

class SK_API GrBackendTexture {
public:
    // Creates an invalid backend texture.
    GrBackendTexture() : fIsValid(false) {}

#if GR_TEST_UTILS
    // GrGLTextureInfo::fFormat is ignored
    // Deprecated: Should use version that does not take a GrPixelConfig instead
    GrBackendTexture(int width,
                     int height,
                     GrPixelConfig config,
                     const GrGLTextureInfo& glInfo);

    // GrGLTextureInfo::fFormat is ignored
    // Deprecated: Should use version that does not take a GrPixelConfig instead
    GrBackendTexture(int width,
                     int height,
                     GrPixelConfig config,
                     GrMipMapped,
                     const GrGLTextureInfo& glInfo);
#endif

    // The GrGLTextureInfo must have a valid fFormat.
    GrBackendTexture(int width,
                     int height,
                     GrMipMapped,
                     const GrGLTextureInfo& glInfo);

#ifdef SK_VULKAN
    GrBackendTexture(int width,
                     int height,
                     const GrVkImageInfo& vkInfo);
#endif

    GrBackendTexture(int width,
                     int height,
                     GrMipMapped,
                     const GrMockTextureInfo& mockInfo);

    GrBackendTexture(const GrBackendTexture& that);

    ~GrBackendTexture();

    GrBackendTexture& operator=(const GrBackendTexture& that);

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    bool hasMipMaps() const { return GrMipMapped::kYes == fMipMapped; }
    GrBackend backend() const {return fBackend; }

    // If the backend API is GL, copies a snapshot of the GrGLTextureInfo struct into the passed in
    // pointer and returns true. Otherwise returns false if the backend API is not GL.
    bool getGLTextureInfo(GrGLTextureInfo*) const;

#ifdef SK_VULKAN
    // If the backend API is Vulkan, copies a snapshot of the GrGLImageInfo struct into the passed
    // in pointer and returns true. This snapshot will set the fImageLayout to the current layout
    // state. Otherwise returns false if the backend API is not Vulkan.
    bool getVkImageInfo(GrVkImageInfo*) const;

    void setVkImageLayout(VkImageLayout);
#endif

    // If the backend API is Mock, copies a snapshot of the GrMockTextureInfo struct into the passed
    // in pointer and returns true. Otherwise returns false if the backend API is not Mock.
    bool getMockTextureInfo(GrMockTextureInfo*) const;

    // Returns true if the backend texture has been initialized.
    bool isValid() const { return fIsValid; }

    /**
     * Create a GrBackendFormat object that matches this texture
     */
    GrBackendFormat format() const;

#if GR_TEST_UTILS
    GrPixelConfig testingOnly_getPixelConfig() const;
    static bool TestingOnly_Equals(const GrBackendTexture& , const GrBackendTexture&);
#endif

private:
    // Friending for access to the GrPixelConfig
    friend class SkImage;
    friend class SkImage_Gpu;
    friend class SkSurface;
    friend class GrBackendTextureImageGenerator;
    friend class GrProxyProvider;
    friend class GrGpu;
    friend class GrGLGpu;
    friend class GrVkGpu;
    friend class PromiseImageHelper;

    GrPixelConfig config() const { return fConfig; }

#ifdef SK_VULKAN
   // Requires friending of GrVkGpu (done above already)
   sk_sp<GrVkImageLayout> getGrVkImageLayout() const;

   friend class GrVkTexture;
   GrBackendTexture(int width,
                    int height,
                    const GrVkImageInfo& vkInfo,
                    sk_sp<GrVkImageLayout> layout);
#endif

    // Free and release and resources being held by the GrBackendTexture.
    void cleanup();

    bool fIsValid;
    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels
    GrPixelConfig fConfig;
    GrMipMapped fMipMapped;
    GrBackend fBackend;

    union {
        GrGLTextureInfo fGLInfo;
#ifdef SK_VULKAN
        GrVkBackendSurfaceInfo fVkInfo;
#endif
        GrMockTextureInfo fMockInfo;
    };
};

class SK_API GrBackendRenderTarget {
public:
    // Creates an invalid backend texture.
    GrBackendRenderTarget() : fIsValid(false) {}

#if GR_TEST_UTILS
    // GrGLTextureInfo::fFormat is ignored
    // Deprecated: Should use version that does not take a GrPixelConfig instead
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          GrPixelConfig config,
                          const GrGLFramebufferInfo& glInfo);
#endif

    // The GrGLTextureInfo must have a valid fFormat.
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          const GrGLFramebufferInfo& glInfo);

#ifdef SK_VULKAN
    /** Deprecated, use version that does not take stencil bits. */
    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          const GrVkImageInfo& vkInfo);
    GrBackendRenderTarget(int width, int height, int sampleCnt, const GrVkImageInfo& vkInfo);
#endif

    GrBackendRenderTarget(int width,
                          int height,
                          int sampleCnt,
                          int stencilBits,
                          const GrMockRenderTargetInfo& mockInfo);

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    int sampleCnt() const { return fSampleCnt; }
    int stencilBits() const { return fStencilBits; }
    GrBackend backend() const {return fBackend; }

    // If the backend API is GL, this returns a pointer to the GrGLFramebufferInfo struct. Otherwise
    // it returns nullptr.
    const GrGLFramebufferInfo* getGLFramebufferInfo() const;

#ifdef SK_VULKAN
    // If the backend API is Vulkan, this returns a pointer to the GrVkImageInfo struct. Otherwise
    // it returns nullptr
    const GrVkImageInfo* getVkImageInfo() const;
#endif

    const GrMockRenderTargetInfo* getMockRenderTargetInfo() const;

    // Returns true if the backend texture has been initialized.
    bool isValid() const { return fIsValid; }


#if GR_TEST_UTILS
    GrPixelConfig testingOnly_getPixelConfig() const;
    static bool TestingOnly_Equals(const GrBackendRenderTarget&, const GrBackendRenderTarget&);
#endif

private:
    // Friending for access to the GrPixelConfig
    friend class SkSurface;
    friend class SkSurface_Gpu;
    friend class SkImage_Gpu;
    friend class GrGpu;
    friend class GrGLGpu;
    friend class GrProxyProvider;
    friend class GrVkGpu;
    GrPixelConfig config() const { return fConfig; }

    bool fIsValid;
    int fWidth;         //<! width in pixels
    int fHeight;        //<! height in pixels

    int fSampleCnt;
    int fStencilBits;
    GrPixelConfig fConfig;

    GrBackend fBackend;

    union {
        GrGLFramebufferInfo fGLInfo;
#ifdef SK_VULKAN
        GrVkImageInfo   fVkInfo;
#endif
        GrMockRenderTargetInfo fMockInfo;
    };
};

#endif

#endif

