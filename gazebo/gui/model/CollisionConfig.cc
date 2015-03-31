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

#include "gazebo/common/Console.hh"
#include "gazebo/gui/ConfigWidget.hh"
#include "gazebo/gui/model/CollisionConfig.hh"

using namespace gazebo;
using namespace gui;

/////////////////////////////////////////////////
CollisionConfig::CollisionConfig()
{
  this->setObjectName("CollisionConfig");
  QVBoxLayout *mainLayout = new QVBoxLayout;

  this->collisionsTreeWidget = new QTreeWidget();
  this->collisionsTreeWidget->setColumnCount(1);
  this->collisionsTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  this->collisionsTreeWidget->header()->hide();
  this->collisionsTreeWidget->setIndentation(4);

  this->collisionsTreeWidget->setSelectionMode(QAbstractItemView::NoSelection);
  connect(this->collisionsTreeWidget,
      SIGNAL(itemClicked(QTreeWidgetItem *, int)),
      this, SLOT(OnItemSelection(QTreeWidgetItem *, int)));

  QPushButton *addCollisionButton = new QPushButton(tr("+ &Another Collision"));
  connect(addCollisionButton, SIGNAL(clicked()), this, SLOT(OnAddCollision()));

  mainLayout->addWidget(this->collisionsTreeWidget);
  mainLayout->addWidget(addCollisionButton);
  this->setLayout(mainLayout);

  this->counter = 0;
  this->signalMapper = new QSignalMapper(this);

  connect(this->signalMapper, SIGNAL(mapped(int)),
     this, SLOT(OnRemoveCollision(int)));
}

/////////////////////////////////////////////////
CollisionConfig::~CollisionConfig()
{
  while (!this->configs.empty())
  {
    auto config = this->configs.begin();
    this->configs.erase(config);
  }
}

/////////////////////////////////////////////////
void CollisionConfig::OnAddCollision()
{
  std::stringstream collisionIndex;
  collisionIndex << "collision_" << this->counter;
  this->AddCollision(collisionIndex.str());
  emit CollisionAdded(collisionIndex.str());
}

/////////////////////////////////////////////////
unsigned int CollisionConfig::GetCollisionCount() const
{
  return this->configs.size();
}

/////////////////////////////////////////////////
void CollisionConfig::Reset()
{
  for (auto &it : this->configs)
    delete it.second;

  this->configs.clear();
  this->collisionsTreeWidget->clear();
}

/////////////////////////////////////////////////
void CollisionConfig::UpdateCollision(const std::string &_name,
    ConstCollisionPtr _collisionMsg)
{
  for (auto &it : this->configs)
  {
    if (it.second->name == _name)
    {
      CollisionConfigData *configData = it.second;
      configData->configWidget->UpdateFromMsg(_collisionMsg.get());
      break;
    }
  }
}

/////////////////////////////////////////////////
void CollisionConfig::AddCollision(const std::string &_name,
    const msgs::Collision *_collisionMsg)
{
  // Collision name label
  QLabel *collisionLabel = new QLabel(QString(_name.c_str()));

  // Remove button
  QToolButton *removeCollisionButton = new QToolButton(this);
  removeCollisionButton->setFixedSize(QSize(30, 30));
  removeCollisionButton->setToolTip("Remove " + QString(_name.c_str()));
  removeCollisionButton->setIcon(QPixmap(":/images/trashcan.png"));
  removeCollisionButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  removeCollisionButton->setIconSize(QSize(16, 16));
  removeCollisionButton->setCheckable(false);
  connect(removeCollisionButton, SIGNAL(clicked()), this->signalMapper,
      SLOT(map()));
  this->signalMapper->setMapping(removeCollisionButton, this->counter);

  // Item Layout
  QHBoxLayout *collisionItemLayout = new QHBoxLayout;
  collisionItemLayout->addWidget(collisionLabel);
  collisionItemLayout->addWidget(removeCollisionButton);
  collisionItemLayout->setContentsMargins(10, 0, 0, 0);

  // Item widget
  QWidget *collisionItemWidget = new QWidget;
  collisionItemWidget->setLayout(collisionItemLayout);

  // Top-level tree item
  QTreeWidgetItem *collisionItem =
      new QTreeWidgetItem(static_cast<QTreeWidgetItem*>(0));
  this->collisionsTreeWidget->addTopLevelItem(collisionItem);

  this->collisionsTreeWidget->setItemWidget(collisionItem, 0,
      collisionItemWidget);

  // ConfigWidget
  msgs::Collision msgToLoad;
  if (_collisionMsg)
    msgToLoad = *_collisionMsg;

  // set default values
  // TODO: auto-fill them with SDF defaults
  if (!msgToLoad.has_max_contacts())
    msgToLoad.set_max_contacts(10);
  msgs::Surface *surfaceMsg = msgToLoad.mutable_surface();
  if (!surfaceMsg->has_bounce_threshold())
    surfaceMsg->set_bounce_threshold(10e5);
  if (!surfaceMsg->has_soft_erp())
      surfaceMsg->set_soft_erp(0.2);
  if (!surfaceMsg->has_kp())
    surfaceMsg->set_kp(10e12);
  if (!surfaceMsg->has_kd())
    surfaceMsg->set_kd(1.0);
  if (!surfaceMsg->has_max_vel())
    surfaceMsg->set_max_vel(0.01);
  if (!surfaceMsg->has_collide_without_contact_bitmask())
    surfaceMsg->set_collide_without_contact_bitmask(1);
  if (!surfaceMsg->has_collide_bitmask())
    surfaceMsg->set_collide_bitmask(1);
  msgs::Friction *frictionMsg = surfaceMsg->mutable_friction();
  if (!frictionMsg->has_mu())
    frictionMsg->set_mu(1.0);
  if (!frictionMsg->has_mu2())
    frictionMsg->set_mu2(1.0);

  ConfigWidget *configWidget = new ConfigWidget;
  configWidget->Load(&msgToLoad);

  configWidget->SetWidgetVisible("id", false);
  configWidget->SetWidgetVisible("name", false);
  configWidget->SetWidgetReadOnly("id", true);
  configWidget->SetWidgetReadOnly("name", true);

  CollisionConfigData *configData = new CollisionConfigData;
  configData->configWidget = configWidget;
  configData->id =  this->counter;
  configData->treeItem = collisionItem;
  configData->name = _name;
  this->configs[this->counter] = configData;

  // Scroll area
  QScrollArea *scrollArea = new QScrollArea;
  scrollArea->setWidget(configWidget);
  scrollArea->setWidgetResizable(true);

  // Layout
  QVBoxLayout *collisionLayout = new QVBoxLayout;
  collisionLayout->setContentsMargins(0, 0, 0, 0);
  collisionLayout->addWidget(scrollArea);

  // Widget
  QWidget *collisionWidget = new QWidget;
  collisionWidget->setLayout(collisionLayout);
  collisionWidget->setMinimumHeight(800);

  // Child item
  QTreeWidgetItem *collisionChildItem =
      new QTreeWidgetItem(collisionItem);
  this->collisionsTreeWidget->setItemWidget(collisionChildItem, 0,
      collisionWidget);

  collisionItem->setExpanded(false);
  collisionChildItem->setExpanded(false);

  this->counter++;
}

/////////////////////////////////////////////////
void CollisionConfig::OnItemSelection(QTreeWidgetItem *_item,
                                         int /*_column*/)
{
  if (_item && _item->childCount() > 0)
    _item->setExpanded(!_item->isExpanded());
}


/////////////////////////////////////////////////
void CollisionConfig::OnRemoveCollision(int _id)
{
  auto it = this->configs.find(_id);
  if (it == this->configs.end())
  {
    gzerr << "Collision not found " << std::endl;
    return;
  }

  CollisionConfigData *configData = this->configs[_id];

  int index = this->collisionsTreeWidget->indexOfTopLevelItem(
      configData->treeItem);
  this->collisionsTreeWidget->takeTopLevelItem(index);

  emit CollisionRemoved(this->configs[_id]->name);
  this->configs.erase(it);
}

/////////////////////////////////////////////////
msgs::Collision *CollisionConfig::GetData(const std::string &_name) const
{
  for (auto const &it : this->configs)
  {
    std::string name = it.second->name;
    if (name == _name)
    {
      return dynamic_cast<msgs::Collision *>(it.second->configWidget->GetMsg());
    }
  }
  return NULL;
}

/////////////////////////////////////////////////
void CollisionConfig::SetGeometry(const std::string &_name,
    const math::Vector3 &_size, const std::string &_uri)
{
  for (auto &it : this->configs)
  {
    if (it.second->name == _name)
    {
      math::Vector3 dimensions;
      std::string uri;
      std::string type = it.second->configWidget->GetGeometryWidgetValue(
          "geometry", dimensions, uri);
      it.second->configWidget->SetGeometryWidgetValue("geometry", type,
          _size, _uri);
      break;
    }
  }
}
