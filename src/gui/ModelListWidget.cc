#include <QtGui>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "sdf/sdf.h"
#include "sdf/sdf_parser.h"
#include "common/SystemPaths.hh"
#include "common/Console.hh"
#include "common/Events.hh"

#include "rendering/Rendering.hh"
#include "rendering/Scene.hh"
#include "rendering/UserCamera.hh"
#include "rendering/Visual.hh"
#include "gui/Gui.hh"

#include "transport/Node.hh"
#include "transport/Publisher.hh"

#include "math/Angle.hh"
#include "math/Helpers.hh"

#include "gui/GuiEvents.hh"
#include "gui/qtpropertybrowser/qttreepropertybrowser.h"
#include "gui/qtpropertybrowser/qtvariantproperty.h"
#include "gui/ModelListWidget.hh"

using namespace gazebo;
using namespace gui;

const unsigned short dgrsUnicode = 0x00b0;
const std::string dgrsStdStr = QString::fromUtf16(&dgrsUnicode, 1).toStdString();
const std::string rollLbl = std::string("Roll") + dgrsStdStr;
const std::string pitchLbl = std::string("Pitch") + dgrsStdStr;
const std::string yawLbl = std::string("Yaw") + dgrsStdStr;
const std::string xLbl = std::string("X");
const std::string yLbl = std::string("Y");
const std::string zLbl = std::string("Z");


ModelListWidget::ModelListWidget(QWidget *parent)
  : QWidget(parent)
{
  this->requestMsg = NULL;
  this->propMutex = new boost::mutex();
  this->receiveMutex = new boost::mutex();
  this->fillPropertyTree = false;

  setMinimumWidth(280);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  this->modelTreeWidget = new QTreeWidget();
  this->modelTreeWidget->setColumnCount(1);
  this->modelTreeWidget->setContextMenuPolicy( Qt::CustomContextMenu );
  this->modelTreeWidget->header()->hide();
  connect(this->modelTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
          this, SLOT(OnModelSelection(QTreeWidgetItem *, int)) );
  connect(this->modelTreeWidget, 
      SIGNAL( customContextMenuRequested(const QPoint &)),
      this, SLOT(OnCustomContextMenu(const QPoint &)));

  this->variantManager = new QtVariantPropertyManager();
  this->propTreeBrowser = new QtTreePropertyBrowser();
  this->variantFactory = new QtVariantEditorFactory();
  this->propTreeBrowser->setFactoryForManager(this->variantManager, 
                                              this->variantFactory);
  connect(this->variantManager, 
          SIGNAL(propertyChanged(QtProperty*)), 
          this, SLOT(OnPropertyChanged(QtProperty *)));
  connect(this->propTreeBrowser, 
          SIGNAL(currentItemChanged(QtBrowserItem*)), 
          this, SLOT(OnCurrentPropertyChanged(QtBrowserItem *)));


  mainLayout->addWidget(this->modelTreeWidget,0);
  mainLayout->addWidget(this->propTreeBrowser,1);
  this->setLayout(mainLayout);
  this->layout()->setContentsMargins(2,2,2,2);

  this->node = transport::NodePtr(new transport::Node());
  this->node->Init();

  this->modelPub = this->node->Advertise<msgs::Model>("~/model/modify");
  this->requestPub = this->node->Advertise<msgs::Request>("~/request");
  this->responseSub = this->node->Subscribe("~/response", 
      &ModelListWidget::OnResponse, this, false);

  this->poseSub = this->node->Subscribe("~/pose/info",
      &ModelListWidget::OnPose, this);

  this->requestSub = this->node->Subscribe("~/request",
      &ModelListWidget::OnRequest, this, false);

  this->factoryPub = this->node->Advertise<msgs::Factory>("~/factory");

  this->followAction = new QAction(tr("Follow"), this);
  this->followAction->setStatusTip(tr("Follow the selection"));
  connect(this->followAction, SIGNAL(triggered()), this, SLOT(OnFollow()));

  this->moveToAction = new QAction(tr("Move To"), this);
  this->moveToAction->setStatusTip(tr("Move camera to the selection"));
  connect(this->moveToAction, SIGNAL(triggered()), this, SLOT(OnMoveTo()));

  this->deleteAction = new QAction(tr("Delete"), this);
  this->deleteAction->setStatusTip(tr("Delete the selection"));
  connect(this->deleteAction, SIGNAL(triggered()), this, SLOT(OnDelete()));

  this->showCollisionAction = new QAction(tr("Show Collision"), this);
  this->showCollisionAction->setStatusTip(tr("Show Collision Entity"));
  this->showCollisionAction->setCheckable(true);
  connect(this->showCollisionAction, SIGNAL(triggered()), this, 
          SLOT(OnShowCollision()));

  this->fillingPropertyTree = false;
  this->selectedProperty = NULL;

  this->connections.push_back( 
      gui::Events::ConnectModelUpdate( 
        boost::bind(&ModelListWidget::OnModelUpdate, this, _1) ) );


  QTimer::singleShot( 500, this, SLOT(Update()) );
}

ModelListWidget::~ModelListWidget()
{
  delete this->propMutex;
  delete this->receiveMutex;
}


void ModelListWidget::OnModelSelection(QTreeWidgetItem *_item, int /*_column*/)
{
  if (_item)
  {
    msgs::Selection msg;

    if (!this->selectedModelName.empty())
      event::Events::setSelectedEntity("");

    this->propTreeBrowser->clear();
    this->selectedModelName = 
      _item->data(1, Qt::UserRole).toString().toStdString();

    event::Events::setSelectedEntity(this->selectedModelName);

    this->requestMsg = msgs::CreateRequest("entity_info",
        this->selectedModelName);
    this->requestPub->Publish(*this->requestMsg);
  }
  else
    this->selectedModelName.clear();
}

void ModelListWidget::Update()
{
  if (this->fillPropertyTree)
  {
    this->fillingPropertyTree = true;
    this->FillPropertyTree(this->modelMsg, NULL);
    this->fillingPropertyTree = false;
    this->fillPropertyTree = false;
  }

  this->ProcessPoseMsgs();
  QTimer::singleShot(500, this, SLOT(Update()));
}

void ModelListWidget::OnModelUpdate(const msgs::Model &_msg)
{
  std::string name = _msg.name();

  QTreeWidgetItem *listItem = this->GetModelListItem(_msg.id());

  if (!listItem)
  {
    if (!_msg.has_deleted() || !_msg.deleted())
    {
      // Create a top-level tree item for the path
      QTreeWidgetItem *topItem = new QTreeWidgetItem((QTreeWidgetItem*)0, 
          QStringList(QString("%1").arg(QString::fromStdString(name))));

      topItem->setData(0, Qt::UserRole, QVariant(_msg.id()));
      topItem->setData(1, Qt::UserRole, QVariant(_msg.name().c_str()));
      this->modelTreeWidget->addTopLevelItem(topItem);

      for (int i=0; i < _msg.link_size(); i++)
      {
        std::string linkName = _msg.link(i).name();
        int index = linkName.rfind("::") + 2;
        std::string linkNameShort = linkName.substr(
            index, linkName.size() - index);

        QTreeWidgetItem *linkItem = new QTreeWidgetItem( topItem, 
          QStringList(QString("%1").arg( 
              QString::fromStdString(linkNameShort)) ));

        linkItem->setData(0, Qt::UserRole, QVariant(_msg.link(i).id()));
        linkItem->setData(1, Qt::UserRole, QVariant(_msg.name().c_str()));
        this->modelTreeWidget->addTopLevelItem(linkItem);
      }
    }
  }
  else
  {
    if (_msg.has_deleted() && _msg.deleted())
    {
      int i = this->modelTreeWidget->indexOfTopLevelItem(listItem);
      this->modelTreeWidget->takeTopLevelItem(i);
    }
    else
    {
      listItem->setText(0, _msg.name().c_str());
      listItem->setData(1, Qt::UserRole, QVariant(_msg.name().c_str()));
    }
  }
}

void ModelListWidget::OnResponse(
    const boost::shared_ptr<msgs::Response const> &_msg)
{
  if (!this->requestMsg || _msg->id() != this->requestMsg->id())
    return;

  msgs::Model modelMsg;

  if (_msg->has_type() && _msg->type() == this->modelMsg.GetTypeName())
  {
    this->propMutex->lock();
    this->modelMsg.ParseFromString(_msg->serialized_data());
    this->propTreeBrowser->clear();
    this->fillPropertyTree = true;
    this->propMutex->unlock();
  }
  else if (_msg->has_type() && _msg->type() == "error")
  {
    if (_msg->response() == "nonexistant")
    {
      this->RemoveEntity(this->selectedModelName);
    }
  }

  delete this->requestMsg;
  this->requestMsg = NULL;
}

void ModelListWidget::RemoveEntity(const std::string &_name)
{
  QTreeWidgetItem *listItem = this->GetModelListItem(gui::get_entity_id(_name));
  if (listItem)
  {
    int i = this->modelTreeWidget->indexOfTopLevelItem(listItem);
    this->modelTreeWidget->takeTopLevelItem(i);

    this->propTreeBrowser->clear();
    this->selectedModelName.clear();
    this->sdfElement.reset();
  }
}

QTreeWidgetItem *ModelListWidget::GetModelListItem(unsigned int _id)
{
  QTreeWidgetItem *listItem = NULL;

  // Find an existing element with the name from the message
  for (int i=0; i < this->modelTreeWidget->topLevelItemCount() && !listItem;i++)
  {
    QTreeWidgetItem *item = this->modelTreeWidget->topLevelItem(i);
    unsigned int data = item->data(0, Qt::UserRole).toUInt();
    if (data == _id)
    {
      listItem = item;
      break;
    }

    for (int j=0; j < item->childCount(); j++)
    {
      QTreeWidgetItem *childItem = item->child(j);
      data = childItem->data(0, Qt::UserRole).toUInt();
      if (data == _id)
      {
        listItem = childItem;
        break;
      }
    }
  }

  return listItem;
}

void ModelListWidget::OnShowCollision()
{
  QTreeWidgetItem *item = this->modelTreeWidget->currentItem();
  std::string modelName = item->text(0).toStdString();

  if (this->showCollisionAction->isChecked())
    this->requestMsg = msgs::CreateRequest("show_collision", modelName);
  else
    this->requestMsg = msgs::CreateRequest("hide_collision", modelName);

  this->requestPub->Publish(*this->requestMsg);
}

void ModelListWidget::OnDelete()
{
  QTreeWidgetItem *item = this->modelTreeWidget->currentItem();
  std::string modelName = item->text(0).toStdString();

  this->requestMsg = msgs::CreateRequest( "entity_delete", modelName );
  this->requestPub->Publish(*this->requestMsg);
}

void ModelListWidget::OnFollow()
{
  QTreeWidgetItem *item = this->modelTreeWidget->currentItem();
  std::string modelName = item->text(0).toStdString();

  rendering::UserCameraPtr cam = gui::get_active_camera();
  cam->TrackVisual(modelName);
}

void ModelListWidget::OnMoveTo()
{
  QTreeWidgetItem *item = this->modelTreeWidget->currentItem();
  std::string modelName = item->text(0).toStdString();

  rendering::UserCameraPtr cam = gui::get_active_camera();
  cam->MoveToVisual(modelName);
}

void ModelListWidget::OnCustomContextMenu(const QPoint &_pt)
{
  QTreeWidgetItem *item = this->modelTreeWidget->itemAt(_pt);

  if (item)
  {
    QMenu menu(this->modelTreeWidget);
    menu.addAction(this->moveToAction);
    menu.addAction(this->followAction);
    menu.addAction(this->deleteAction);
    menu.addAction(this->showCollisionAction);
    menu.exec(this->modelTreeWidget->mapToGlobal(_pt));
  }
}

void ModelListWidget::OnCurrentPropertyChanged(QtBrowserItem *_item)
{
  if (_item)
    this->selectedProperty = _item->property();
  else
    this->selectedProperty = NULL;
}

void ModelListWidget::OnPropertyChanged(QtProperty *_item)
{
  if (!this->propMutex->try_lock())
    return;

  if (this->selectedProperty != _item || this->fillingPropertyTree)
  {
    this->propMutex->unlock();
    return;
  }

  msgs::Model msg;

  msg.set_id(this->modelMsg.id());
  msg.set_name(this->modelMsg.name());

  const google::protobuf::Descriptor *descriptor = msg.GetDescriptor();
  const google::protobuf::Reflection *reflection = msg.GetReflection();

  QList<QtProperty*> properties = this->propTreeBrowser->properties();
  for (QList<QtProperty*>::iterator iter = properties.begin(); 
       iter != properties.end(); iter++)
  {
    if (!this->HasChildItem(*iter, _item))
      continue;
    std::cout << "Has Item[" << (*iter)->propertyName().toStdString() << "]\n";

    const google::protobuf::FieldDescriptor *field =
      descriptor->FindFieldByName((*iter)->propertyName().toStdString());

    // If the message has the field, and it's another message type, then
    // recursively call FillMsg
    if (field &&
        field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
    {
      if (field->is_repeated())
      {
        this->FillMsg((*iter), reflection->AddMessage(&msg, field),
            field->message_type(), _item);
      }
      else
      {
        this->FillMsg((*iter),
            reflection->MutableMessage(&msg, field),
            field->message_type(), _item);
      }
    }
    else if (field)
    {
      this->FillMsgField((*iter), &msg, reflection, field);
    }
    else
    {
      gzerr << "Unable to process[" 
            << (*iter)->propertyName().toStdString() << "]\n";
    }
  }

  std::cout << "\n\n" << msg.DebugString() << "\n\n";
  this->modelPub->Publish(msg);

  this->propMutex->unlock();
}

void ModelListWidget::FillMsgField(QtProperty *_item, 
    google::protobuf::Message *_message,
    const google::protobuf::Reflection *_reflection,
    const google::protobuf::FieldDescriptor *_field)
{
  if (_field->type() == google::protobuf::FieldDescriptor::TYPE_INT32)
    _reflection->SetInt32(_message, _field, 
        this->variantManager->value(_item).toInt());
  else if (_field->type() == google::protobuf::FieldDescriptor::TYPE_DOUBLE)
    _reflection->SetDouble(_message, _field, 
        this->variantManager->value(_item).toDouble());
  else if (_field->type() == google::protobuf::FieldDescriptor::TYPE_FLOAT)
    _reflection->SetFloat(_message, _field, 
        this->variantManager->value(_item).toDouble());
  else if (_field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL)
    _reflection->SetBool(_message, _field, 
        this->variantManager->value(_item).toBool());
  else if (_field->type() == google::protobuf::FieldDescriptor::TYPE_STRING)
    _reflection->SetString(_message, _field, 
        this->variantManager->value(_item).toString().toStdString());
  else if (_field->type() == google::protobuf::FieldDescriptor::TYPE_UINT32)
    _reflection->SetUInt32(_message, _field, 
        this->variantManager->value(_item).toUInt());
  else
    gzerr << "Unable to fill message field[" << _field->type() << "]\n";
}

void ModelListWidget::FillGeometryMsg(QtProperty *_item, 
    google::protobuf::Message *_message,
    const google::protobuf::Descriptor *_descriptor,
    QtProperty *_changedItem)
{
  QtProperty *typeProperty = this->GetChildItem(_item,"type");

  std::string type = typeProperty->valueText().toStdString();
  const google::protobuf::Reflection *reflection = _message->GetReflection();
  reflection->SetEnum(_message, _descriptor->FindFieldByName("type"),
      _descriptor->FindEnumValueByName(type));

  boost::to_lower(type);
  const google::protobuf::FieldDescriptor *field =
    _descriptor->FindFieldByName(type);
  google::protobuf::Message *message =
    _message->GetReflection()->MutableMessage(_message, field);

  if (type == "box")
  {
    QtProperty *sizeProperty = this->GetChildItem(_item,"size");
    msgs::BoxGeom *boxMsg = (msgs::BoxGeom*)(message);
    double xValue = this->variantManager->value(
        this->GetChildItem(sizeProperty,"x")).toDouble();
    double yValue = this->variantManager->value(
        this->GetChildItem(sizeProperty,"y")).toDouble();
    double zValue = this->variantManager->value(
        this->GetChildItem(sizeProperty,"z")).toDouble();

    boxMsg->mutable_size()->set_x(xValue);
    boxMsg->mutable_size()->set_y(yValue);
    boxMsg->mutable_size()->set_z(zValue);
  }
  else if (type == "sphere")
  {
    QtProperty *radiusProperty = this->GetChildItem(_item,"radius");
    msgs::SphereGeom *sphereMsg = (msgs::SphereGeom*)(message);

    sphereMsg->set_radius(
        this->variantManager->value(radiusProperty).toDouble());
  }
  else if (type == "cylinder")
  {
    QtProperty *radiusProperty = this->GetChildItem(_item,"radius");
    QtProperty *lengthProperty = this->GetChildItem(_item,"length");

    msgs::CylinderGeom *cylinderMsg = (msgs::CylinderGeom*)(message);
    cylinderMsg->set_radius(
        this->variantManager->value(radiusProperty).toDouble());
    cylinderMsg->set_length(
        this->variantManager->value(lengthProperty).toDouble());
  }
  else if (type == "plane")
  {
    QtProperty *normalProperty = this->GetChildItem(_item,"normal");
    msgs::PlaneGeom *planeMessage = (msgs::PlaneGeom*)(message);

    double xValue = this->variantManager->value(
        this->GetChildItem(normalProperty,"x")).toDouble();
    double yValue = this->variantManager->value(
        this->GetChildItem(normalProperty,"y")).toDouble();
    double zValue = this->variantManager->value(
        this->GetChildItem(normalProperty,"z")).toDouble();

    planeMessage->mutable_normal()->set_x(xValue);
    planeMessage->mutable_normal()->set_y(yValue);
    planeMessage->mutable_normal()->set_z(zValue);
  }
  else if (type == "image")
  {
    QtProperty *fileProp = this->GetChildItem(_item,"filename");
    QtProperty *scaleProp = this->GetChildItem(_item,"scale");
    QtProperty *heightProp = this->GetChildItem(_item,"height");
    QtProperty *thresholdProp = this->GetChildItem(_item,"threshold");
    QtProperty *granularityProp = this->GetChildItem(_item,"granularity");

    msgs::ImageGeom *imageMessage = (msgs::ImageGeom*)(message);
    imageMessage->set_filename(
        this->variantManager->value(fileProp).toString().toStdString());
    imageMessage->set_scale(
        this->variantManager->value(scaleProp).toDouble());
    imageMessage->set_height(
        this->variantManager->value(heightProp).toDouble());
    imageMessage->set_threshold(
        this->variantManager->value(thresholdProp).toInt());
    imageMessage->set_granularity(
        this->variantManager->value(granularityProp).toInt());
  }
  else if (type == "heightmap")
  {
    QtProperty *sizeProp = this->GetChildItem(_item, "size");
    QtProperty *offsetProp = this->GetChildItem(_item, "offset");
    QtProperty *fileProp = this->GetChildItem(_item, "filename");

    double x,y,z;
    msgs::HeightmapGeom *heightmapMessage = (msgs::HeightmapGeom*)(message);

    heightmapMessage->set_filename(this->variantManager->value(
          fileProp).toString().toStdString());

    x = this->variantManager->value(
        this->GetChildItem(sizeProp,"x")).toDouble();
    y = this->variantManager->value(
        this->GetChildItem(sizeProp,"y")).toDouble();
    z = this->variantManager->value(
        this->GetChildItem(sizeProp,"z")).toDouble();

    heightmapMessage->mutable_size()->set_x(x);
    heightmapMessage->mutable_size()->set_y(y);
    heightmapMessage->mutable_size()->set_z(z);

    x = this->variantManager->value(
        this->GetChildItem(offsetProp,"x")).toDouble();
    y = this->variantManager->value(
        this->GetChildItem(offsetProp,"y")).toDouble();
    z = this->variantManager->value(
        this->GetChildItem(offsetProp,"z")).toDouble();

    heightmapMessage->mutable_offset()->set_x(x);
    heightmapMessage->mutable_offset()->set_y(y);
    heightmapMessage->mutable_offset()->set_z(z);
  }
  else if (type == "mesh")
  {
    QtProperty *sizeProp = this->GetChildItem(_item, "scale");
    QtProperty *fileProp = this->GetChildItem(_item, "filename");

    double x,y,z;
    msgs::MeshGeom *meshMessage = (msgs::MeshGeom*)(message);
    meshMessage->set_filename(this->variantManager->value(
          fileProp).toString().toStdString());

    x = this->variantManager->value(
        this->GetChildItem(sizeProp,"x")).toDouble();
    y = this->variantManager->value(
        this->GetChildItem(sizeProp,"y")).toDouble();
    z = this->variantManager->value(
        this->GetChildItem(sizeProp,"z")).toDouble();

    meshMessage->mutable_scale()->set_x(x);
    meshMessage->mutable_scale()->set_y(y);
    meshMessage->mutable_scale()->set_z(z);
  }
  else
    std::cout << "Unknown geom type[" << type << "]\n";
}

void ModelListWidget::FillPoseMsg(QtProperty *_item, 
    google::protobuf::Message *_message,
    const google::protobuf::Descriptor *_descriptor)
{
  const google::protobuf::Descriptor *posDescriptor; 
  const google::protobuf::FieldDescriptor *posField; 
  google::protobuf::Message *posMessage; 
  const google::protobuf::Reflection *posReflection; 

  const google::protobuf::Descriptor *orientDescriptor; 
  const google::protobuf::FieldDescriptor *orientField; 
  google::protobuf::Message *orientMessage; 
  const google::protobuf::Reflection *orientReflection; 

  posField = _descriptor->FindFieldByName("position");
  posDescriptor = posField->message_type();
  posMessage = _message->GetReflection()->MutableMessage(_message, posField);
  posReflection = posMessage->GetReflection();

  orientField = _descriptor->FindFieldByName("orientation");
  orientDescriptor = orientField->message_type();
  orientMessage = _message->GetReflection()->MutableMessage(
      _message, orientField);
  orientReflection = orientMessage->GetReflection();

  this->FillMsgField(this->GetChildItem(_item,"x"), posMessage, posReflection,
      posDescriptor->FindFieldByName("x"));
  this->FillMsgField(this->GetChildItem(_item,"y"), posMessage, posReflection,
      posDescriptor->FindFieldByName("y"));
  this->FillMsgField(this->GetChildItem(_item,"z"), posMessage, posReflection,
      posDescriptor->FindFieldByName("z"));

  double roll, pitch, yaw; 
  roll = this->variantManager->value(
      this->GetChildItem(_item,"roll")).toDouble();
  pitch = this->variantManager->value(
      this->GetChildItem(_item,"pitch")).toDouble();
  yaw = this->variantManager->value(
      this->GetChildItem(_item,"yaw")).toDouble();
  math::Quaternion q(roll, pitch, yaw);

  orientReflection->SetDouble(
      orientMessage,
      orientDescriptor->FindFieldByName("x"),
      q.x);
  orientReflection->SetDouble(
      orientMessage,
      orientDescriptor->FindFieldByName("y"),
      q.y);
  orientReflection->SetDouble(
      orientMessage,
      orientDescriptor->FindFieldByName("z"),
      q.z);
  orientReflection->SetDouble(
      orientMessage,
      orientDescriptor->FindFieldByName("w"),
      q.w);

}

void ModelListWidget::FillMsg(QtProperty *_item, 
    google::protobuf::Message *_message,
    const google::protobuf::Descriptor *_descriptor,
    QtProperty *_changedItem)
{
  if (!_item)
    return;

  std::cout << "::FillMsg[" << _item->propertyName().toStdString() << "]\n";

  if (_item->propertyName().toStdString() == "link")
  {
    QtProperty *nameItem = this->GetChildItem(_item,"name");
    ((msgs::Link*)(_message))->set_name(nameItem->valueText().toStdString());
    ((msgs::Link*)(_message))->set_id(
      gui::get_entity_id(nameItem->valueText().toStdString()));
  }
  else if (_item->propertyName().toStdString() == "collision")
  {
    QtProperty *nameItem = this->GetChildItem(_item,"name");
    ((msgs::Collision*)_message)->set_name(nameItem->valueText().toStdString());
    ((msgs::Collision*)_message)->set_id(
      gui::get_entity_id(nameItem->valueText().toStdString()));
  }

  if (_item->propertyName().toStdString() == "geometry" &&
      this->HasChildItem(_item, _changedItem))
  {
    this->FillGeometryMsg(_item, _message, _descriptor, _changedItem);
  }
  else if (_item->propertyName().toStdString() == "pose")
  {
    std::cout << "Item is a pose\n";
    if (this->HasChildItem(_item, _changedItem))
    {
      std::cout << "Filling pose message\n";
      this->FillPoseMsg(_item, _message, _descriptor);
    }
  }
  else
  {
    const google::protobuf::Reflection *reflection = _message->GetReflection();

    QList<QtProperty*> properties = _item->subProperties();
    for (QList<QtProperty*>::iterator iter = properties.begin(); 
        iter != properties.end(); iter++)
    {
      if (!this->HasChildItem(*iter, _changedItem))
        continue;
      std::cout << "    Has Item[" << (*iter)->propertyName().toStdString() << "]\n";
      const google::protobuf::FieldDescriptor *field =
        _descriptor->FindFieldByName((*iter)->propertyName().toStdString());

      // If the message has the field, and it's another message type, then
      // recursively call FillMsg
      if (field &&
          field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
      {
        if (field->is_repeated())
        {
          this->FillMsg((*iter), reflection->AddMessage(_message, field),
              field->message_type(), _changedItem);
        }
        else
        {
          this->FillMsg((*iter),
              reflection->MutableMessage(_message, field),
              field->message_type(), _changedItem);
        }
      }
      else if (field)
      {
        this->FillMsgField((*iter), _message, reflection, field);
      }
      else
      {
        gzerr << "Unable to process[" 
          << (*iter)->propertyName().toStdString() << "]\n";
      }
    }
  }
}

QtProperty *ModelListWidget::PopChildItem(QList<QtProperty*> &_list,
    const std::string &_name)
{
  for (QList<QtProperty*>::iterator iter = _list.begin(); 
      iter != _list.end(); iter++)
  {
    if ( (*iter)->propertyName().toStdString() == _name )
    {
      _list.erase(iter);
      return (*iter);
    }
  }

  return NULL;
}

QtProperty *ModelListWidget::GetParentItemValue(const std::string &_name)
{
  QtProperty *result = NULL;

  QList<QtProperty*> properties = this->propTreeBrowser->properties();
  for (QList<QtProperty*>::iterator iter = properties.begin(); 
      iter != properties.end(); iter++)
  {
    if ((*iter)->valueText().toStdString() == _name)
      return NULL;
    else if ((result = this->GetParentItemValue(*iter, _name)) != NULL)
      break;
  }

  return result;
}

QtProperty *ModelListWidget::GetParentItemValue(QtProperty *_item,
                                           const std::string &_name)
{
  if (!_item)
    return NULL;

  QtProperty *result = NULL;

  QList<QtProperty*> subProperties = _item->subProperties();
  for (QList<QtProperty*>::iterator iter = subProperties.begin(); 
      iter != subProperties.end(); iter++)
  {
    if ((*iter)->valueText().toStdString() == _name)
    {
      result = _item;
      break;
    }
    else if ((result = this->GetParentItemValue(*iter, _name)) != NULL)
      break;
  }

  return result;
}

QtProperty *ModelListWidget::GetParentItem(const std::string &_name)
{
  QtProperty *result = NULL;

  QList<QtProperty*> properties = this->propTreeBrowser->properties();
  for (QList<QtProperty*>::iterator iter = properties.begin(); 
      iter != properties.end(); iter++)
  {
    if ((*iter)->propertyName().toStdString() == _name)
      return NULL;
    else if ((result = this->GetParentItem(*iter, _name)) != NULL)
      break;
  }

  return result;
}

QtProperty *ModelListWidget::GetParentItem(QtProperty *_item,
                                           const std::string &_name)
{
  if (!_item)
    return NULL;

  QtProperty *result = NULL;

  QList<QtProperty*> subProperties = _item->subProperties();
  for (QList<QtProperty*>::iterator iter = subProperties.begin(); 
      iter != subProperties.end(); iter++)
  {
    if ((*iter)->propertyName().toStdString() == _name)
    {
      result = _item;
      break;
    }
    else if ( (result = this->GetParentItem(*iter, _name)) != NULL)
      break;
  }

  return result;
}

bool ModelListWidget::HasChildItem(QtProperty *_parent, QtProperty *_child)
{
  if (!_parent)
    return false;
  if (_parent == _child)
    return true;

  bool result = false;
  QList<QtProperty*> subProperties = _parent->subProperties();
  for (QList<QtProperty*>::iterator iter = subProperties.begin(); 
      iter != subProperties.end(); iter++)
  {
    if ((result = this->HasChildItem(*iter, _child)))
      break;
  }

  return result;
}

QtProperty *ModelListWidget::GetChildItemValue(const std::string &_name)
{
  QtProperty *result = NULL;

  QList<QtProperty*> properties = this->propTreeBrowser->properties();
  for (QList<QtProperty*>::iterator iter = properties.begin(); 
      iter != properties.end(); iter++)
  {
    if ((result = this->GetChildItemValue(*iter, _name)) != NULL)
      break;
  }

  return result;
}

QtProperty *ModelListWidget::GetChildItemValue(QtProperty *_item,
                                          const std::string &_name)
{
  if (!_item)
    return NULL;
  if (_item->valueText().toStdString() == _name)
    return _item;

  QtProperty *result = NULL;
  QList<QtProperty*> subProperties = _item->subProperties();
  for (QList<QtProperty*>::iterator iter = subProperties.begin(); 
      iter != subProperties.end(); iter++)
  {
    if ((result = this->GetChildItem(*iter, _name)) != NULL)
      break;
  }

  return result;
}

QtProperty *ModelListWidget::GetChildItem(const std::string &_name)
{
  QtProperty *result = NULL;

  QList<QtProperty*> properties = this->propTreeBrowser->properties();
  for (QList<QtProperty*>::iterator iter = properties.begin(); 
      iter != properties.end(); iter++)
  {
    if ((result = this->GetChildItem(*iter, _name)) != NULL)
      break;
  }

  return result;
}

QtProperty *ModelListWidget::GetChildItem(QtProperty *_item,
                                          const std::string &_name)
{
  if (!_item)
    return NULL;
  if (_item->propertyName().toStdString() == _name)
    return _item;

  QtProperty *result = NULL;
  QList<QtProperty*> subProperties = _item->subProperties();
  for (QList<QtProperty*>::iterator iter = subProperties.begin(); 
      iter != subProperties.end(); iter++)
  {
    if ((result = this->GetChildItem(*iter, _name)) != NULL)
      break;
  }

  return result;
}


/*void ModelListWidget::FillSDF( QtProperty *_item, QtProperty *_parent, 
                               std::string &_sdf )
{
  std::string name = _item->propertyName().toStdString();
  std::string value = _item->valueText().toStdString();

  if (_item->hasValue())
  {
    std::cout << " " << name << "='" << value << "'";
  }
  else
  {
    std::cout << "<" << name;

    bool closed = false;
    QList<QtProperty*> subProperties = _item->subProperties();
    for (QList<QtProperty*>::iterator iter = subProperties.begin(); 
        iter != subProperties.end(); iter++)
    {
      if ( !(*iter)->hasValue() && !closed )
      {
        closed = true;
        std::cout << ">\n";
      }
      this->FillSDF(*iter, _item, _sdf);
    }

    std::cout << "</" << name << ">\n";
  }
}*/



void ModelListWidget::FillPropertyTree(const msgs::Link &_msg,
                                       QtProperty *_parent)
{
  QtProperty *topItem = NULL;
  QtProperty *inertialItem = NULL;
  QtVariantProperty *item = NULL;

  item = this->variantManager->addProperty(QVariant::String,
                                           QLatin1String("name"));
  item->setValue(_msg.name().c_str());
  _parent->addSubProperty(item);


  // Self-collide
  item = this->variantManager->addProperty(QVariant::Bool,
                                           QLatin1String("self_collide"));
  if (_msg.has_self_collide())
    item->setValue(_msg.self_collide());
  else
    item->setValue(true);
  _parent->addSubProperty(item);

  // gravity
  item = this->variantManager->addProperty(QVariant::Bool,
                                           QLatin1String("gravity"));
  if (_msg.has_gravity())
    item->setValue(_msg.gravity());
  else
    item->setValue(true);
  _parent->addSubProperty(item);

  // kinematic
  item = this->variantManager->addProperty(QVariant::Bool,
                                           QLatin1String("kinematic"));
  if (_msg.has_kinematic())
    item->setValue(_msg.kinematic());
  else
    item->setValue(false);
  _parent->addSubProperty(item);

  topItem = this->variantManager->addProperty(
      QtVariantPropertyManager::groupTypeId(),
      QLatin1String("pose"));
  _parent->addSubProperty(topItem);
  this->FillPoseProperty(_msg.pose(), topItem);

  // Inertial
  inertialItem = this->variantManager->addProperty(
      QtVariantPropertyManager::groupTypeId(),
      QLatin1String("inertial"));
  _parent->addSubProperty(inertialItem);

  // Inertial::Mass
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("mass"));
  if (_msg.inertial().has_mass())
    item->setValue(_msg.inertial().mass());
  else
    item->setValue(0.0);
  inertialItem->addSubProperty(item);

  // Inertial::LinearDamping
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("linear_damping"));
  if (_msg.inertial().has_linear_damping())
    item->setValue(_msg.inertial().linear_damping());
  else
    item->setValue(0.0);
  inertialItem->addSubProperty(item);

  // Inertial::AngularDamping
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("angular_damping"));
  if (_msg.inertial().has_angular_damping())
    item->setValue(_msg.inertial().angular_damping());
  else
    item->setValue(0.0);
  inertialItem->addSubProperty(item);

  // Inertial::ixx
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("ixx"));
  if (_msg.inertial().has_ixx())
    item->setValue(_msg.inertial().ixx());
  else
    item->setValue(0.0);
  inertialItem->addSubProperty(item);

  // Inertial::ixy
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("ixy"));
  if (_msg.inertial().has_ixy())
    item->setValue(_msg.inertial().ixy());
  else
    item->setValue(0.0);
  inertialItem->addSubProperty(item);

  // Inertial::ixz
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("ixz"));
  if (_msg.inertial().has_ixz())
    item->setValue(_msg.inertial().ixz());
  else
    item->setValue(0.0);
  inertialItem->addSubProperty(item);

  // Inertial::iyy
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("iyy"));
  if (_msg.inertial().has_iyy())
    item->setValue(_msg.inertial().iyy());
  else
    item->setValue(0.0);
  inertialItem->addSubProperty(item);

  // Inertial::iyz
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("iyz"));
  if (_msg.inertial().has_iyz())
    item->setValue(_msg.inertial().iyz());
  else
    item->setValue(0.0);
  inertialItem->addSubProperty(item);

  // Inertial::izz
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("izz"));
  if (_msg.inertial().has_izz())
    item->setValue(_msg.inertial().izz());
  else
    item->setValue(0.0);
  inertialItem->addSubProperty(item);

  topItem = this->variantManager->addProperty(
      QtVariantPropertyManager::groupTypeId(),
      QLatin1String("pose"));
  inertialItem->addSubProperty(topItem);
  this->FillPoseProperty(_msg.inertial().pose(), topItem);


  for (int i=0; i < _msg.collision_size(); i++)
  {
    topItem = this->variantManager->addProperty(
        QtVariantPropertyManager::groupTypeId(),
        QLatin1String("collision"));
    _parent->addSubProperty(topItem);
 
    this->FillPropertyTree(_msg.collision(i),topItem);
  }


  for (int i=0; i < _msg.visual_size(); i++)
  {
    topItem = this->variantManager->addProperty(
        QtVariantPropertyManager::groupTypeId(),
        QLatin1String("visual"));
    _parent->addSubProperty(topItem);
    this->FillPropertyTree(_msg.visual(i),topItem);
  }

  for (int i=0; i < _msg.sensor_size(); i++)
  {
    topItem = this->variantManager->addProperty(
        QtVariantPropertyManager::groupTypeId(),
        QLatin1String("sensor"));
    _parent->addSubProperty(topItem);
 
    //this->FillPropertyTree(_msg.sensor(i),topItem);
  }

}

void ModelListWidget::FillPropertyTree(const msgs::Collision &_msg,
                                       QtProperty *_parent)
{
  QtProperty *topItem = NULL;
  QtVariantProperty *item = NULL;

  item = this->variantManager->addProperty(QVariant::String,
                                           QLatin1String("name"));
  item->setValue(_msg.name().c_str());
  _parent->addSubProperty(item);

  // Laser Retro value
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("laser_retro"));
  if (_msg.has_laser_retro())
    item->setValue(_msg.laser_retro());
  else
    item->setValue(0.0);
  _parent->addSubProperty(item);

  // Pose value
  topItem = this->variantManager->addProperty(
      QtVariantPropertyManager::groupTypeId(),
      QLatin1String("pose"));
  _parent->addSubProperty(topItem);
  this->FillPoseProperty(_msg.pose(), topItem);

  // Geometry shape value
  topItem = this->variantManager->addProperty(
      QtVariantPropertyManager::groupTypeId(),
      QLatin1String("geometry"));
  _parent->addSubProperty(topItem);
  this->FillPropertyTree(_msg.geometry(), topItem);

  // Surface value
  topItem = this->variantManager->addProperty(
      QtVariantPropertyManager::groupTypeId(),
      QLatin1String("surface"));
  _parent->addSubProperty(topItem);
  this->FillPropertyTree(_msg.surface(), topItem);
}

void ModelListWidget::FillPropertyTree(const msgs::Surface &_msg,
                                       QtProperty *_parent)
{
  QtProperty *topItem = NULL;
  QtVariantProperty *item = NULL;

  // Restituion Coefficient
  item = this->variantManager->addProperty(QVariant::Double,
      QLatin1String("restitution_coefficient"));
  item->setValue(_msg.restitution_coefficient());
  _parent->addSubProperty(item);

  // Bounce Threshold
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("bounce_threshold"));
  item->setValue(_msg.bounce_threshold());
  _parent->addSubProperty(item);

  // Soft CFM
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("soft_cfm"));
  item->setValue(_msg.soft_cfm());
  _parent->addSubProperty(item);

  // Soft ERP
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("soft_erp"));
  item->setValue(_msg.soft_erp());
  _parent->addSubProperty(item);

  // KP
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("kp"));
  item->setValue(_msg.kp());
  _parent->addSubProperty(item);

  // KD
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("kd"));
  item->setValue(_msg.kd());
  _parent->addSubProperty(item);

  // max vel
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("max_vel"));
  item->setValue(_msg.max_vel());
  _parent->addSubProperty(item);

  // min depth
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("min_depth"));
  item->setValue(_msg.min_depth());
  _parent->addSubProperty(item);

  // Friction
  topItem = this->variantManager->addProperty(
      QtVariantPropertyManager::groupTypeId(),
      QLatin1String("friction"));
  _parent->addSubProperty(topItem);

  // Mu
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("mu"));
  item->setValue(_msg.friction().mu());
  topItem->addSubProperty(item);

  // Mu2
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("mu2"));
  item->setValue(_msg.friction().mu2());
  topItem->addSubProperty(item);

  // slip1
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("slip1"));
  item->setValue(_msg.friction().slip1());
  topItem->addSubProperty(item);

  // slip2
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("slip2"));
  item->setValue(_msg.friction().slip2());
  topItem->addSubProperty(item);

  // Fdir1
  QtProperty *fdirItem = this->variantManager->addProperty(
      QtVariantPropertyManager::groupTypeId(),
      QLatin1String("fdir1"));
    topItem->addSubProperty(fdirItem);
    this->FillVector3dProperty(_msg.friction().fdir1(), fdirItem);
}

void ModelListWidget::FillPropertyTree(const msgs::Geometry &_msg,
                                       QtProperty *_parent)
{
  QtVariantProperty *item = NULL;

  item = this->variantManager->addProperty(
      QtVariantPropertyManager::enumTypeId(), QLatin1String("type"));
  QStringList types;
  types << "BOX" << "SPHERE" << "CYLINDER" << "PLANE" << "MESH" << "IMAGE" 
        << "HEIGHTMAP";
  item->setAttribute("enumNames",types);
  _parent->addSubProperty(item);

  if (_msg.type() == msgs::Geometry::BOX)
  {
    item->setValue(0);
    QtProperty *sizeItem = this->variantManager->addProperty(
        QtVariantPropertyManager::groupTypeId(),
        QLatin1String("size"));
    _parent->addSubProperty(sizeItem);
    this->FillVector3dProperty(_msg.box().size(), sizeItem);
  }
  else if (_msg.type() == msgs::Geometry::SPHERE)
  {
    item->setValue(1);

    item = this->variantManager->addProperty(QVariant::Double,
        QLatin1String("radius"));
    item->setValue(_msg.sphere().radius());
    _parent->addSubProperty(item);
  }
  else if (_msg.type() == msgs::Geometry::CYLINDER)
  {
    item->setValue(2);
    item = this->variantManager->addProperty(QVariant::Double,
        QLatin1String("radius"));
    item->setValue(_msg.cylinder().radius());
    _parent->addSubProperty(item);

    item = this->variantManager->addProperty(QVariant::Double,
        QLatin1String("length"));
    item->setValue(_msg.cylinder().length());
    _parent->addSubProperty(item);
  }
  else if (_msg.type() == msgs::Geometry::PLANE)
  {
    item->setValue(3);
    QtProperty *normalItem = this->variantManager->addProperty(
        QtVariantPropertyManager::groupTypeId(),
        QLatin1String("normal"));
    _parent->addSubProperty(normalItem);
    this->FillVector3dProperty(_msg.plane().normal(), normalItem);
  }
  else if (_msg.type() == msgs::Geometry::MESH)
  {
    item->setValue(4);

    item = this->variantManager->addProperty(QVariant::String,
        QLatin1String("filename"));
    item->setValue(_msg.mesh().filename().c_str());
    _parent->addSubProperty(item);

    QtProperty *sizeItem = this->variantManager->addProperty(
        QtVariantPropertyManager::groupTypeId(),
        QLatin1String("scale"));
    _parent->addSubProperty(sizeItem);
    this->FillVector3dProperty(_msg.mesh().scale(), sizeItem);
  }
  else if (_msg.type() == msgs::Geometry::IMAGE)
  {
    item->setValue(5);

    item = this->variantManager->addProperty(QVariant::String,
        QLatin1String("filename"));
    item->setValue(_msg.image().filename().c_str());
    _parent->addSubProperty(item);

    item = this->variantManager->addProperty(QVariant::Double,
        QLatin1String("scale"));
    item->setValue(_msg.image().scale());
    _parent->addSubProperty(item);

    item = this->variantManager->addProperty(QVariant::Double,
        QLatin1String("height"));
    item->setValue(_msg.image().height());
    _parent->addSubProperty(item);

    item = this->variantManager->addProperty(QVariant::Int,
        QLatin1String("threshold"));
    item->setValue(_msg.image().threshold());
    _parent->addSubProperty(item);

    item = this->variantManager->addProperty(QVariant::Int,
        QLatin1String("granularity"));
    item->setValue(_msg.image().granularity());
    _parent->addSubProperty(item);
  }
  else if (_msg.type() == msgs::Geometry::HEIGHTMAP)
  {
    item->setValue(6);

    item = this->variantManager->addProperty(QVariant::String,
        QLatin1String("filename"));
    item->setValue(_msg.image().filename().c_str());
    _parent->addSubProperty(item);

    QtProperty *sizeItem = this->variantManager->addProperty(
        QtVariantPropertyManager::groupTypeId(),
        QLatin1String("size"));
    _parent->addSubProperty(sizeItem);
    this->FillVector3dProperty(_msg.heightmap().size(), sizeItem);

    sizeItem = this->variantManager->addProperty(
        QtVariantPropertyManager::groupTypeId(),
        QLatin1String("offset"));
    _parent->addSubProperty(sizeItem);
    this->FillVector3dProperty(_msg.heightmap().offset(), sizeItem);
  }

}

void ModelListWidget::FillPropertyTree(const msgs::Visual &_msg,
                                       QtProperty *_parent)
{
  QtProperty *topItem = NULL;
  QtVariantProperty *item = NULL;

  // Name value
  item = this->variantManager->addProperty(QVariant::String,
                                           QLatin1String("name"));
  item->setValue(_msg.name().c_str());
  _parent->addSubProperty(item);

  // Laser Retro value
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("laser_retro"));
  if (_msg.has_laser_retro())
    item->setValue(_msg.laser_retro());
  else
    item->setValue(0.0);
  _parent->addSubProperty(item);

  // cast shadows value
  item = this->variantManager->addProperty(QVariant::Bool,
                                           QLatin1String("cast_shadows"));
  if (_msg.has_cast_shadows())
    item->setValue(_msg.cast_shadows());
  else
    item->setValue(true);
  _parent->addSubProperty(item);

  // transparency
  item = this->variantManager->addProperty(QVariant::Double,
                                           QLatin1String("transparency"));
  if (_msg.has_transparency())
    item->setValue(_msg.transparency());
  else
    item->setValue(0.0);
  _parent->addSubProperty(item);


  // Pose value
  topItem = this->variantManager->addProperty(
      QtVariantPropertyManager::groupTypeId(),
      QLatin1String("pose"));
  _parent->addSubProperty(topItem);
  this->FillPoseProperty(_msg.pose(), topItem);

  // Geometry shape value
  topItem = this->variantManager->addProperty(
      QtVariantPropertyManager::groupTypeId(),
      QLatin1String("geometry"));
  _parent->addSubProperty(topItem);
  this->FillPropertyTree(_msg.geometry(), topItem);
}

void ModelListWidget::FillPropertyTree(const msgs::Model &_msg,
                                       QtProperty * /*_parent*/)
{
  QtProperty *topItem = NULL;
  QtVariantProperty *item = NULL;

  item = this->variantManager->addProperty(QVariant::String,
                                           QLatin1String("name"));
  item->setValue(_msg.name().c_str());
  this->propTreeBrowser->addProperty(item);


  item = this->variantManager->addProperty(QVariant::Bool,
                                           QLatin1String("is_static"));
  if (_msg.has_is_static())
    item->setValue(_msg.is_static());
  else
    item->setValue(false);
  this->propTreeBrowser->addProperty(item);

  topItem = this->variantManager->addProperty(
      QtVariantPropertyManager::groupTypeId(),
      QLatin1String("pose"));
  this->propTreeBrowser->addProperty(topItem);
  this->FillPoseProperty(_msg.pose(), topItem);

  for (int i=0; i < _msg.link_size(); i++)
  {
    topItem = this->variantManager->addProperty(
        QtVariantPropertyManager::groupTypeId(),
        QLatin1String("link"));
    this->propTreeBrowser->addProperty(topItem);
 
    this->FillPropertyTree(_msg.link(i),topItem);
  }
}

void ModelListWidget::FillVector3dProperty(const msgs::Vector3d &_msg,
                                           QtProperty *_parent)
{
  QtVariantProperty *item;
  math::Vector3 value;
  value = msgs::Convert(_msg);
  value.Round(6);

  // Add X value
  item = (QtVariantProperty*)this->GetChildItem(_parent, "x");
  if (!item)
  {
    item = this->variantManager->addProperty(QVariant::Double, "x");
    if (_parent)
      _parent->addSubProperty(item);
  }
  ((QtVariantPropertyManager*)
   this->variantFactory->propertyManager(item))->setAttribute(item,
   "decimals", 6);
  item->setValue(value.x);

  // Add Y value
  item = (QtVariantProperty*)this->GetChildItem(_parent, "y");
  if (!item)
  {
    item = this->variantManager->addProperty(QVariant::Double, "y");
    if (_parent)
      _parent->addSubProperty(item);
  }
  ((QtVariantPropertyManager*)
   this->variantFactory->propertyManager(item))->setAttribute(item,
   "decimals", 6);
  item->setValue(value.y);

  // Add Z value
  item = (QtVariantProperty*)this->GetChildItem(_parent, "z");
  if (!item)
  {
    item = this->variantManager->addProperty( QVariant::Double, "z");
    if (_parent)
      _parent->addSubProperty(item);
  }
  ((QtVariantPropertyManager*)
   this->variantFactory->propertyManager(item))->setAttribute(item,
   "decimals", 6);
  item->setValue(value.z);
}

void ModelListWidget::FillPoseProperty(const msgs::Pose &_msg,
                                       QtProperty *_parent)
{
  QtVariantProperty *item;
  math::Pose value;
  value = msgs::Convert(_msg);
  value.Round(6);

  math::Vector3 rpy = value.rot.GetAsEuler();
  rpy.Round(6);

  this->FillVector3dProperty(_msg.position(), _parent);

  // Add Roll value
  item = (QtVariantProperty*)this->GetChildItem(_parent, "roll");
  if (!item)
  {
    item = this->variantManager->addProperty( QVariant::Double, "roll");
    if (_parent)
      _parent->addSubProperty(item);
  }
  ((QtVariantPropertyManager*)this->variantFactory->propertyManager(item))->setAttribute(item, "decimals", 6);
  item->setValue(RTOD(rpy.x));

  // Add Pitch value
  item = (QtVariantProperty*)this->GetChildItem(_parent, "pitch");
  if (!item)
  {
    item = this->variantManager->addProperty( QVariant::Double, "pitch");
    if (_parent)
      _parent->addSubProperty(item);
  }
  ((QtVariantPropertyManager*)this->variantFactory->propertyManager(item))->setAttribute(item, "decimals", 6);
  item->setValue(RTOD(rpy.y));

  // Add Yaw value
  item = (QtVariantProperty*)this->GetChildItem(_parent, "yaw");
  if (!item)
  {
    item = this->variantManager->addProperty( QVariant::Double, "yaw");
    if (_parent)
      _parent->addSubProperty(item);
  }
  ((QtVariantPropertyManager*)this->variantFactory->propertyManager(item))->setAttribute(item, "decimals", 6);
  item->setValue(RTOD(rpy.z));
}

void ModelListWidget::ProcessPoseMsgs()
{
  this->receiveMutex->lock();
  this->propMutex->lock();
  this->fillingPropertyTree = true;

  std::list<msgs::Pose>::iterator iter;
  for (iter = this->poseMsgs.begin(); iter != this->poseMsgs.end(); iter++)
  {
    if ((*iter).name().find(this->selectedModelName) != std::string::npos)
    {
      QtProperty *poseItem;
      QtProperty *nameItem = this->GetParentItemValue((*iter).name());
      if (!nameItem)
        poseItem = this->GetChildItem("pose");
      else
        poseItem = this->GetChildItem(nameItem, "pose");
      this->FillPoseProperty(*iter, poseItem);
    }
  }
  this->poseMsgs.clear();
  this->fillingPropertyTree = false;
  this->propMutex->unlock();
  this->receiveMutex->unlock();
}

void ModelListWidget::OnPose(
    const boost::shared_ptr<msgs::Pose const> &_msg)
{
  this->receiveMutex->lock();
  this->poseMsgs.push_back(*_msg);
  this->receiveMutex->unlock();
}

void ModelListWidget::OnRequest(
    const boost::shared_ptr<msgs::Request const> &_msg)
{
  if (_msg->request() == "entity_delete")
  {
    this->RemoveEntity(_msg->data());
  }
}
