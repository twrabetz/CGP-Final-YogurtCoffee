#include "FrameBuffers.hpp"

#include "check_fb.hpp"

void FrameBuffers::resize(glm::uvec2 const &drawable_size) {
    if (drawable_size == size) return;
    size = drawable_size;

    //helper to allocate a texture:
    auto alloc_tex = [&](GLuint &tex, GLenum internal_format, GLenum format, GLenum type) {
        if (tex == 0) glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, size.x, size.y, 0, format, type, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    };

    //set up position_tex as a 32-bit floating point RGB texture:
    alloc_tex(position_tex, GL_RGB32F, GL_RGB, GL_FLOAT);

    //set up normal_roughness_tex as a 16-bit floating point RGBA texture:
    alloc_tex(normal_tex, GL_RGB16F, GL_RGB, GL_FLOAT);

    //set up albedo_tex as an 8-bit fixed point RGBA texture:
    alloc_tex(albedo_tex, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);

    //set up output_tex as an 16-bit fixed point RGBA texture:
    // HDR rendering. We use 16 floating point instead.
    alloc_tex(output_tex, GL_RGBA16F, GL_RGBA, GL_FLOAT);

    //if depth_rb does not have a name, name it:
    if (depth_rb == 0) glGenRenderbuffers(1, &depth_rb);
    //set up depth_rb as a 24-bit fixed-point depth buffer:
    glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size.x, size.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    //if objects framebuffer doesn't have a name, name it and attach textures:
    if (objects_fb == 0) {
        glGenFramebuffers(1, &objects_fb);
        //set up framebuffer: (don't need to do when resizing)
        glBindFramebuffer(GL_FRAMEBUFFER, objects_fb);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, position_tex, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_tex, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedo_tex, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
        GLenum bufs[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, bufs);
        check_fb();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    //if lights-drawing framebuffer doesn't have a name, name it and attach textures:
    if (lights_fb == 0) {
        glGenFramebuffers(1, &lights_fb);
        //set up framebuffer: (don't need to do when resizing)
        glBindFramebuffer(GL_FRAMEBUFFER, lights_fb);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_tex, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
        GLenum bufs[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, bufs);
        check_fb();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

}
