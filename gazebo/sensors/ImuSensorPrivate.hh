/*
 * Copyright (C) 2015-2016 Open Source Robotics Foundation
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
#ifndef _GAZEBO_SENSORS_IMUSENSOR_PRIVATE_HH_
#define _GAZEBO_SENSORS_IMUSENSOR_PRIVATE_HH_

#include <array>
#include <mutex>
#include <string>
#include <ignition/math/Vector3.hh>
#include <ignition/math/Pose3.hh>

#include "gazebo/physics/PhysicsTypes.hh"
#include "gazebo/transport/TransportTypes.hh"

namespace gazebo
{
  namespace sensors
  {
    /// \internal
    /// \brief Imu sensor private data.
    class ImuSensorPrivate
    {
      /// \brief type of IMU. Options are specified in:
      /// http://bitbucket.org/sdformat/src/default/sdf/1.6/imu.sdf
      public: std::string referenceFrame;

      /// \brief transform from world frame to Imu reference frame.
      public: ignition::math::Pose3d worldToReference;

      /// \brief Save previous imu linear velocity in the xxx frame
      /// for computing acceleration in xxx frame
      public: ignition::math::Vector3d lastImuWorldLinearVel;

      /// \brief Noise free linear acceleration
      public: ignition::math::Vector3d linearAcc;

      /// \brief store gravity vector to be added to the imu output.
      public: ignition::math::Vector3d gravity;

      /// \brief Imu data publisher
      public: transport::PublisherPtr pub;

      /// \brief Subscriber to link data published by parent entity
      public: transport::SubscriberPtr linkDataSub;

      /// \brief Parent entity which the IMU is attached to
      public: physics::LinkPtr parentEntity;

      /// \brief Imu message
      public: msgs::IMU imuMsg;

      /// \brief Mutex to protect reads and writes.
      public: mutable std::mutex mutex;

      /// \brief Buffer for storing link data
      public: std::array<boost::shared_ptr<msgs::LinkData const>, 2>
              incomingLinkData;

      /// \brief Index for accessing element in the link data array
      public: unsigned int dataIndex;

      /// \brief True if new link data is received
      public: bool dataDirty;

      /// \brief Noise free angular velocity.
      public: ignition::math::Vector3d angularVel;
    };
  }
}
#endif
