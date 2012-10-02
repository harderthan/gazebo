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

/* Desc: A Light
 * Author: Nate Koenig
 * Date: 15 July 2003
 */

#ifndef _LIGHT_HH_
#define _LIGHT_HH_

#include <string>
#include <iostream>

#include "common/Event.hh"
#include "common/Color.hh"

#include "sdf/sdf.hh"

namespace Ogre
{
  class Light;
}

namespace gazebo
{
  namespace rendering
  {
    class Scene;
    class DynamicLines;

    /// \addtogroup gazebo_rendering
    /// \{

    /// \class Light Light.hh rendering/Light.hh
    /// \brief A light source.
    ///
    /// There are three types of lights: Point, Spot, and Directional. This
    /// class encapsulates all three. Point lights are light light bulbs,
    /// spot lights project a cone of light, and directional lights are light
    /// sun light.
    class Light
    {
      /// \brief Constructor.
      /// \param[in] _scene Pointer to the scene that contains the Light.
      public: Light(Scene *_scene);

      /// \brief Destructor
      public: virtual ~Light();

      /// \brief Load the light using a set of SDF parameters.
      /// \param[in] _sdf Pointer to the SDF containing the Light
      /// description.
      public: void Load(sdf::ElementPtr _sdf);

      /// \brief Load the light using default parameters.
      public: void Load();

      /// \brief Load from a light message.
      /// \param[in] _msg Containing the light information.
      public: void LoadFromMsg(ConstLightPtr &_msg);

      /// \brief Set the name of the visual.
      /// \param[in] _name Name of the light source.
      public: void SetName(const std::string &_name);

      /// \brief Get the name of the visual.
      /// \return The light's name.
      public: std::string GetName() const;

      /// \brief Get the type of the light.
      /// \return The light type: "point", "spot", "directional".
      public: std::string GetType() const;

      /// \brief Set the position of the light
      /// \param[in] _p New position for the light
      public: void SetPosition(const math::Vector3 &_p);

      /// \brief Set whether this entity has been selected by the user through
      /// the gui.
      /// \param[in] _s Set to True when the light is selected by the user.
      public: virtual bool SetSelected(bool _s);

      // \brief Toggle light visual visibility
      public: void ToggleShowVisual();

      /// \brief Set whether to show the visual
      /// \param[in] _s Set to true to draw a representation of the light.
      public: void ShowVisual(bool _s);

      /// \brief Set the light type.
      /// \param[in] _type The light type: "point", "spot", "directional"
      public: void SetLightType(const std::string &_type);

      /// \brief Set the diffuse color
      /// \param[in] _color Light diffuse color.
      public: void SetDiffuseColor(const common::Color &_color);

      /// \brief Get the diffuse color
      /// \return The light's diffuse color.
      public: common::Color GetDiffuseColor() const;

      /// \brief Set the specular color
      /// \param[in] _color The specular color
      public: void SetSpecularColor(const common::Color &_color);

      /// \brief Set the direction
      /// \param _dir Set the light's direction. Only applicable to spot and
      /// directional lights.
      public: void SetDirection(const math::Vector3 &_dir);

      /// \brief Get the direction
      /// \return The light's direction.
      public: math::Vector3 GetDirection() const;

      /// \brief Set the attenuation
      /// \param[in] _contant Constant attenuation
      /// \param[in] _linear Linear attenuation
      /// \param[in] _quadratic Quadratic attenuation
      public: void SetAttenuation(double _constant, double _linear,
                                  double _quadratic);

      /// \brief Set the spot light inner angle
      /// \param _angle Inner angle in radians
      public: void SetSpotInnerAngle(const double &_angle);

      /// \brief Set the spot light outer angle
      /// \param _angle Outer angle in radians
      public: void SetSpotOuterAngle(const double &_angle);

      /// \brief Set the spot light falloff
      /// \param _value Falloff value
      public: void SetSpotFalloff(const double &_value);

      /// \brief Set the range
      /// \param _range Rage of the light in meters.
      public: void SetRange(const double &_range);

      /// \brief Set cast shadows
      /// \param _cast Set to true to cast shadows.
      public: void SetCastShadows(const bool &_cast);

      /// \private Helper node to create a visual representation of the light
      private: void CreateVisual();

      /// \brief On pose change callback
      protected: virtual void OnPoseChange() {}

      /// The OGRE light source
      private: Ogre::Light *light;

      private: VisualPtr visual;
      private: DynamicLines *line;

      private: sdf::ElementPtr sdf;

      private: event::ConnectionPtr showLightsConnection;
      private: static unsigned int lightCounter;
      private: Scene *scene;
    };
    /// \}
  }
}
#endif
