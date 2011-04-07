/*
 * Copyright 2011 Nate Koenig & Andrew Howard
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
#ifndef RENDERCONTROL_HH
#define RENDERCONTROL_HH

#include <wx/wx.h>

#include "common/Event.hh"

#include "gui/SphereMaker.hh"
#include "gui/BoxMaker.hh"
#include "gui/CylinderMaker.hh"
#include "gui/PointLightMaker.hh"
#include "gui/SpotLightMaker.hh"
#include "gui/DirectionalLightMaker.hh"

#include "rendering/RenderTypes.hh"

#include "common/MouseEvent.hh"

namespace gazebo
{
  namespace rendering
  {
    class UserCamera;
  }

	namespace gui
  {
  
    class RenderControl : public wxControl
    {
      DECLARE_CLASS(RenderControl)
      DECLARE_EVENT_TABLE()
  
      /// \brief Constructor
      public: RenderControl(wxWindow *parent);
  
      /// \brief Destructor
      public: virtual ~RenderControl();
  
      /// \brief Get an ogre window handle
      public: std::string GetOgreHandle() const;
  
      public: bool Reparent(wxWindowBase* new_parent);
  
      public: wxSize DoGetBestSize () const;
  
      public: int GetWidth();
      public: int GetHeight();
  
      public: void OnMouseEvent( wxMouseEvent &event);
  
      public: void OnKeyUp( wxKeyEvent &event);
      public: void OnKeyDown( wxKeyEvent &event);
  
      public: void Init();
  
      /// \brief Create the camera
      //public: void CreateCamera(rendering::Scene *scene);
      public: void ViewScene(rendering::ScenePtr scene);
  
      /// \brief Get the camera
      public: rendering::UserCamera *GetCamera();
  
      /// \brief Get the cursor state
      public: std::string GetCursorState() const;
  
      /// \brief Set the state of the cursor
      public: void SetCursorState(const std::string &state);
  
      protected: virtual void OnSize( wxSizeEvent &evt );
  
      private: void CreateEntity(std::string name);
  
      private: void ManipModeCB(bool mode);
      private: void MoveModeCB(bool mode);
  
      private: void SetSelectedEntityCB(const std::string &name);
  
      /// \brief Rotate and entity, or apply torque
      //private: void EntityRotate(Entity *entity);
  
      /// \brief Translate an entity, or apply force
      //private: void EntityTranslate(Entity *entity);
  
      private: rendering::UserCamera *userCamera;
  
      private: int windowId;
  
      private: common::MouseEvent mouseEvent;
  
      private: EntityMaker *currMaker;
      private: std::string cursorState;
      private: std::string mouseModifier;
  
      private: CylinderMaker cylinderMaker;
      private: BoxMaker boxMaker;
      private: SphereMaker sphereMaker;
      private: PointLightMaker pointLightMaker;
      private: SpotLightMaker spotLightMaker;
      private: DirectionalLightMaker directionalLightMaker;
  
      private: std::vector<event::ConnectionPtr> connections;
    };
  }
}
#endif
