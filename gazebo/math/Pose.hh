/*
 * Copyright 2011 Nate Koenig
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
/* Desc: External interfaces for Gazebo
 * Author: Nate Koenig
 * Date: 03 Apr 2007
 */

#ifndef POSE_HH
#define POSE_HH

#include <iostream>

#include "math/Vector3.hh"
#include "math/Quaternion.hh"

namespace gazebo
{
  namespace math
  {
    /// \addtogroup gazebo_math
    /// \{

    /// \brief Encapsulates a position and rotation in three space
    class Pose
    {
      /// \brief Default constructors
      public: Pose();

      /// \brief Constructor
      /// \param pos A position
      /// \param rot A rotation
      public: Pose(const Vector3 &_pos, const Quaternion &_rot);

      /// \brief Constructor
      public: Pose(double _x, double _y, double _z,
                   double _roll, double _pitch, double _yaw);

      /// \brief Copy constructor
      /// \param pose Pose to copy
      public: Pose(const Pose &_pose);

      /// \brief Destructor
      public: virtual ~Pose();

      /// \brief Set the pose from a Vector3 and a Quaternion
      /// \param[in] _pos The position.
      /// \param[in] _rot The rotation.
      public: void Set(const Vector3 &_pos, const Quaternion &_rot);

      /// \brief Set the pose from a six tuple.
      /// \param[in] _x x position in meters.
      /// \param[in] _y y position in meters.
      /// \param[in] _z z position in meters.
      /// \param[in] _roll Roll (rotation about X-axis) in radians.
      /// \param[in] _pitch Pitch (rotation about y-axis) in radians.
      /// \param[in] _yaw Pitch (rotation about z-axis) in radians.
      public: void Set(double _x, double _y, double _z,
                       double _roll, double _pitch, double _yaw);

      /// \brief See if a pose is finite (e.g., not nan)
      public: bool IsFinite() const;

      /// \brief Fix any nan values
      public: inline void Correct()
              {
                this->pos.Correct();
                this->rot.Correct();
              }

      /// \brief Get the inverse of this pose
      /// \return the inverse pose
      public: Pose GetInverse() const;

      /// \brief Addition operator
      /// \param[in] pose Pose to add to this pose
      /// \return The resulting pose
      public: Pose operator+(const Pose &_pose) const;

      /// \brief Add-Equals operator
      /// \param[in] pose Pose to add to this pose
      /// \return The resulting pose
      public: const Pose &operator+=(const Pose &_pose);

      /// \brief Subtraction operator
      /// \param[in] pose Pose to subtract from this one
      /// \return The resulting pose
      public: inline Pose operator-(const Pose &_pose) const
              {
                return Pose(this->CoordPositionSub(_pose),
                            this->CoordRotationSub(_pose.rot));
              }

      /// \brief Subtraction operator
      /// \param[in] pose Pose to subtract from this one
      /// \return The resulting pose
      public: const Pose &operator-=(const Pose &_pose);

      /// \brief Equality operator
      /// \param[in] _pose Pose for comparison
      /// \return True if equal
      public: bool operator ==(const Pose &_pose) const;

      /// \brief Inequality operator
      /// \param[in] _pose Pose for comparison
      /// \return True if not equal
      public: bool operator!=(const Pose &_pose) const;

      /// \brief Multiplication operator
      /// \param[in] _pose the other pose
      /// \return itself
      public: Pose operator*(const Pose &_pose);

      /// \brief Add one point to a vector: result = this + pos
      /// \param[in] pos Position to add to this pose
      /// \return the resulting position
      public: Vector3 CoordPositionAdd(const Vector3 &_pos) const;

      /// \brief Add one point to another: result = this + pose
      /// \param[in] _pose The Pose to add
      /// \return The resulting position
      public: Vector3 CoordPositionAdd(const Pose &_pose) const;

      /// \brief Subtract one position from another: result = this - pose
      /// \param[in] _pose Pose to subtract
      /// \return The resulting position
      public: inline Vector3 CoordPositionSub(const Pose &_pose) const
              {
                Quaternion tmp(0.0,
                    this->pos.x - _pose.pos.x,
                    this->pos.y - _pose.pos.y,
                    this->pos.z - _pose.pos.z);

                tmp = _pose.rot.GetInverse() * (tmp * _pose.rot);
                return Vector3(tmp.x, tmp.y, tmp.z);
              }

      /// \brief Add one rotation to another: result =  this->rot + rot
      /// \param[in] _rot Rotation to add
      /// \return The resulting rotation
      public: Quaternion CoordRotationAdd(const Quaternion &_rot) const;

      /// \brief Subtract one rotation from another: result = this->rot - rot
      /// \param[in] _rot The rotation to subtract
      /// \return The resulting rotation
      public: inline Quaternion CoordRotationSub(const Quaternion &_rot) const
              {
                Quaternion result(_rot.GetInverse() * this->rot);
                result.Normalize();
                return result;
              }


      /// \brief Find the inverse of a pose; i.e., if b = this + a, given b and
      ///        this, find a
      /// \param[in] _b the other pose
      public: Pose CoordPoseSolve(const Pose &_b) const;

      /// \brief Reset the pose
      public: void Reset();

      /// \brief Rotate vector part of a pose about the origin
      /// \param[in] _rot rotation
      /// \return the rotated pose
      public: Pose RotatePositionAboutOrigin(const Quaternion &_rot) const;

      /// \brief Round all values to _precision decimal places
      /// \param[in] _precision
      public: void Round(int _precision);

      /// \brief The position
      public: Vector3 pos;

      /// \brief The rotation
      public: Quaternion rot;

      /// \brief Stream insertion operator
      /// \param out output stream
      /// \param pose pose to output
      /// \return the stream
      public: friend std::ostream &operator<<(std::ostream &_out,
                                              const gazebo::math::Pose &_pose)
              {
                _out << _pose.pos << " " << _pose.rot;
                return _out;
              }

      /// \brief Stream extraction operator
      /// \param[in] _in the input stream
      /// \param[in] _pose the pose
      /// \return the stream
      public: friend std::istream &operator>>(std::istream &_in,
                gazebo::math::Pose &_pose)
            {
              // Skip white spaces
              _in.setf(std::ios_base::skipws);
              _in >> _pose.pos >> _pose.rot;
              return _in;
            }
    };
    /// \}
  }
}
#endif
