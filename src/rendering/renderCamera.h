//-----------------------------------------------------------------------------
// Copyright (c) 2015 Andrew Mac
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------


#ifndef _RENDER_CAMERA_H_
#define _RENDER_CAMERA_H_

#ifndef _CONSOLEINTERNAL_H_
#include "console/consoleInternal.h"
#endif

#ifndef _RENDERING_H_
#include "rendering/rendering.h"
#endif

#ifndef BGFX_H_HEADER_GUARD
#include <bgfx/bgfx.h>
#endif

#ifndef _DEFERRED_SHADING_H_
#include "deferredShading.h"
#endif

#ifndef _TRANSPARENCY_H_
#include "rendering/transparency.h"
#endif

namespace Rendering 
{
   class DLL_PUBLIC RenderCamera;

   // Post Process
   struct DLL_PUBLIC RenderPostProcess
   {
      RenderCamera*  mCamera;
      S16            mPriority;

      RenderPostProcess() : mCamera(NULL), mPriority(0) { }

      virtual void onAddToCamera() { }
      virtual void onRemoveFromCamera() { }

      virtual void process() { }
   };

   class DLL_PUBLIC RenderCamera
   {
      protected:
         struct CommonUniforms
         {
            bgfx::UniformHandle camPos;
            bgfx::UniformHandle time;
            bgfx::UniformHandle sceneViewMat;
            bgfx::UniformHandle sceneInvViewMat;
            bgfx::UniformHandle sceneProjMat;
            bgfx::UniformHandle sceneInvProjMat;
            bgfx::UniformHandle sceneViewProjMat;
            bgfx::UniformHandle sceneInvViewProjMat;
         } mCommonUniforms;

         StringTableEntry mName;
         S16              mPriority;
         bool             mInitialized;

         bool                       mBeginEnabled;
         Graphics::Shader*          mBeginShader;
         Graphics::ViewTableEntry*  mBeginView;

         bool                       mFinishEnabled;
         Graphics::Shader*          mFinishShader;
         Graphics::ViewTableEntry*  mFinishView;
         bgfx::FrameBufferHandle    mFinishBuffer;

         StringTableEntry           mRenderTextureName;
         Vector<RenderPostProcess*> mRenderPostProcessList;

         void initBuffers();
         void destroyBuffers();
         void setCommonUniforms();

      public:
         F32 nearPlane;
         F32 farPlane;
         F32 viewMatrix[16];
         F32 projectionMatrix[16];
         F32 projectionWidth;
         F32 projectionHeight;
         Point3F position;

         DeferredShading*  mDeferredShading;
         Transparency*     mTransparency;

         RenderCamera();
         ~RenderCamera();

         // Post Processing
         U32                     mPostBufferIdx;
         bgfx::FrameBufferHandle mPostBuffers[2];
         bgfx::FrameBufferHandle getPostSource();
         bgfx::FrameBufferHandle getPostTarget();
         void flipPostBuffers();
         Graphics::ViewTableEntry* overrideBegin();
         Graphics::ViewTableEntry* overrideFinish();
         void freeBegin();
         void freeFinish();
         void addRenderPostProcess(RenderPostProcess* postProcess);
         bool removeRenderPostProcess(RenderPostProcess* postProcess);
         Vector<RenderPostProcess*>* getRenderPostProcessList();

         // Render Targets
         bgfx::FrameBufferHandle getBackBuffer();
         bgfx::TextureHandle     getColorTexture();
         bgfx::TextureHandle     getDepthTexture();
         bgfx::TextureHandle     getDepthTextureRead();
         bgfx::TextureHandle     getNormalTexture();
         bgfx::TextureHandle     getMatInfoTexture();

         StringTableEntry getName();
         void setName(StringTableEntry name);
         StringTableEntry getRenderTextureName();
         void setRenderTextureName(StringTableEntry name);
         S16 getRenderPriority();
         void setRenderPriority(S16 priority);

         virtual void render();
         virtual void postProcess();
   };
}

#endif