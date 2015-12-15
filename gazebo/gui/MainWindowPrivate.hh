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
#ifndef _MAINWINDOW_PRIVATE_HH_
#define _MAINWINDOW_PRIVATE_HH_

#include <map>
#include <string>
#include <vector>
#include <list>

#include <boost/thread/mutex.hpp>

#include "gazebo/gazebo_config.h"
#include "gazebo/gui/qt.h"
#include "gazebo/gui/DataLogger.hh"
#include "gazebo/gui/HotkeyDialog.hh"
#include "gazebo/common/Event.hh"
#include "gazebo/msgs/MessageTypes.hh"
#include "gazebo/transport/TransportTypes.hh"
#include "gazebo/util/system.hh"

#ifdef HAVE_OCULUS
#include "gazebo/gui/OculusWindow.hh"
#endif

namespace gazebo
{
  namespace gui
  {
    class InsertModelWidget;
    class RenderWidget;
    class ToolsWidget;
    class ModelListWidget;
    class Editor;
    class SpaceNav;
    class UserCmdHistory;

    class MainWindowPrivate
    {
      public: QToolBar *playToolbar;

      public: RenderWidget *renderWidget;
      public: ToolsWidget *toolsWidget;
      public: ModelListWidget *modelListWidget;

      public: transport::NodePtr node;
      public: transport::PublisherPtr worldControlPub;
      public: transport::PublisherPtr serverControlPub;
      public: transport::PublisherPtr requestPub;
      public: transport::PublisherPtr scenePub;

      /// \brief Publish user command messages for the server to place in the
      /// undo queue.
      public: transport::PublisherPtr userCmdPub;

      public: transport::SubscriberPtr responseSub;
      public: transport::SubscriberPtr guiSub;
      public: transport::SubscriberPtr newEntitySub, statsSub;
      public: transport::SubscriberPtr worldModSub;

      /// \brief Subscriber to the light modify topic.
      public: transport::SubscriberPtr lightModifySub;

      /// \brief Subscriber to the light factory topic.
      public: transport::SubscriberPtr lightFactorySub;

      public: QDockWidget *toolsDock;

      public: std::vector<event::ConnectionPtr> connections;

      // A map that associates physics_id's with entity names
      public: std::map<std::string, unsigned int> entities;

      /// \brief Message used to field requests.
      public: msgs::Request *requestMsg;

      /// \brief The left-hand tab widget
      public: QTabWidget *tabWidget;

      /// \brief Mainwindow's menubar
      public: QMenuBar *menuBar;

      /// \brief The Edit menu.
      public: QMenu *editMenu;

      /// \brief A layout for the menu bar.
      public: QHBoxLayout *menuLayout;

      /// \brief Used to control size of each pane.
      public: QStackedWidget *leftColumn;

      /// \brief Map of names to widgets in the leftColumn QStackedWidget
      public: std::map<std::string, int> leftColumnStack;

      /// \brief The filename set via "Save As". This filename is used by
      /// the "Save" feature.
      public: std::string saveFilename;

      /// \brief User specified step size for manually stepping the world
      public: int inputStepSize;

      /// \brief Map of all the editors to their names.
      public: std::map<std::string, Editor *> editors;

      /// \brief List of all the align action groups.
      public: std::vector<QActionGroup *> alignActionGroups;

      /// \brief Space navigator interface.
      public: SpaceNav *spacenav;

#ifdef HAVE_OCULUS
      public: gui::OculusWindow *oculusWindow;
#endif

      /// \brief Buffer of plugin messages to process.
      public: std::vector<boost::shared_ptr<msgs::Plugin const> > pluginMsgs;

      /// \brief Mutext to protect plugin loading.
      public: boost::mutex pluginLoadMutex;

      /// \brief Splitter for the main window.
      public: QSplitter *splitter;

      /// \brief Data logger dialog.
      public: gui::DataLogger *dataLogger;

      /// \brief Hotkey chart dialog.
      public: gui::HotkeyDialog *hotkeyDialog;

      /// \brief Tab to insert models.
      public: InsertModelWidget *insertModel;

      /// \brief Class which manages user commands and undoing / redoing them.
      public: UserCmdHistory *userCmdHistory;
    };
  }
}
#endif
