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

#ifndef _VIDEO_VISUAL_HH_
#define _VIDEO_VISUAL_HH_

#include <string>
#include "gazebo/rendering/Visual.hh"
#include "gazebo/util/system.hh"

namespace gazebo
{
  namespace rendering
  {
    /// \addtogroup gazebo_rendering
    /// \{

    /// \class VideoVisual VideoVisual.hh rendering/rendering.hh
    /// \brief A visual element that displays a video as a texture
    class GZ_RENDERING_VISIBLE VideoVisual : public Visual
    {
      /// \brief Constructor
      /// \param[in] _name Name of the video visual.
      /// \param[in] _parent Parent of the video visual.
      public: VideoVisual(const std::string &_name, VisualPtr _parent);

      /// \brief Destructor
      public: virtual ~VideoVisual();

      /// \brief PreRender event callback.
      private: void PreRender();
    };
    /// \}
  }
}
#endif
