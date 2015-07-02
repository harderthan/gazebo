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

#ifndef _GAZEBO_VIEW_ANGLE_WIDGET_HH_
#define _GAZEBO_VIEW_ANGLE_WIDGET_HH_

#include <string>

#include "gazebo/math/Vector3.hh"

#include "gazebo/gui/qt.h"
#include "gazebo/util/system.hh"

namespace gazebo
{
  namespace gui
  {
    class ViewAngleWidgetPrivate;

    /// \brief A gui widget for changing the camera view angle.
    class GAZEBO_VISIBLE ViewAngleWidget : public QWidget
    {
      Q_OBJECT

      public: enum Angle
      {
        /// Top
        TOP,
        /// Bottom
        BOTTOM,
        /// Front
        FRONT,
        /// Back
        BACK,
        /// Left
        LEFT,
        /// Right
        RIGHT,
        /// Reset view
        RESET
      };

      /// \brief Constructor
      /// \param[in] _parent Parent Qt widget.
      public: ViewAngleWidget(QWidget *_parent = 0);

      /// \brief Destructor
      public: virtual ~ViewAngleWidget();

      /// \brief Add an action to the widget.
      /// \param[in] _angle View angle.
      /// \param[in] _action The Qt action to add.
      public: void Add(Angle _angle, QAction *_action);

      /// \brief Move the camera to face the given direction. In case there's
      /// an object immediately in front of the camera, the camera will
      /// continue facing the object, with about the same distance (zoom). If
      /// there's no object, the camera will face the world origin. The movement
      /// takes 1 second.
      /// \param[in] _dir Direction for the camera to face.
      private: void MoveCamera(math::Vector3 _dir);

      private: void showEvent(QShowEvent *_event);

      /// \brief Qt callback when the top view is triggered.
      private slots: void OnTopView();

      /// \brief Qt callback when the bottom view is triggered.
      private slots: void OnBottomView();

      /// \brief Qt callback when the front view is triggered.
      private slots: void OnFrontView();

      /// \brief Qt callback when the back view is triggered.
      private slots: void OnBackView();

      /// \brief Qt callback when the left view is triggered.
      private slots: void OnLeftView();

      /// \brief Qt callback when the right view is triggered.
      private slots: void OnRightView();

      /// \brief Qt callback when the reset view is triggered.
      private slots: void OnResetView();

      /// \brief QT Callback that turns on orthographic projection
      private slots: void OnOrtho();

      /// \brief QT Callback that turns on perspective projection
      private slots: void OnPerspective();

      /// \brief Qt callback when the reset view is triggered.
      private slots: void OnProjection(int _index);

      /// \internal
      /// \brief Pointer to private data.
      private: ViewAngleWidgetPrivate *dataPtr;
    };
  }
}
#endif
