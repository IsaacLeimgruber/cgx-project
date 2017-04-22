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
        void Bind(bool depthAttachment = false) {
            if(depthAttachment){
                glViewport(0, 0, width, height);
                glBindFramebuffer(GL_FRAMEBUFFER, framebufferObjectId);
                glDrawBuffer(GL_NONE);
            }
            else{
                glViewport(0, 0, width, height);
                glBindFramebuffer(GL_FRAMEBUFFER, framebufferObjectId);
                const GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
                glDrawBuffers(1 /*length of buffers[]*/, buffers);
            }
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

          @param    attachment:
                    GL_COLOR_ATTACHMENT0 for a color texture or GL_DEPTH_ATTACHMENT
                    for a depth texture. Do not confuse this with the depth channel.
                    In fact, a depth texture is a depth channel that can be sampled
                    later on.

          @param    enableDepthChannel:

          @param    useInterpolation:
         */
        int Init(int imageWidth, int imageHeight,
                 GLint internalFormat, GLint format, GLint attachment,
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

                if(attachment == GL_DEPTH_ATTACHMENT){
                    glFramebufferTexture(GL_FRAMEBUFFER, attachment,
                                         colorTextureId, 0);
                    glDrawBuffer(GL_NONE);
                    glReadBuffer(GL_NONE);
                } else{
                    glFramebufferTexture2D(GL_FRAMEBUFFER,
                                           attachment,
                                           GL_TEXTURE_2D, colorTextureId,
                                           0 /*level*/);
                }

                if(enableDepthChannel){
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                          GL_RENDERBUFFER, depthRenderBufferId);
                }

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
