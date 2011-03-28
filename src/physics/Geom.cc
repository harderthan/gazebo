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
/* Desc: Geom class
 * Author: Nate Koenig
 * Date: 13 Feb 2006
 */

#include <sstream>

#include "common/Events.hh"
#include "Model.hh"
#include "Shape.hh"
#include "Mass.hh"
#include "PhysicsEngine.hh"
#include "common/Global.hh"
#include "common/GazeboMessage.hh"
#include "SurfaceParams.hh"
#include "World.hh"
#include "Body.hh"
#include "physics/Geom.hh"

using namespace gazebo;
using namespace physics;


////////////////////////////////////////////////////////////////////////////////
// Constructor
Geom::Geom( Body *body )
    : Entity(body->GetCoMEntity())
{
  this->AddType(GEOM);

  this->body = body;

  // Create the contact parameters
  this->surface = new SurfaceParams();

  this->transparency = 0;

  this->shape = NULL;

  this->contactsEnabled = false;

  common::Param::Begin(&this->parameters);

  this->massP = new common::ParamT<double>("mass",0.001,0);
  this->massP->Callback( &Geom::SetMass, this);

  this->xyzP = new common::ParamT<common::Vector3>("xyz", common::Vector3(), 0);
  this->xyzP->Callback( &Entity::SetRelativePosition, (Entity*)this);

  this->rpyP = new common::ParamT<common::Quatern>("rpy", common::Quatern(), 0);
  this->rpyP->Callback( &Entity::SetRelativeRotation, (Entity*)this);

  this->laserFiducialIdP = new common::ParamT<int>("laser_fiducial_id",-1,0);
  this->laserRetroP = new common::ParamT<float>("laser_retro",-1,0);

  this->enableContactsP = new common::ParamT<bool>("enable_contacts", false, 0);
  //this->enableContactsP->Callback( &Geom::SetContactsEnabled, this );
  
  common::Param::End();

  this->connections.push_back( event::Events::ConnectShowBoundingBoxesSignal( boost::bind(&Geom::ShowBoundingBox, this, _1) ) );
  this->connections.push_back( this->body->ConnectEnabledSignal( boost::bind(&Geom::EnabledCB, this, _1) ) );
}

////////////////////////////////////////////////////////////////////////////////
// Destructor
Geom::~Geom()
{
  if (!this->bbVisual.empty())
  {
    msgs::Visual msg;
    common::Message::Init(msg, this->bbVisual);
    msg.set_action( msgs::Visual::DELETE);
    this->vis_pub->Publish(msg);
  }

  delete this->massP;
  delete this->xyzP;
  delete this->rpyP;
  delete this->laserFiducialIdP;
  delete this->laserRetroP;
  delete this->enableContactsP;

  if (this->shape)
    delete this->shape;
  this->shape = NULL;
}

////////////////////////////////////////////////////////////////////////////////
/// Finalize the geom
void Geom::Fini()
{
  this->connections.clear();
}

////////////////////////////////////////////////////////////////////////////////
// First step in the loading process
void Geom::Load(common::XMLConfigNode *node)
{
  Entity::Load(node);

  this->SetName(this->nameP->GetValue());
  this->massP->Load(node);

  common::XMLConfigNode *childNode;
  if ( (childNode = node->GetChild("origin")) != NULL)
  {
    this->xyzP->Load(childNode);
    this->rpyP->Load(childNode);
  }

  this->laserFiducialIdP->Load(node);
  this->laserRetroP->Load(node);
  this->enableContactsP->Load(node);

  this->SetContactsEnabled(**this->enableContactsP);

  this->SetRelativePose( common::Pose3d( **this->xyzP, **this->rpyP ) );

  this->mass.SetMass( **this->massP );

  this->surface->Load(node);

  if (this->shape)
  {
    this->shape->Load(node->GetChild("geometry"));
  }
  else
    std::cerr << "No shape has been specified. Error!!!\n";

  this->CreateBoundingBox();

  this->body->AttachGeom(this);
}

////////////////////////////////////////////////////////////////////////////////
// Create the bounding box for the geom
void Geom::CreateBoundingBox()
{
  // Create the bounding box
  if (this->GetShapeType() != PLANE_SHAPE && this->GetShapeType() != MAP_SHAPE)
  {
    common::Vector3 min;
    common::Vector3 max;

    this->GetBoundingBox(min,max);

    std::ostringstream visname;
    visname << this->GetCompleteScopedName() << "::BBVISUAL" ;

    msgs::Visual msg;
    msg.set_render_type( msgs::Visual::MESH_RESOURCE );
    msg.set_parent_id( this->GetCompleteScopedName() );
    msg.mutable_header()->set_str_id( this->GetCompleteScopedName() + "_BBVISUAL" );
    msg.set_cast_shadows(false);
    // NATY: Set it so bounding boxes are visible upon creation if a flag is
    // set. The flag should exist only on the rendering side. May need to
    // put something in the message to indicate that this is a bounding box
    //msg.set_visible( RenderState::GetShowBoundingBoxes() );
    msg.set_mesh( "unit_box" );
    if (this->IsStatic() )
      msg.set_material( "Gazebo/YellowTransparent" );
    else
      msg.set_material( "Gazebo/GreenTransparent" );

    common::Message::Set(msg.mutable_scale(), (max - min) * 1.01);
    common::Message::Set(msg.mutable_pose()->mutable_position(), common::Vector3(0,0,0.0));
    common::Message::Set(msg.mutable_pose()->mutable_orientation(), common::Quatern(1,0,0,0));
    msg.set_transparency( .5 );

    this->vis_pub->Publish(msg);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Save the body based on our common::XMLConfig node
void Geom::Save(std::string &prefix, std::ostream &stream)
{
  if (!this->GetSaveable())
    return;

  std::string p = prefix + "  ";

  this->xyzP->SetValue( this->GetRelativePose().pos );
  this->rpyP->SetValue( this->GetRelativePose().rot );

  stream << prefix << "<geom name=\"" 
         << this->nameP->GetValue() << "\">\n";

  stream << prefix << "  " << *(this->xyzP) << "\n";
  stream << prefix << "  " << *(this->rpyP) << "\n";

  this->shape->Save(p,stream);

  stream << prefix << "  " << *(this->massP) << "\n";

  stream << prefix << "  " << *(this->laserFiducialIdP) << "\n";
  stream << prefix << "  " << *(this->laserRetroP) << "\n";

  stream << prefix << "</geom>\n";
}

////////////////////////////////////////////////////////////////////////////////
// Set the encapsulated geometry object
void Geom::SetGeom(bool placeable)
{
  this->GetWorld()->GetPhysicsEngine()->LockMutex();

  this->placeable = placeable;

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

  this->GetWorld()->GetPhysicsEngine()->UnlockMutex();
}

////////////////////////////////////////////////////////////////////////////////
// Return whether this is a placeable geom.
bool Geom::IsPlaceable() const
{
  return this->placeable;
}

////////////////////////////////////////////////////////////////////////////////
/// Set the laser fiducial integer id
void Geom::SetLaserFiducialId(int id)
{
  this->laserFiducialIdP->SetValue( id );
}

////////////////////////////////////////////////////////////////////////////////
/// Get the laser fiducial integer id
int Geom::GetLaserFiducialId() const
{
  return this->laserFiducialIdP->GetValue();
}

////////////////////////////////////////////////////////////////////////////////
/// Set the laser retro reflectiveness
void Geom::SetLaserRetro(float retro)
{
  this->laserRetroP->SetValue(retro);
}

////////////////////////////////////////////////////////////////////////////////
/// Get the laser retro reflectiveness
float Geom::GetLaserRetro() const
{
  return this->laserRetroP->GetValue();
}


////////////////////////////////////////////////////////////////////////////////
/// Set the visibility of the Bounding box of this geometry
void Geom::ShowBoundingBox(const bool &show)
{
  if (!this->bbVisual.empty())
  {
    msgs::Visual msg;
    common::Message::Init(msg, this->bbVisual);
    msg.set_visible( show );
    msg.set_action( msgs::Visual::UPDATE );
    this->vis_pub->Publish(msg);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Set the mass
void Geom::SetMass(const Mass &_mass)
{
  this->mass = _mass;
  //this->body->UpdateCoM();
}

////////////////////////////////////////////////////////////////////////////////
/// Set the mass
void Geom::SetMass(const double &_mass)
{
  this->mass.SetMass( _mass );
  //this->body->UpdateCoM();
}

////////////////////////////////////////////////////////////////////////////////
/// Get the body this geom belongs to
Body *Geom::GetBody() const
{
  return this->body;
}

////////////////////////////////////////////////////////////////////////////////
/// Get the model this geom belongs to
Model *Geom::GetModel() const
{
  return this->body->GetModel();
}

////////////////////////////////////////////////////////////////////////////////
/// Set the friction mode of the geom
void Geom::SetFrictionMode( const bool &v )
{
  this->surface->enableFriction = v;
}


////////////////////////////////////////////////////////////////////////////////
/// Get a pointer to the mass
const Mass &Geom::GetMass() const
{
  return this->mass;
}

////////////////////////////////////////////////////////////////////////////////
// Get the shape type
EntityType Geom::GetShapeType()
{
  return this->shape->GetLeafType();
}

////////////////////////////////////////////////////////////////////////////////
/// Set the shape for this geom
void Geom::SetShape(Shape *shape)
{
  this->shape = shape;
}

////////////////////////////////////////////////////////////////////////////////
/// Get the attached shape
Shape *Geom::GetShape() const
{
  return this->shape;
}

////////////////////////////////////////////////////////////////////////////////
// Turn contact recording on or off
void Geom::SetContactsEnabled(const bool &enable)
{
  this->contactsEnabled = enable;
}

////////////////////////////////////////////////////////////////////////////////
// Return true of contact recording is on
bool Geom::GetContactsEnabled() const
{
  return this->contactsEnabled;
}

////////////////////////////////////////////////////////////////////////////////
/// Get the number of contacts
unsigned int Geom::GetContactCount() const
{
  return this->GetParentModel()->GetContactCount(this);
}

////////////////////////////////////////////////////////////////////////////////
/// Add an occurance of a contact to this geom
void Geom::AddContact(const Contact &contact)
{
  if (!this->GetContactsEnabled() || this->GetShapeType() == RAY_SHAPE || this->GetShapeType() == PLANE_SHAPE)
    return;

  this->GetParentModel()->StoreContact(this, contact);
  this->contactSignal( contact );
}           

////////////////////////////////////////////////////////////////////////////////
/// Get a specific contact
Contact Geom::GetContact(unsigned int i) const
{
  return this->GetParentModel()->RetrieveContact(this, i);
}

////////////////////////////////////////////////////////////////////////////////
/// Enable callback: Called when the body changes
void Geom::EnabledCB(bool enabled)
{
  msgs::Visual msg;
  common::Message::Init(msg, this->bbVisual);

  if (enabled)
    msg.set_material( "Gazebo/GreenTransparent" );
  else
    msg.set_material( "Gazebo/RedTransparent" );

  this->vis_pub->Publish(msg);
}

////////////////////////////////////////////////////////////////////////////////
/// Get the linear velocity of the geom
common::Vector3 Geom::GetRelativeLinearVel() const
{
  if (this->body)
    return this->body->GetRelativeLinearVel();
  else
    return common::Vector3();
}

////////////////////////////////////////////////////////////////////////////////
/// Get the linear velocity of the geom in the world frame
common::Vector3 Geom::GetWorldLinearVel() const
{
  if (this->body)
    return this->body->GetWorldLinearVel();
  else
    return common::Vector3();
}

////////////////////////////////////////////////////////////////////////////////
/// Get the angular velocity of the geom
common::Vector3 Geom::GetRelativeAngularVel() const
{
  if (this->body)
    return this->body->GetRelativeAngularVel();
  else
    return common::Vector3();
}

////////////////////////////////////////////////////////////////////////////////
/// Get the angular velocity of the geom in the world frame
common::Vector3 Geom::GetWorldAngularVel() const
{
  if (this->body)
    return this->body->GetWorldAngularVel();
  else
    return common::Vector3();
}

////////////////////////////////////////////////////////////////////////////////
/// Get the linear acceleration of the geom
common::Vector3 Geom::GetRelativeLinearAccel() const
{
  if (this->body)
    return this->body->GetRelativeLinearAccel();
  else
    return common::Vector3();
}

////////////////////////////////////////////////////////////////////////////////
/// Get the linear acceleration of the geom in the world frame
common::Vector3 Geom::GetWorldLinearAccel() const
{
  if (this->body)
    return this->body->GetWorldLinearAccel();
  else
    return common::Vector3();
}

////////////////////////////////////////////////////////////////////////////////
/// Get the angular acceleration of the geom
common::Vector3 Geom::GetRelativeAngularAccel() const
{
  if (this->body)
    return this->body->GetRelativeAngularAccel();
  else
    return common::Vector3();
}

////////////////////////////////////////////////////////////////////////////////
/// Get the angular acceleration of the geom in the world frame
common::Vector3 Geom::GetWorldAngularAccel() const
{
  if (this->body)
    return this->body->GetWorldAngularAccel();
  else
    return common::Vector3();
}
