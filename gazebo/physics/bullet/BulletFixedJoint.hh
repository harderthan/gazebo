/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef _GAZEBO_BULLETFIXEDJOINT_HH_
#define _GAZEBO_BULLETFIXEDJOINT_HH_

#include <string>
#include "gazebo/math/Angle.hh"
#include "gazebo/math/Vector3.hh"
#include "gazebo/physics/FixedJoint.hh"
#include "gazebo/physics/bullet/BulletJoint.hh"
#include "gazebo/physics/bullet/BulletPhysics.hh"
#include "gazebo/util/system.hh"

namespace gazebo
{
  namespace physics
  {
    /// \ingroup gazebo_physics
    /// \addtogroup gazebo_physics_bullet Bullet Physics
    /// \{

    /// \brief A fixed joint.
    class GZ_PHYSICS_VISIBLE BulletFixedJoint : public FixedJoint<BulletJoint>
    {
      /// \brief Constructor
      /// \param[in] world pointer to the bullet world
      /// \param[in] _parent pointer to the parent Model
      public: BulletFixedJoint(btDynamicsWorld *world, BasePtr _parent);

      /// \brief Destructor
      public: virtual ~BulletFixedJoint();

      // Documentation inherited.
      protected: virtual void Load(sdf::ElementPtr _sdf);

      // Documentation inherited.
      public: virtual void Init();

      // Documentation inherited.
      public: virtual void SetAxis(unsigned int _index,
                  const math::Vector3 &_axis);

      // Documentation inherited.
      public: virtual void SetVelocity(unsigned int _index, double _vel);

      // Documentation inherited.
      public: virtual double GetVelocity(unsigned int _index) const;

      // Documentation inherited.
      public: virtual void SetMaxForce(unsigned int _index, double _t);

      // Documentation inherited.
      public: virtual double GetMaxForce(unsigned int _index);

      // Documentation inherited.
      public: virtual bool SetHighStop(unsigned int _index,
                  const math::Angle &_angle);

      // Documentation inherited.
      public: virtual bool SetLowStop(unsigned int _index,
                  const math::Angle &_angle);

      // Documentation inherited.
      public: virtual math::Vector3 GetGlobalAxis(unsigned int _index) const;

      // Documentation inherited.
      public: virtual math::Angle GetAngleImpl(unsigned int _index) const;

      // Documentation inherited.
      protected: virtual void SetForceImpl(unsigned int _index, double _effort);

      /// \brief Pointer to bullet fixed constraint implementation
      private: btHingeConstraint *bulletFixed;
    };
    /// \}
  }
}
#endif
