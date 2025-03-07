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

#include <sstream>

#include "gazebo/msgs/msgs.hh"
#include "gazebo/msgs/MessageTypes.hh"

#include "gazebo/common/Events.hh"
#include "gazebo/common/Console.hh"
#include "gazebo/transport/TransportIface.hh"

#include "gazebo/transport/Publisher.hh"

#include "gazebo/physics/World.hh"
#include "gazebo/physics/ContactManager.hh"
#include "gazebo/physics/PhysicsIface.hh"
#include "gazebo/physics/Contact.hh"
#include "gazebo/physics/Shape.hh"
#include "gazebo/physics/SurfaceParams.hh"
#include "gazebo/physics/Model.hh"
#include "gazebo/physics/Link.hh"
#include "gazebo/physics/Collision.hh"

using namespace gazebo;
using namespace physics;

//////////////////////////////////////////////////
Collision::Collision(LinkPtr _link)
    : Entity(_link), maxContacts(1)
{
  this->AddType(Base::COLLISION);

  this->link = _link;

  this->placeable = false;

  sdf::initFile("collision.sdf", this->sdf);

  this->collisionVisualId = physics::getUniqueId();
}

//////////////////////////////////////////////////
Collision::~Collision()
{
}

//////////////////////////////////////////////////
void Collision::Fini()
{
  if (this->requestPub)
  {
    msgs::Request *msg = msgs::CreateRequest("entity_delete",
        this->GetScopedName()+"__COLLISION_VISUAL__");
    this->requestPub->Publish(*msg, true);
  }

  Entity::Fini();
  this->link.reset();
  this->shape.reset();
  this->surface.reset();
}

//////////////////////////////////////////////////
void Collision::Load(sdf::ElementPtr _sdf)
{
  Entity::Load(_sdf);

  this->maxContacts = _sdf->Get<unsigned int>("max_contacts");
  this->SetMaxContacts(this->maxContacts);

  if (this->sdf->HasElement("laser_retro"))
    this->SetLaserRetro(this->sdf->Get<double>("laser_retro"));

  this->SetRelativePose(this->sdf->Get<math::Pose>("pose"));

  this->surface->Load(this->sdf->GetElement("surface"));

  if (this->shape)
    this->shape->Load(this->sdf->GetElement("geometry")->GetFirstElement());
  else
    gzwarn << "No shape has been specified. Error!!!\n";
}

//////////////////////////////////////////////////
void Collision::Init()
{
  this->shape->Init();

  this->SetRelativePose(
    this->sdf->Get<math::Pose>("pose"));
}

//////////////////////////////////////////////////
void Collision::SetCollision(bool _placeable)
{
  this->placeable = _placeable;

  if (this->IsStatic())
  {
    this->SetCategoryBits(GZ_FIXED_COLLIDE);
    this->SetCollideBits(~GZ_FIXED_COLLIDE);
  }
  else
  {
    // collide with all
    this->SetCategoryBits(GZ_ALL_COLLIDE);
    this->SetCollideBits(GZ_ALL_COLLIDE);
  }
}

//////////////////////////////////////////////////
bool Collision::IsPlaceable() const
{
  return this->placeable;
}


//////////////////////////////////////////////////
void Collision::SetLaserRetro(float _retro)
{
  this->sdf->GetElement("laser_retro")->Set(_retro);
  this->laserRetro = _retro;
}

//////////////////////////////////////////////////
float Collision::GetLaserRetro() const
{
  return this->laserRetro;
}

//////////////////////////////////////////////////
LinkPtr Collision::GetLink() const
{
  return this->link;
}

//////////////////////////////////////////////////
ModelPtr Collision::GetModel() const
{
  return this->link->GetModel();
}

//////////////////////////////////////////////////
unsigned int Collision::GetShapeType() const
{
  return this->shape->GetType();
}

//////////////////////////////////////////////////
void Collision::SetShape(ShapePtr _shape)
{
  this->shape = _shape;
}

//////////////////////////////////////////////////
ShapePtr Collision::GetShape() const
{
  return this->shape;
}

//////////////////////////////////////////////////
void Collision::SetScale(const math::Vector3 &_scale)
{
  this->shape->SetScale(_scale);
}

//////////////////////////////////////////////////
math::Vector3 Collision::GetRelativeLinearVel() const
{
  if (this->link)
    return this->link->GetRelativeLinearVel();
  else
    return math::Vector3();
}

//////////////////////////////////////////////////
math::Vector3 Collision::GetWorldLinearVel() const
{
  if (this->link)
    return this->link->GetWorldLinearVel();
  else
    return math::Vector3();
}

//////////////////////////////////////////////////
math::Vector3 Collision::GetRelativeAngularVel() const
{
  if (this->link)
    return this->link->GetRelativeAngularVel();
  else
    return math::Vector3();
}

//////////////////////////////////////////////////
math::Vector3 Collision::GetWorldAngularVel() const
{
  if (this->link)
    return this->link->GetWorldAngularVel();
  else
    return math::Vector3();
}

//////////////////////////////////////////////////
math::Vector3 Collision::GetRelativeLinearAccel() const
{
  if (this->link)
    return this->link->GetRelativeLinearAccel();
  else
    return math::Vector3();
}

//////////////////////////////////////////////////
math::Vector3 Collision::GetWorldLinearAccel() const
{
  if (this->link)
    return this->link->GetWorldLinearAccel();
  else
    return math::Vector3();
}

//////////////////////////////////////////////////
math::Vector3 Collision::GetRelativeAngularAccel() const
{
  if (this->link)
    return this->link->GetRelativeAngularAccel();
  else
    return math::Vector3();
}

//////////////////////////////////////////////////
math::Vector3 Collision::GetWorldAngularAccel() const
{
  if (this->link)
    return this->link->GetWorldAngularAccel();
  else
    return math::Vector3();
}

//////////////////////////////////////////////////
void Collision::UpdateParameters(sdf::ElementPtr _sdf)
{
  Entity::UpdateParameters(_sdf);
}

//////////////////////////////////////////////////
void Collision::FillMsg(msgs::Collision &_msg)
{
  msgs::Set(_msg.mutable_pose(), this->GetRelativePose().Ign());
  _msg.set_id(this->GetId());
  _msg.set_name(this->GetScopedName());
  _msg.set_laser_retro(this->GetLaserRetro());

  this->shape->FillMsg(*_msg.mutable_geometry());
  this->surface->FillMsg(*_msg.mutable_surface());

  msgs::Set(this->visualMsg->mutable_pose(), this->GetRelativePose().Ign());

  if (!this->HasType(physics::Base::SENSOR_COLLISION))
  {
    _msg.add_visual()->CopyFrom(*this->visualMsg);
    // TODO remove the need to create the special collision visual msg and
    // let the gui handle this.
    _msg.add_visual()->CopyFrom(this->CreateCollisionVisual());
  }
}

//////////////////////////////////////////////////
void Collision::ProcessMsg(const msgs::Collision &_msg)
{
  if (_msg.id() != this->GetId())
  {
    gzerr << "Incorrect ID\n";
    return;
  }

  this->SetName(_msg.name());
  if (_msg.has_laser_retro())
    this->SetLaserRetro(_msg.laser_retro());

  if (_msg.has_pose())
  {
    this->link->SetEnabled(true);
    this->SetRelativePose(msgs::ConvertIgn(_msg.pose()));
  }

  if (_msg.has_geometry())
  {
    this->link->SetEnabled(true);
    this->shape->ProcessMsg(_msg.geometry());
  }

  if (_msg.has_surface())
  {
    this->link->SetEnabled(true);
    this->surface->ProcessMsg(_msg.surface());
  }
}

/////////////////////////////////////////////////
msgs::Visual Collision::CreateCollisionVisual()
{
  msgs::Visual msg;
  msg.set_name(this->GetScopedName()+"__COLLISION_VISUAL__");

  // Put in a unique ID because this is a special visual.
  msg.set_id(this->collisionVisualId);

  auto parent_ = this->parent.lock();
  if (parent_) {
      msg.set_parent_name(parent_->GetScopedName());
      msg.set_parent_id(parent_->GetId());
  }
  else
  {
      // TODO What else if there is no parent?
      msg.set_parent_id(0);
  }

  msg.set_is_static(this->IsStatic());
  msg.set_cast_shadows(false);
  msg.set_type(msgs::Visual::COLLISION);
  msgs::Set(msg.mutable_pose(), this->GetRelativePose().Ign());
  msg.mutable_material()->mutable_script()->add_uri(
      "file://media/materials/scripts/gazebo.material");
  msg.mutable_material()->mutable_script()->set_name(
      "Gazebo/OrangeTransparent");
  msgs::Geometry *geom = msg.mutable_geometry();
  geom->CopyFrom(msgs::GeometryFromSDF(this->sdf->GetElement("geometry")));

  return msg;
}

/////////////////////////////////////////////////
CollisionState Collision::GetState()
{
  return this->state;
}

/////////////////////////////////////////////////
void Collision::SetState(const CollisionState &_state)
{
  this->SetRelativePose(_state.GetPose());
}

/////////////////////////////////////////////////
void Collision::SetMaxContacts(unsigned int _maxContacts)
{
  this->maxContacts = _maxContacts;
  this->sdf->GetElement("max_contacts")->GetValue()->Set(_maxContacts);
}

/////////////////////////////////////////////////
unsigned int Collision::GetMaxContacts()
{
  return this->maxContacts;
}

/////////////////////////////////////////////////
const math::Pose &Collision::GetWorldPose() const
{
  // If true, compute a new world pose value.
  //
  if (this->worldPoseDirty)
  {
    this->worldPose = this->GetInitialRelativePose() +
                      this->link->GetWorldPose();
    this->worldPoseDirty = false;
  }

  return this->worldPose;
}

/////////////////////////////////////////////////
void Collision::SetWorldPoseDirty()
{
  // Tell the collision object that the next call to ::GetWorldPose should
  // compute a new worldPose value.
  this->worldPoseDirty = true;
}
