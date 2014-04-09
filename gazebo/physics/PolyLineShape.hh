/*
 * Copyright (C) 2012-2014 Open Source Robotics Foundation
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
#ifndef _POLYLINESHAPE_HH_
#define _POLYLINESHAPE_HH_

#include <vector>

#include "gazebo/physics/Shape.hh"

namespace gazebo
{
  namespace physics
  {
    /// \addtogroup gazebo_physics
    /// \{

    /// \class PolyLineShape PolyLineShape.hh physics/physcs.hh
    /// \brief PolyLine geometry primitive.
    class PolyLineShape : public Shape
    {
      /// \brief Constructor.
      /// \param[in] _parent Parent Collision.
      public: explicit PolyLineShape(CollisionPtr _parent);

      /// \brief Destructor.
      public: virtual ~PolyLineShape();

      /// \brief Initialize the polyLine.
      public: virtual void Init();

      /// \brief Set the height of the polyLine.
      /// \param[in] _height Height of the polyLine.
      public: virtual void SetHeight(const double &_height);

      /// \brief Set the scale of the polyLine.
      /// \param[in] _scale Scale of the polyLine.
      public: virtual void SetScale(const math::Vector3 &_scale);

      /// \brief Set the vertices of the polyline
      /// \param[in] _vertices std::vector<math::Vector2d>
      /// containing the vertex information
      public: virtual void SetVertices(const std::vector<math::Vector2d>
                                             &_vertices);

      /// \brief Set the vertices of the polyline
      /// \param[in] _msg geometry msg containing the vertex information
      public: virtual void SetVertices(const msgs::Geometry &_msg);

      /// \brief Get the vertices of the polyline
      /// \return The vertex information of the polyline
      public: std::vector<math::Vector2d> GetVertices() const;

      /// \brief Get the height of the polyLine.
      /// \return The height of each side of the polyLine.
      public: double GetHeight() const;

      /// \brief Set the parameters of polyline shape
      /// \param[in] _height Height of the polygon
      /// \param[in] _vertices std::vector<math::Vector2d>
      /// containing the vertex information
      public: void SetPolylineShape(const double &_height,
                                    const std::vector<math::Vector2d>
                                          &_vertices);

      /// \brief Fill in the values for a geomertry message.
      /// \param[out] _msg The geometry message to fill.
      public: void FillMsg(msgs::Geometry &_msg);

      /// \brief Process a geometry message.
      /// \param[in] _msg The message to set values from.
      public: virtual void ProcessMsg(const msgs::Geometry &_msg);
    };
    /// \}
  }
}
#endif
