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

#ifdef _WIN32
  // Ensure that Winsock2.h is included before Windows.h, which can get
  // pulled in by anybody (e.g., Boost).
  #include <Winsock2.h>
#endif

#include <fstream>

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <sdf/sdf.hh>

#include "gazebo/common/SystemPaths.hh"
#include "gazebo/common/Console.hh"
#include "gazebo/common/ModelDatabase.hh"

#include "gazebo/rendering/RenderingIface.hh"
#include "gazebo/rendering/Scene.hh"
#include "gazebo/rendering/UserCamera.hh"
#include "gazebo/rendering/Visual.hh"
#include "gazebo/gui/GuiIface.hh"
#include "gazebo/gui/GuiEvents.hh"

#include "gazebo/transport/Node.hh"
#include "gazebo/transport/Publisher.hh"

#include "gazebo/gui/InsertModelWidgetPrivate.hh"
#include "gazebo/gui/InsertModelWidget.hh"

using namespace gazebo;
using namespace gui;

/////////////////////////////////////////////////
InsertModelWidget::InsertModelWidget(QWidget *_parent)
: QWidget(_parent), dataPtr(new InsertModelWidgetPrivate)
{
  this->setObjectName("insertModel");
  this->dataPtr->modelDatabaseItem = NULL;

  QVBoxLayout *mainLayout = new QVBoxLayout;
  this->dataPtr->fileTreeWidget = new QTreeWidget();
  this->dataPtr->fileTreeWidget->setColumnCount(1);
  this->dataPtr->fileTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  this->dataPtr->fileTreeWidget->header()->hide();
  connect(this->dataPtr->fileTreeWidget,
      SIGNAL(itemClicked(QTreeWidgetItem *, int)),
      this, SLOT(OnModelSelection(QTreeWidgetItem *, int)));

  QFrame *frame = new QFrame;
  QVBoxLayout *frameLayout = new QVBoxLayout;
  frameLayout->addWidget(this->dataPtr->fileTreeWidget, 0);
  frameLayout->setContentsMargins(0, 0, 0, 0);
  frame->setLayout(frameLayout);

  mainLayout->addWidget(frame);
  this->setLayout(mainLayout);
  this->layout()->setContentsMargins(0, 0, 0, 0);
  // Create a system path watcher
  this->dataPtr->watcher = new QFileSystemWatcher();

  // Update the list of models on the local system.
  this->UpdateAllLocalPaths();

  // Create a top-level tree item for the path
  this->dataPtr->modelDatabaseItem =
    new QTreeWidgetItem(static_cast<QTreeWidgetItem*>(0),
        QStringList(QString("Connecting to model database...")));
  this->dataPtr->fileTreeWidget->addTopLevelItem(
      this->dataPtr->modelDatabaseItem);

  // Also insert additional paths from gui.ini
  std::string additionalPaths =
      gui::getINIProperty<std::string>("model_paths.filenames", "");
  if (!additionalPaths.empty())
  {
    common::SystemPaths::Instance()->AddModelPaths(additionalPaths);

    // Get each path in the : separated list
    std::string delim(":");
    size_t pos1 = 0;
    size_t pos2 = additionalPaths.find(delim);
    while (pos2 != std::string::npos)
    {
      this->UpdateLocalPath(additionalPaths.substr(pos1, pos2-pos1));
      pos1 = pos2+1;
      pos2 = additionalPaths.find(delim, pos2+1);
    }
    this->UpdateLocalPath(additionalPaths.substr(pos1,
          additionalPaths.size()-pos1));
  }

  // Connect callbacks now that everything else is initialized

  // Connect a callback that is triggered whenever a directory is changed.
  connect(this->dataPtr->watcher, SIGNAL(directoryChanged(const QString &)),
          this, SLOT(OnDirectoryChanged(const QString &)));

  // Connect a callback to trigger when the model paths are updated.
  this->connections.push_back(
          common::SystemPaths::Instance()->updateModelRequest.Connect(
            boost::bind(&InsertModelWidget::OnModelUpdateRequest, this, _1)));

  /// Non-blocking call to get all the models in the database.
  this->dataPtr->getModelsConnection =
    common::ModelDatabase::Instance()->GetModels(
        boost::bind(&InsertModelWidget::OnModels, this, _1));

  // Start a timer to check for the results from the ModelDatabase. We need
  // to do this so that the QT elements get added in the main thread.
  QTimer::singleShot(1000, this, SLOT(Update()));
}

/////////////////////////////////////////////////
InsertModelWidget::~InsertModelWidget()
{
  delete this->dataPtr->watcher;
  delete this->dataPtr;
  this->dataPtr = NULL;
}

/////////////////////////////////////////////////
bool InsertModelWidget::LocalPathInFileWidget(const std::string &_path)
{
  return this->dataPtr->localFilenameCache.find(_path) !=
          this->dataPtr->localFilenameCache.end();
}

/////////////////////////////////////////////////
void InsertModelWidget::Update()
{
  boost::mutex::scoped_lock lock(this->dataPtr->mutex);

  // If the model database has call the OnModels callback function, then
  // add all the models from the database.
  if (!this->dataPtr->modelBuffer.empty())
  {
    std::string uri = common::ModelDatabase::Instance()->GetURI();
    this->dataPtr->modelDatabaseItem->setText(0,
        QString("%1").arg(QString::fromStdString(uri)));

    if (!this->dataPtr->modelBuffer.empty())
    {
      for (std::map<std::string, std::string>::const_iterator iter =
          this->dataPtr->modelBuffer.begin();
          iter != this->dataPtr->modelBuffer.end(); ++iter)
      {
        // Add a child item for the model
        QTreeWidgetItem *childItem = new QTreeWidgetItem(
            this->dataPtr->modelDatabaseItem,
            QStringList(QString("%1").arg(
                QString::fromStdString(iter->second))));
        childItem->setData(0, Qt::UserRole, QVariant(iter->first.c_str()));
        this->dataPtr->fileTreeWidget->addTopLevelItem(childItem);
      }
    }

    this->dataPtr->modelBuffer.clear();
    this->dataPtr->getModelsConnection.reset();
  }
  else
    QTimer::singleShot(1000, this, SLOT(Update()));
}



/////////////////////////////////////////////////
void InsertModelWidget::OnModels(
    const std::map<std::string, std::string> &_models)
{
  boost::mutex::scoped_lock lock(this->dataPtr->mutex);
  this->dataPtr->modelBuffer = _models;
}

/////////////////////////////////////////////////
void InsertModelWidget::OnModelSelection(QTreeWidgetItem *_item,
                                         int /*_column*/)
{
  boost::mutex::scoped_lock lock(this->dataPtr->mutex);
  if (_item)
  {
    std::string path, filename;

    if (_item->parent())
      path = _item->parent()->text(0).toStdString() + "/";

    path = _item->data(0, Qt::UserRole).toString().toStdString();

    if (!path.empty())
    {
      QApplication::setOverrideCursor(Qt::BusyCursor);
      filename = common::ModelDatabase::Instance()->GetModelFile(path);
      gui::Events::createEntity("model", filename);

      this->dataPtr->fileTreeWidget->clearSelection();
      QApplication::setOverrideCursor(Qt::ArrowCursor);
    }
  }
}

/////////////////////////////////////////////////
void InsertModelWidget::UpdateLocalPath(const std::string &_path)
{
  if (_path.empty())
    return;

  boost::filesystem::path dir(_path);
  bool pathExists = this->IsPathAccessible(dir);

  QString qpath = QString::fromStdString(_path);
  QTreeWidgetItem *topItem = NULL;

  QList<QTreeWidgetItem *> matchList =
    this->dataPtr->fileTreeWidget->findItems(qpath, Qt::MatchExactly);

  // Create a top-level tree item for the path
  if (matchList.empty())
  {
    topItem = new QTreeWidgetItem(
        static_cast<QTreeWidgetItem*>(0), QStringList(qpath));
    this->dataPtr->fileTreeWidget->addTopLevelItem(topItem);
    this->dataPtr->localFilenameCache.insert(_path);

    // Add the new path to the directory watcher
    if (pathExists)
    {
      this->dataPtr->watcher->addPath(qpath);
    }
  }
  else
    topItem = matchList.first();

  // Remove current items.
  topItem->takeChildren();

  if (pathExists && boost::filesystem::is_directory(dir))
  {
    std::vector<boost::filesystem::path> paths;

    // Get all the paths in alphabetical order
    try
    {
      std::copy(boost::filesystem::directory_iterator(dir),
          boost::filesystem::directory_iterator(),
          std::back_inserter(paths));
    }
    catch(boost::filesystem::filesystem_error & e)
    {
      gzerr << "Not loading models in: " << _path << " ("
            << e.what() << ")" << std::endl;
      return;
    }

    std::sort(paths.begin(), paths.end());

    // Iterate over all the models in the current gazebo path
    for (std::vector<boost::filesystem::path>::iterator dIter = paths.begin();
        dIter != paths.end(); ++dIter)
    {
      std::string modelName;
      boost::filesystem::path fullPath = _path / dIter->filename();
      boost::filesystem::path manifest = fullPath;

      if (!boost::filesystem::is_directory(fullPath))
      {
        if (dIter->filename() != "database.config")
        {
          gzlog << "Invalid filename or directory[" << fullPath
            << "] in GAZEBO_MODEL_PATH. It's not a good idea to put extra "
            << "files in a GAZEBO_MODEL_PATH because the file structure may"
            << " be modified by Gazebo.\n";
        }
        continue;
      }

      manifest /= GZ_MODEL_MANIFEST_FILENAME;

      // Check if the manifest does not exists
      if (!this->IsPathAccessible(manifest))
      {
        gzerr << "Missing " << GZ_MODEL_MANIFEST_FILENAME << " for model "
          << (*dIter) << "\n";

        manifest = manifest / "manifest.xml";
      }

       if (!this->IsPathAccessible(manifest) || manifest == fullPath)
      {
        gzlog << "model.config file is missing in directory["
              << fullPath << "]\n";
        continue;
      }

      TiXmlDocument xmlDoc;
      if (xmlDoc.LoadFile(manifest.string()))
      {
        TiXmlElement *modelXML = xmlDoc.FirstChildElement("model");
        if (!modelXML || !modelXML->FirstChildElement("name"))
          gzerr << "No model name in manifest[" << manifest << "]\n";
        else
          modelName = modelXML->FirstChildElement("name")->GetText();
        // Add a child item for the model
        QTreeWidgetItem *childItem = new QTreeWidgetItem(topItem,
            QStringList(QString::fromStdString(modelName)));

        childItem->setData(0, Qt::UserRole,
            QVariant((std::string("file://") + fullPath.string()).c_str()));

        this->dataPtr->fileTreeWidget->addTopLevelItem(childItem);
        this->dataPtr->localFilenameCache.insert(fullPath.string());
      }
    }
  }

  // Make all top-level items expanded. Trying to reduce mouse clicks.
  this->dataPtr->fileTreeWidget->expandItem(topItem);
}

/////////////////////////////////////////////////
void InsertModelWidget::UpdateAllLocalPaths()
{
  std::list<std::string> gazeboPaths =
    common::SystemPaths::Instance()->GetModelPaths();

  // Iterate over all the gazebo paths
  for (std::list<std::string>::iterator iter = gazeboPaths.begin();
      iter != gazeboPaths.end(); ++iter)
  {
    // This is the full model path
    this->UpdateLocalPath((*iter));
  }
}

/////////////////////////////////////////////////
void InsertModelWidget::OnDirectoryChanged(const QString &_path)
{
  boost::mutex::scoped_lock lock(this->dataPtr->mutex);
  this->UpdateLocalPath(_path.toStdString());
}

/////////////////////////////////////////////////
void InsertModelWidget::OnModelUpdateRequest(const std::string &_path)
{
  boost::mutex::scoped_lock lock(this->dataPtr->mutex);
  this->UpdateLocalPath(_path);
}

/////////////////////////////////////////////////
bool InsertModelWidget::IsPathAccessible(const boost::filesystem::path &_path)
{
  try
  {
    if (!boost::filesystem::exists(_path))
      return false;

    if (boost::filesystem::is_directory(_path))
    {
      // Try to retrieve a pointer to the first entry in this directory.
      // If permission denied to the directory, will throw filesystem_error
      boost::filesystem::directory_iterator iter(_path);
      return true;
    }
    else
    {
      std::ifstream ifs(_path.string().c_str(), std::ifstream::in);
      if (ifs.fail() || ifs.bad())
      {
        gzerr << "File unreadable: " << _path << std::endl;
        return false;
      }
      return true;
    }
  }
  catch(boost::filesystem::filesystem_error & e)
  {
    gzerr << "Permission denied for directory: " << _path << std::endl;
  }
  catch(std::exception & e)
  {
    gzerr << "Unexpected error while accessing to: " << _path << "."
          << "Error reported: " << e.what() << std::endl;
  }

  return false;
}
