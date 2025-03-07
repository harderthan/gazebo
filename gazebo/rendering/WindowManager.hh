/*
 * Copyright (C) 2012-2015 Open Source Robotics Foundation
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
#ifndef _WINDOWMANAGER_HH_
#define _WINDOWMANAGER_HH_

#ifdef _WIN32
  // Oh, yeah, CreateWindow is taken, too.
  #include <windows.h>
  #undef CreateWindow
#endif

#include <string>
#include <vector>

#include "gazebo/rendering/RenderTypes.hh"
#include "gazebo/util/system.hh"

namespace Ogre
{
  class RenderWindow;
}

namespace gazebo
{
  namespace rendering
  {
    /// \addtogroup gazebo_rendering
    /// \{

    /// \class WindowManager WindowManager.hh rendering/rendering.hh
    /// \brief Class to mangage render windows.
    class GZ_RENDERING_VISIBLE WindowManager
    {
      /// \brief Constructor
      public: WindowManager();

      /// \brief Destructor
      public: virtual ~WindowManager();

      /// \brief Shutdown all the windows
      public: void Fini();

      /// \brief Create a window.
      /// \param[in] _ogreHandle String representing the ogre window handle.
      /// \param[in] _width With of the window in pixels.
      /// \param[in] _height Height of the window in pixels.
      public: int CreateWindow(const std::string &_ogreHandle,
                               uint32_t _width,
                               uint32_t _height);

      /// \brief Attach a camera to a window.
      /// \param[in] _windowId Id of the window to add the camera to.
      /// \param[in] _camera Pointer to the camera to attach.
      public: void SetCamera(int _windowId, CameraPtr _camera);

      /// \brief Resize a window.
      /// \param[in] _id Id of the window to resize.
      /// \param[in] _width New width of the window.
      /// \param[in] _height New height of the window.
      public: void Resize(uint32_t _id, int _width, int _height);

      /// \brief Tells Ogre the window has moved, and needs updating.
      /// \param[in] _id ID of the window.
      public: void Moved(uint32_t _id);

      /// \brief Get the average FPS.
      /// \param[in] _id ID of the window.
      /// \return The frames per second.
      public: float GetAvgFPS(uint32_t _id);

      /// \brief Get the triangle count.
      /// \param[in] _id ID of the window.
      /// \return The triangle count.
      public: uint32_t GetTriangleCount(uint32_t _id);

      /// \brief Get the render window associated with the given id.
      /// \param[in] _id ID of the window.
      /// \return Pointer to the render window, NULL if the id is invalid.
      public: Ogre::RenderWindow *GetWindow(uint32_t _id);

      /// \brief All the render windows.
      private: std::vector<Ogre::RenderWindow *> windows;

      /// \brief Used to create unique names for the windows.
      private: static uint32_t windowCounter;
    };
    /// \}
  }
}
#endif


