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
#ifndef _GAZEBO_RAYSENSOR_HH_
#define _GAZEBO_RAYSENSOR_HH_

#include <vector>
#include <string>

#include <ignition/math/Angle.hh>
#include <ignition/math/Pose3.hh>

#include "gazebo/math/Angle.hh"
#include "gazebo/math/Pose.hh"
#include "gazebo/transport/TransportTypes.hh"
#include "gazebo/sensors/Sensor.hh"
#include "gazebo/util/system.hh"

namespace gazebo
{
  class OgreDynamicLines;
  class Collision;
  class MultiRayShape;

  /// \ingroup gazebo_sensors
  /// \brief Sensors namespace
  namespace sensors
  {
    /// \addtogroup gazebo_sensors
    /// \{

    /// \class RaySensor RaySensor.hh sensors/sensors.hh
    /// \brief Sensor with one or more rays.
    ///
    /// This sensor cast rays into the world, tests for intersections, and
    /// reports the range to the nearest object.  It is used by ranging
    /// sensor models (e.g., sonars and scanning laser range finders).
    class GAZEBO_VISIBLE RaySensor: public Sensor
    {
      /// \brief Constructor
      public: RaySensor();

      /// \brief Destructor
      public: virtual ~RaySensor();

      // Documentation inherited
      public: virtual void Load(const std::string &_worldName);

      // Documentation inherited
      public: virtual void Init();

      // Documentation inherited
      protected: virtual bool UpdateImpl(bool _force);

      // Documentation inherited
      protected: virtual void Fini();

      // Documentation inherited
      public: virtual std::string GetTopic() const;

      /// \brief Get the minimum angle
      /// \return The minimum angle object
      /// \deprecated See AngleMin() function that returns an
      /// ignition::math::Angle object.
      public: math::Angle GetAngleMin() const GAZEBO_DEPRECATED(6.0);

      /// \brief Get the minimum angle
      /// \return The minimum angle object
      public: ignition::math::Angle AngleMin() const;

      /// \brief Get the maximum angle
      /// \return the maximum angle object
      /// \deprecated See AngleMax() function that returns an
      /// ignition::math::Angle object.
      public: math::Angle GetAngleMax() const GAZEBO_DEPRECATED(6.0);

      /// \brief Get the maximum angle
      /// \return the maximum angle object
      public: ignition::math::Angle AngleMax() const;

      /// \brief Get the angle in radians between each range
      /// \return Resolution of the angle
      public: double GetAngleResolution() const;

      /// \brief Get the minimum range
      /// \return The minimum range
      public: double GetRangeMin() const;

      /// \brief Get the maximum range
      /// \return The maximum range
      public: double GetRangeMax() const;

      /// \brief Get the range resolution
      /// \return Resolution of the range
      public: double GetRangeResolution() const;

      /// \brief Get the ray count
      /// \return The number of rays
      public: int GetRayCount() const;

      /// \brief Get the range count
      /// \return The number of ranges
      public: int GetRangeCount() const;

      /// \brief Get the vertical scan line count
      /// \return The number of scan lines vertically
      public: int GetVerticalRayCount() const;

      /// \brief Get the vertical scan line count
      /// \return The number of scan lines vertically
      public: int GetVerticalRangeCount() const;

      /// \brief Get the vertical scan bottom angle
      /// \return The minimum angle of the scan block
      /// \deprecated See VerticalAngleMin() function that returns an
      /// ignition::math::Angle object.
      public: math::Angle GetVerticalAngleMin() const GAZEBO_DEPRECATED(6.0);

      /// \brief Get the vertical scan bottom angle
      /// \return The minimum angle of the scan block
      public: ignition::math::Angle VerticalAngleMin() const;

      /// \brief Get the vertical scan line top angle
      /// \return The Maximum angle of the scan block
      /// \deprecated See VerticalAngleMax() function that returns an
      /// ignition::math::Angle object.
      public: math::Angle GetVerticalAngleMax() const GAZEBO_DEPRECATED(6.0);

      /// \brief Get the vertical scan line top angle
      /// \return The Maximum angle of the scan block
      public: ignition::math::Angle VerticalAngleMax() const;

      /// \brief Get the vertical angle in radians between each range
      /// \return Resolution of the angle
      public: double GetVerticalAngleResolution() const;

      /// \brief Get detected range for a ray.
      ///         Warning: If you are accessing all the ray data in a loop
      ///         it's possible that the Ray will update in the middle of
      ///         your access loop. This means some data will come from one
      ///         scan, and some from another scan. You can solve this
      ///         problem by using SetActive(false) <your accessor loop>
      ///         SetActive(true).
      /// \param[in] _index Index of specific ray
      /// \return Returns RangeMax for no detection.
      public: double GetRange(unsigned int _index);

      /// \brief Get all the ranges
      /// \param _ranges A vector that will contain all the range data
      public: void GetRanges(std::vector<double> &_ranges);

      /// \brief Get detected retro (intensity) value for a ray.
      ///         Warning: If you are accessing all the ray data in a loop
      ///         it's possible that the Ray will update in the middle of
      ///         your access loop. This means some data will come from one
      ///         scan, and some from another scan. You can solve this
      ///         problem by using SetActive(false) <your accessor loop>
      ///         SetActive(true).
      /// \param[in] _index Index of specific ray
      /// \return Retro (intensity) value for ray
      public: double GetRetro(unsigned int _index);

      /// \brief Get detected fiducial value for a ray.
      ///         Warning: If you are accessing all the ray data in a loop
      ///         it's possible that the Ray will update in the middle of
      ///         your access loop. This means some data will come from one
      ///         scan, and some from another scan. You can solve this
      ///         problem by using SetActive(false) <your accessor loop>
      ///         SetActive(true).
      /// \param[in] _index Index value of specific ray
      /// \return Fiducial value
      public: int GetFiducial(unsigned int _index);

      /// \brief Returns a pointer to the internal physics::MultiRayShape
      /// \return Pointer to ray shape
      public: physics::MultiRayShapePtr GetLaserShape() const
              {return this->laserShape;}

      // Documentation inherited
      public: virtual bool IsActive();

      private: physics::CollisionPtr laserCollision;
      private: physics::MultiRayShapePtr laserShape;
      private: physics::EntityPtr parentEntity;

      private: transport::PublisherPtr scanPub;
      private: boost::mutex mutex;
      private: msgs::LaserScanStamped laserMsg;
    };
    /// \}
  }
}

#endif
