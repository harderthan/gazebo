/*
 * Copyright (C) 2012-2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef _GAZEBO_RENDERING_DEPTHCAMERA_HH_
#define _GAZEBO_RENDERING_DEPTHCAMERA_HH_

#include <memory>
#include <string>

#include <sdf/sdf.hh>

#include "gazebo/common/CommonTypes.hh"

#include "gazebo/rendering/Camera.hh"
#include "gazebo/util/system.hh"

namespace Ogre
{
  class Material;
  class RenderTarget;
  class Texture;
  class Viewport;
}

namespace gazebo
{
  namespace rendering
  {
    // Forward declare private data.
    class DepthCameraPrivate;

    /// \addtogroup gazebo_rendering Rendering
    /// \{

    /// \class DepthCamera DepthCamera.hh rendering/rendering.hh
    /// \brief Depth camera used to render depth data into an image buffer
    class GZ_RENDERING_VISIBLE DepthCamera : public Camera
    {
      /// \brief Constructor
      /// \param[in] _namePrefix Unique prefix name for the camera.
      /// \param[in] _scene Scene that will contain the camera
      /// \param[in] _autoRender Almost everyone should leave this as true.
      public: DepthCamera(const std::string &_namePrefix,
                          ScenePtr _scene, bool _autoRender = true);

      /// \brief Destructor
      public: virtual ~DepthCamera();

      /// \brief Load the camera with a set of parmeters
      /// \param[in] _sdf The SDF camera info
      public: void Load(sdf::ElementPtr _sdf);

       /// \brief Load the camera with default parmeters
      public: void Load();

      /// \brief Initialize the camera
      public: void Init();

      /// Finalize the camera
      public: void Fini();

      /// \brief Create a texture which will hold the depth data
      /// \param[in] _textureName Name of the texture to create
      public: void CreateDepthTexture(const std::string &_textureName);

      /// \brief Render the camera
      public: virtual void PostRender();

      /// \brief All things needed to get back z buffer for depth data
      /// \return The z-buffer as a float array
      /// \deprecated See DepthData()
      public: virtual const float *GetDepthData() GAZEBO_DEPRECATED(7.0);

      /// \brief All things needed to get back z buffer for depth data
      /// \return The z-buffer as a float array
      public: virtual const float *DepthData() const;

      /// \brief Set the render target, which renders the depth data
      /// \param[in] _target Pointer to the render target
      public: virtual void SetDepthTarget(Ogre::RenderTarget *_target);

      /// \brief Connect a to the new depth image signal
      /// \param[in] _subscriber Subscriber callback function
      /// \return Pointer to the new Connection. This must be kept in scope
      public: event::ConnectionPtr ConnectNewDepthFrame(
          std::function<void (const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)>  _subscriber);

      /// \brief Disconnect from an depth image singal
      /// \param[in] _c The connection to disconnect
      public: void DisconnectNewDepthFrame(event::ConnectionPtr &_c);

      /// \brief Connect a to the new rgb point cloud signal
      /// \param[in] _subscriber Subscriber callback function
      /// \return Pointer to the new Connection. This must be kept in scope
      public: event::ConnectionPtr ConnectNewRGBPointCloud(
          std::function<void (const float *, unsigned int, unsigned int,
          unsigned int, const std::string &)>  _subscriber);

      /// \brief Disconnect from an rgb point cloud singal
      /// \param[in] _c The connection to disconnect
      public: void DisconnectNewRGBPointCloud(event::ConnectionPtr &_c);
      /// \brief Implementation of the render call
      private: virtual void RenderImpl();

      /// \brief Update a render target
      /// \param[in] _target Render target to update
      /// \param[in] _material Material to use
      /// \param[in] _matName Material name
      private: void UpdateRenderTarget(Ogre::RenderTarget *_target,
                                       Ogre::Material *_material,
                                       const std::string &_matName);

      /// \brief Pointer to the depth texture
      protected: Ogre::Texture *depthTexture;

      /// \brief Pointer to the depth target
      protected: Ogre::RenderTarget *depthTarget;

      /// \brief Pointer to the depth viewport
      protected: Ogre::Viewport *depthViewport;

      /// \internal
      /// \brief Pointer to private data.
      private: std::unique_ptr<DepthCameraPrivate> dataPtr;
    };
    /// \}
  }
}
#endif
