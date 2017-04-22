#pragma once
#include "icg_helper.h"

class FrameBuffer {

    private:
        int width;
        int height;
        bool depthEnabled;
        GLuint framebufferObjectId;
        GLuint depthRenderBufferId;
        GLuint colorTextureId;

    public:
        // warning: overrides viewport!!
        void Bind() {
            glViewport(0, 0, width, height);
            glBindFramebuffer(GL_FRAMEBUFFER, framebufferObjectId);
            const GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
            glDrawBuffers(1 /*length of buffers[]*/, buffers);
        }

        void Unbind() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        /**
          Initialise a framebuffer object that stores a unique color attachment

          @param    internalFormat:
                    Specify how the texture components are stored.
                    This can be GL_R32F GL_RG32F GL_RGB32F GL_RGBA32F

          @param    format:
                    Specify how many channels the texture stores.
                    This can be GL_RED GL_RG GL_RGB GL_RGBA

          @param    enableDepthChannel:

          @param    useInterpolation:
         */
        int Init(int imageWidth, int imageHeight,
                 GLint internalFormat, GLint format,
                 bool enableDepthChannel = false, bool useInterpolation = false) {
            this->width = imageWidth;
            this->height = imageHeight;
            this->depthEnabled = enableDepthChannel;

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
                             format, GL_FLOAT, NULL);
            }

            // create render buffer (for depth channel)
            if(depthEnabled){
                glGenRenderbuffers(1, &depthRenderBufferId);
                glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBufferId);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
            } else {
                depthRenderBufferId = 0;
            }

            // tie it all together
            {
                glGenFramebuffers(1, &framebufferObjectId);
                glBindFramebuffer(GL_FRAMEBUFFER, framebufferObjectId);
                glFramebufferTexture2D(GL_FRAMEBUFFER,
                                       GL_COLOR_ATTACHMENT0 /*location = 0*/,
                                       GL_TEXTURE_2D, colorTextureId,
                                       0 /*level*/);

                if(enableDepthChannel){
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                          GL_RENDERBUFFER, depthRenderBufferId);
                }

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
                    GL_FRAMEBUFFER_COMPLETE) {
                    cerr << "!!!ERROR: Framebuffer not OK :(" << endl;
                }
                glBindFramebuffer(GL_FRAMEBUFFER, 0); // avoid pollution
            }

            return colorTextureId;
        }

        void Cleanup() {
            glDeleteTextures(1, &colorTextureId);
            if(depthEnabled){
                glDeleteRenderbuffers(1, &depthRenderBufferId);
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0 /*UNBIND*/);
            glDeleteFramebuffers(1, &framebufferObjectId);
        }
};
