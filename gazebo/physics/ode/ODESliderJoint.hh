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
/* Desc: A slider or primastic joint
 * Author: Nate Koenig, Andrew Howard
 * Date: 21 May 2003
 */

#ifndef _ODESLIDERJOINT_HH_
#define _ODESLIDERJOINT_HH_

#include "gazebo/physics/SliderJoint.hh"
#include "gazebo/physics/ode/ODEJoint.hh"
#include "gazebo/util/system.hh"

namespace gazebo
{
  namespace physics
  {
    /// \brief A slider joint
    class GZ_PHYSICS_ODE_VISIBLE ODESliderJoint : public SliderJoint<ODEJoint>
    {
      /// \brief Constructor
      /// \param[in] _worldId ODE world id.
      /// \param[in] _parent Pointer to the Link that is the joint' parent
      public: ODESliderJoint(dWorldID _worldId, BasePtr _parent);

      /// \brief Destructor
      public: virtual ~ODESliderJoint();

      // Documentation inherited
      public: virtual void Load(sdf::ElementPtr _sdf);

      // Documentation inherited
      public: virtual math::Vector3 GetAnchor(unsigned int _index) const;

      // Documentation inherited
      public: virtual void SetAnchor(unsigned int _index,
                                     const math::Vector3 &_anchor);

      // Documentation inherited
      public: virtual math::Vector3 GetGlobalAxis(unsigned int _index) const;

      // Documentation inherited
      public: virtual void SetAxis(unsigned int _index,
                  const math::Vector3 &_axis);

      // Documentation inherited
      public: virtual math::Angle GetAngleImpl(unsigned int _index) const;

      // Documentation inherited
      public: virtual double GetVelocity(unsigned int _index) const;

      // Documentation inherited
      public: virtual void SetVelocity(unsigned int _index, double _angle);

      // Documentation inherited
      public: virtual void SetMaxForce(unsigned int _index, double _t);

      // Documentation inherited
      public: virtual double GetMaxForce(unsigned int _index);

      // Documentation inherited
      public: virtual double GetParam(unsigned int _parameter) const;

      // Documentation inherited
      public: virtual void SetParam(unsigned int _parameter, double _value);

      // Documentation inherited
      protected: virtual void SetForceImpl(unsigned int _index, double _effort);
    };
  }
}
#endif
