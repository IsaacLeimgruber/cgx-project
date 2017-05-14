#pragma once
#include "icg_helper.h"

class FrameBuffer {

protected:
    int width;
    int height;
    GLuint framebufferObjectId;

public:
    // warning: overrides viewport!!
    virtual void Bind() = 0;

    virtual void Unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void checkFrameBufferStatus(){
        switch(glCheckFramebufferStatus(GL_FRAMEBUFFER)){
        case GL_FRAMEBUFFER_COMPLETE:
            cout << "Framebuffer complete" << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            cerr << "Not all framebuffer attachment points are framebuffer" <<
                    "attachment complete. This means that at least one"<<
                    "attachment point with a renderbuffer or texture "<<
                    "attached has its attached object no longer in existence"<<
                    "or has an attached image with a width or height of "<<
                    "zero, or the color attachment point has a "<<
                    "non-color-renderable image attached, or the"<<
                    "depth attachment point has a non-depth-renderable"<<
                    "image attached, or the stencil attachment point has a"<<
                    "non-stencil-renderable image attached." << endl;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            cerr << "No images are attached to the framebuffer." << endl;
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            cerr << "The combination of internal formats of the attached"<<
                    "images violates an implementation-dependent set of"<<
                    "restrictions." << endl;
            break;
        default:
            cerr << "FRAMEBUFFER ERROR" << endl;
        }
    }

    virtual void Cleanup() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0 /*UNBIND*/);
        glDeleteFramebuffers(1, &framebufferObjectId);
    }
};

class DepthFBO: public FrameBuffer{

private:
    GLuint depthTextureId;

public:
    int Init(int imageWidth, int imageHeight,
             GLint internalFormat, GLint type){


        this->width = imageWidth;
        this->height = imageHeight;

        // create color attachment
        {
            glGenTextures(1, &depthTextureId);
            glBindTexture(GL_TEXTURE_2D, depthTextureId);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);


            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                         GL_DEPTH_COMPONENT, type, NULL);
        }

        // tie it all together
        {
            glGenFramebuffers(1, &framebufferObjectId);
            glBindFramebuffer(GL_FRAMEBUFFER, framebufferObjectId);

            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                 depthTextureId, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);

            this->checkFrameBufferStatus();

            glBindFramebuffer(GL_FRAMEBUFFER, 0); // avoid pollution
        }

        return depthTextureId;
    }

    void Bind(){
        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferObjectId);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    void Cleanup(){
        glDeleteTextures(1, &depthTextureId);
        glBindFramebuffer(GL_FRAMEBUFFER, 0 /*UNBIND*/);
        glDeleteFramebuffers(1, &framebufferObjectId);
    }
};

class ColorFBO: public FrameBuffer{

private:
    GLuint colorTextureId;
public:
    int Init(int imageWidth, int imageHeight,
             GLint internalFormat, GLint format, GLint type, bool useInterpolation){
        this->width = imageWidth;
        this->height = imageHeight;

        // create color attachment
        {
            glGenTextures(1, &colorTextureId);
            glBindTexture(GL_TEXTURE_2D, colorTextureId);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            if(useInterpolation){
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }

            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                         format, type, NULL);
        }

        // tie it all together
        {
            glGenFramebuffers(1, &framebufferObjectId);
            glBindFramebuffer(GL_FRAMEBUFFER, framebufferObjectId);

            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, colorTextureId,
                                   0 /*level*/);


            checkFrameBufferStatus();

            glBindFramebuffer(GL_FRAMEBUFFER, 0); // avoid pollution
        }

        return colorTextureId;
    }

    GLuint id() {
        return colorTextureId;
    }

    void Bind(){
        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferObjectId);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        //glReadBuffer(GL_COLOR_ATTACHMENT0);
    }

    void Cleanup() {
        glDeleteTextures(1, &colorTextureId);
        glBindFramebuffer(GL_FRAMEBUFFER, 0 /*UNBIND*/);
        glDeleteFramebuffers(1, &framebufferObjectId);
    }
};

class ColorAndDepthFBO: public FrameBuffer{

private:
    GLuint colorTextureId;
    GLuint depthRenderBufferId;

public:
    virtual void Bind() {
        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferObjectId);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    }

    int Init(int imageWidth, int imageHeight,
             GLint internalFormat, GLint format, GLint type, bool useInterpolation = false, bool mirrorRepeat = false) {
        this->width = imageWidth;
        this->height = imageHeight;

        // create color attachment
        {
            glGenTextures(1, &colorTextureId);
            glBindTexture(GL_TEXTURE_2D, colorTextureId);

            if(mirrorRepeat){
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }

            if(useInterpolation){
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }

            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                         format, type, NULL);
        }

        // create render buffer (for depth channel)

        glGenRenderbuffers(1, &depthRenderBufferId);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBufferId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);


        // tie it all together
        {
            glGenFramebuffers(1, &framebufferObjectId);
            glBindFramebuffer(GL_FRAMEBUFFER, framebufferObjectId);

            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, colorTextureId,
                                   0 /*level*/);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                      GL_RENDERBUFFER, depthRenderBufferId);

            checkFrameBufferStatus();

            glBindFramebuffer(GL_FRAMEBUFFER, 0); // avoid pollution
        }

        return colorTextureId;
    }

    GLuint getColorTexture(){
        return colorTextureId;
    }

    void Cleanup() {
        glDeleteTextures(1, &colorTextureId);
        glDeleteRenderbuffers(1, &depthRenderBufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, 0 /*UNBIND*/);
        glDeleteFramebuffers(1, &framebufferObjectId);
    }
};

class ColorAndWritableDepthFBO: public FrameBuffer{

private:
    GLuint colorTextureId;
    GLuint depthTextureId;

public:
    void Bind() {
        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferObjectId);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    }

    int Init(int imageWidth, int imageHeight,
             GLint internalFormat, GLint format,
             GLint internalDepthFormat, GLint type, bool useInterpolation = false, bool mirrorRepeat = false) {
        this->width = imageWidth;
        this->height = imageHeight;

        // create color attachment
        {
            glGenTextures(1, &colorTextureId);
            glBindTexture(GL_TEXTURE_2D, colorTextureId);

            if(mirrorRepeat){
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }

            if(useInterpolation){
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }

            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                         format, type, NULL);
        }

        // create depth attachment (can be sampled)

        {
            glGenTextures(1, &depthTextureId);
            glBindTexture(GL_TEXTURE_2D, depthTextureId);

            if(mirrorRepeat){
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }

            if(useInterpolation){
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }

            glTexImage2D(GL_TEXTURE_2D, 0, internalDepthFormat, width, height, 0,
                         GL_DEPTH_COMPONENT, type, NULL);
        }


        // tie it all together
        {
            glGenFramebuffers(1, &framebufferObjectId);
            glBindFramebuffer(GL_FRAMEBUFFER, framebufferObjectId);

            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, colorTextureId,
                                   0 /*level*/);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_DEPTH_ATTACHMENT,
                                   GL_TEXTURE_2D, depthTextureId,
                                   0);

            checkFrameBufferStatus();

            glBindFramebuffer(GL_FRAMEBUFFER, 0); // avoid pollution
        }

        return colorTextureId;
    }

    GLuint getDepthTexture(){
        return this->depthTextureId;
    }

    GLuint getColorTexture(){
        return this->colorTextureId;
    }

    void Cleanup() {
        glDeleteTextures(1, &colorTextureId);
        glDeleteRenderbuffers(1, &depthTextureId);
        glBindFramebuffer(GL_FRAMEBUFFER, 0 /*UNBIND*/);
        glDeleteFramebuffers(1, &framebufferObjectId);
    }
};
