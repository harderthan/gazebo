/*
 * Copyright (C) 2014-2015 Open Source Robotics Foundation
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
#ifndef _GAZEBO_SIMBODYPOLYLINESHAPE_HH_
#define _GAZEBO_SIMBODYPOLYLINESHAPE_HH_

#include "gazebo/physics/PolylineShape.hh"
#include "gazebo/util/system.hh"

namespace gazebo
{
  namespace physics
  {
    class SimbodyMesh;

    /// \brief Simbody polyline shape
    class GZ_PHYSICS_VISIBLE SimbodyPolylineShape : public PolylineShape
    {
      /// \brief Constructor
      /// \param[in] _parent Collision parent.
      public: explicit SimbodyPolylineShape(CollisionPtr _parent);

      /// \brief Destructor.
      public: virtual ~SimbodyPolylineShape();

      // Documentation inherited
      public: virtual void Load(sdf::ElementPtr _sdf);

      // Documentation inherited
      public: virtual void Init();

      /// \brief Simbody collsion mesh helper class.
      private: SimbodyMesh *simbodyMesh;
    };
  }
}
#endif
