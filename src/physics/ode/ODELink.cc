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
/* Desc: Link class
 * Author: Nate Koenig
 * Date: 13 Feb 2006
 */

#include <sstream>
#include <math.h>

#include "common/Console.hh"
#include "common/Exception.hh"

#include "physics/Collision.hh"
#include "physics/World.hh"
#include "physics/Model.hh"
#include "physics/ode/ODECollision.hh"
#include "physics/ode/ODEPhysics.hh"
#include "physics/ode/ODELink.hh"

using namespace gazebo;
using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// Constructor
ODELink::ODELink(EntityPtr parent)
    : Link(parent)
{
  this->linkId = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor
ODELink::~ODELink()
{
  if (this->linkId)
    dBodyDestroy(this->linkId);
  this->linkId = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Load the link
void ODELink::Load( sdf::ElementPtr &_sdf)
{
  this->odePhysics = boost::shared_dynamic_cast<ODEPhysics>(
      this->GetWorld()->GetPhysicsEngine());

  if (this->odePhysics == NULL)
    gzthrow("Not using the ode physics engine");

  Link::Load(_sdf);
}


////////////////////////////////////////////////////////////////////////////////
// Init the ODE link
void ODELink::Init() 
{
  if (!this->IsStatic())
  {
    this->linkId = dBodyCreate(this->odePhysics->GetWorldId());
    dBodySetData(this->linkId, this);

    // Only use auto disable if no joints are present
    if (this->GetModel()->GetJointCount() == 0)
    {
      /*dBodySetAutoDisableDefaults(this->linkId);
      dBodySetAutoDisableFlag(this->linkId, 1);
      */
    }
  }

  Link::Init();

  if (this->linkId)
  {
    math::Vector3 cog_vec = this->inertial->GetCoG();
    Base_V::iterator iter;
    for (iter = this->children.begin(); iter != this->children.end(); iter++)
    {
      if ((*iter)->HasType(Base::COLLISION))
      {
        ODECollisionPtr g = boost::shared_static_cast<ODECollision>(*iter);
        if (g->IsPlaceable() && g->GetCollisionId())
        {
          dGeomSetBody(g->GetCollisionId(), this->linkId);

          // update pose immediately
          math::Pose localPose = g->GetRelativePose();
          localPose.pos -= cog_vec;

          dQuaternion q;
          q[0] = localPose.rot.w;
          q[1] = localPose.rot.x;
          q[2] = localPose.rot.y;
          q[3] = localPose.rot.z;

          // Set the pose of the encapsulated collision; this is always relative
          // to the CoM
          dGeomSetOffsetPosition(g->GetCollisionId(), localPose.pos.x, 
              localPose.pos.y, localPose.pos.z);
          dGeomSetOffsetQuaternion(g->GetCollisionId(), q);
        }
      }
    }
  }
 
  // Update the Center of Mass.
  this->UpdateMass();

  if (this->linkId)
  {
    dBodySetMovedCallback(this->linkId, MoveCallback);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Move callback. Use this to move the visuals
void ODELink::MoveCallback(dBodyID id)
{
  const dReal *p;
  const dReal *r;
  ODELink *self = (ODELink*)(dBodyGetData(id));
  //self->poseMutex->lock();

  p = dBodyGetPosition(id);
  r = dBodyGetQuaternion(id);

  self->dirtyPose.pos.Set(p[0], p[1], p[2]);
  self->dirtyPose.rot.Set(r[0], r[1], r[2], r[3] );

  // subtracting cog location from ode pose
  math::Vector3 cog_vec = self->dirtyPose.rot.RotateVector(
      self->inertial->GetCoG());

  self->dirtyPose.pos -= cog_vec;

  self->world->dirtyPoses.push_back( self );

  //self->poseMutex->unlock();
}

////////////////////////////////////////////////////////////////////////////////
/// Finalize the link
void ODELink::Fini()
{
  Link::Fini();
  if (this->linkId)
    dBodyDestroy(this->linkId);
  this->linkId = NULL;

  this->odePhysics.reset();
}

////////////////////////////////////////////////////////////////////////////////
// Update the link
void ODELink::Update()
{
  Link::Update();
}

////////////////////////////////////////////////////////////////////////////////
// Set whether gravity affects this link
void ODELink::SetGravityMode(bool _mode)
{
  this->sdf->GetAttribute("gravity")->Set(_mode);
  if (this->linkId)
  {
    dBodySetGravityMode(this->linkId, _mode ? 1: 0);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Get the gravity mode
bool ODELink::GetGravityMode()
{
  int mode = 0;
  if (this->linkId)
  {
    mode = dBodyGetGravityMode(this->linkId);
  }

  return mode;
}

////////////////////////////////////////////////////////////////////////////////
// Set whether this link will collide with others in the model
void ODELink::SetSelfCollide(bool _collide)
{
  this->sdf->GetAttribute("self_collide")->Set(_collide);
  if (_collide && !this->spaceId)
    this->spaceId = dSimpleSpaceCreate(this->odePhysics->GetSpaceId());
}

////////////////////////////////////////////////////////////////////////////////
// Change the ode pose
void ODELink::OnPoseChange()
{
  Link::OnPoseChange();

  if (!this->linkId)
    return;

  this->SetEnabled(true);

  const math::Pose pose = this->GetWorldPose();

  math::Vector3 cog_vec = pose.rot.RotateVector(this->inertial->GetCoG());

  // adding cog location for ode pose
  dBodySetPosition(this->linkId, 
      pose.pos.x + cog_vec.x, 
      pose.pos.y + cog_vec.y, 
      pose.pos.z + cog_vec.z);

  dQuaternion q;
  q[0] = pose.rot.w;
  q[1] = pose.rot.x;
  q[2] = pose.rot.y;
  q[3] = pose.rot.z;

  // Set the rotation of the ODE link
  dBodySetQuaternion(this->linkId, q);
}

////////////////////////////////////////////////////////////////////////////////
// Return the ID of this link
dBodyID ODELink::GetODEId() const
{
  return this->linkId;
}


////////////////////////////////////////////////////////////////////////////////
// Set whether this link is enabled
void ODELink::SetEnabled(bool _enable) const
{
  if (!this->linkId)
    return;

  if (_enable)
    dBodyEnable(this->linkId);
  else
    dBodyDisable(this->linkId);
}

/////////////////////////////////////////////////////////////////////
/// Get whether this link is enabled in the physics engine
bool ODELink::GetEnabled() const
{
  bool result = true;

  if (this->linkId)
    result = dBodyIsEnabled(this->linkId);

  return result;
}

/////////////////////////////////////////////////////////////////////
// Update the mass matrix
void ODELink::UpdateMass()
{
  if (!this->linkId)
    return;

  dMass odeMass;
  dMassSetZero(&odeMass);

  // The CoG must always be (0,0,0)
  math::Vector3 cog(0,0,0);

  math::Vector3 principals = this->inertial->GetPrincipalMoments();
  math::Vector3 products = this->inertial->GetProductsofInertia();

  dMassSetParameters(&odeMass, this->inertial->GetMass(),
      cog.x, cog.y, cog.z,
      principals.x, principals.y, principals.z,
      products.x, products.y, products.z);

  if (this->inertial->GetMass() > 0)
    dBodySetMass(this->linkId, &odeMass);
  else
    gzthrow("Setting custom link " + this->GetName()+"mass to zero!");
}

////////////////////////////////////////////////////////////////////////////////
/// Set the velocity of the link
void ODELink::SetLinearVel(const math::Vector3 &_vel)
{
  if (this->linkId)
  {
    dBodySetLinearVel(this->linkId, _vel.x, _vel.y, _vel.z);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Get the velocity of the link in the world frame
math::Vector3 ODELink::GetWorldLinearVel() const
{
  math::Vector3 vel;

  if (this->linkId)
  {
    const dReal *dvel;
    dvel = dBodyGetLinearVel(this->linkId);
    vel.Set(dvel[0], dvel[1], dvel[2]);
  }

  return vel;
}

////////////////////////////////////////////////////////////////////////////////
/// Set the velocity of the link
void ODELink::SetAngularVel(const math::Vector3 &vel)
{
  if (this->linkId)
  {
    dBodySetAngularVel(this->linkId, vel.x, vel.y, vel.z);
  }
}



////////////////////////////////////////////////////////////////////////////////
/// Get the angular velocity of the link in the world frame
math::Vector3 ODELink::GetWorldAngularVel() const
{
  math::Vector3 vel;

  if (this->linkId)
  {
    const dReal *dvel;

    dvel = dBodyGetAngularVel(this->linkId);

    vel.Set(dvel[0], dvel[1], dvel[2]);
  }

  return vel;
}

void ODELink::SetForce(const math::Vector3 &_force)
{
  if (this->linkId)
    dBodySetForce(this->linkId, _force.x, _force.y, _force.z);
}

void ODELink::SetTorque(const math::Vector3 &_torque)
{
  if (this->linkId)
    dBodySetTorque(this->linkId, _torque.x, _torque.y, _torque.z);
}

void ODELink::AddForce(const math::Vector3 &_force)
{
  if (this->linkId)
    dBodyAddForce(this->linkId, _force.x, _force.y, _force.z);
}

void ODELink::AddRelForce(const math::Vector3 &_force)
{
  if (this->linkId)
	  dBodyAddRelForce(this->linkId, _force.x, _force.y, _force.z);
}

void ODELink::AddForceAtRelPos(const math::Vector3 &_force,const math::Vector3 &_relpos)
{
  if (this->linkId)
    dBodyAddForceAtRelPos(this->linkId, _force.x, _force.y, _force.z,_relpos.x,_relpos.y,_relpos.z);
}

  
void ODELink::AddTorque(const math::Vector3 &_torque)
{
  if (this->linkId)
    dBodyAddTorque(this->linkId, _torque.x, _torque.y, _torque.z);
}

void ODELink::AddRelTorque(const math::Vector3 &_torque)
{
  if (this->linkId)
	  dBodyAddRelTorque(this->linkId, _torque.x, _torque.y, _torque.z);
}

////////////////////////////////////////////////////////////////////////////////
/// \brief Get the force applied to the link in the world frame
math::Vector3 ODELink::GetWorldForce() const
{
  math::Vector3 force;

  if (this->linkId)
  {
    const dReal *dforce;

    dforce = dBodyGetForce(this->linkId);

    force.x = dforce[0];
    force.y = dforce[1];
    force.z = dforce[2];
  }

  return force;
}




////////////////////////////////////////////////////////////////////////////////
/// Get the torque applied to the link in the world frame
math::Vector3 ODELink::GetWorldTorque() const
{
  math::Vector3 torque;

  if (this->linkId)
  {
    const dReal *dtorque;

    dtorque = dBodyGetTorque(this->linkId);
    
    torque.x = dtorque[0];
    torque.y = dtorque[1];
    torque.z = dtorque[2];
  }

  return torque;
}

////////////////////////////////////////////////////////////////////////////////
/// Get the bodies space ID
dSpaceID ODELink::GetSpaceId() const
{
  return this->spaceId;
}

////////////////////////////////////////////////////////////////////////////////
/// Set the bodies space ID
void ODELink::SetSpaceId(dSpaceID spaceid)
{
  this->spaceId = spaceid;
}

////////////////////////////////////////////////////////////////////////////////
/// Set the linear damping factor
void ODELink::SetLinearDamping(double damping)
{
  if (this->GetODEId())
    dBodySetLinearDamping(this->GetODEId(), damping); 
}

////////////////////////////////////////////////////////////////////////////////
/// Set the angular damping factor
void ODELink::SetAngularDamping(double damping)
{
  if (this->GetODEId())
    dBodySetAngularDamping(this->GetODEId(), damping); 
}

////////////////////////////////////////////////////////////////////////////////
// Set whether this link is in the kinematic state
void ODELink::SetKinematic(const bool &_state)
{
  this->sdf->GetAttribute("kinematic")->Set(_state);
  if (this->linkId)
  {
    if (_state)
      dBodySetKinematic(this->linkId);
    else
      dBodySetDynamic(this->linkId);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Get whether this link is in the kinematic state
bool ODELink::GetKinematic() const
{
  bool result = false;

  if (this->linkId)
    result = dBodyIsKinematic(this->linkId);

  return result;
}
