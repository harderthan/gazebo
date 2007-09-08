/*
 *  Gazebo - Outdoor Multi-Robot Simulator
 *  Copyright (C) 2003  
 *     Nate Koenig & Andrew Howard
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/* Desc: Base class for all models.
 * Author: Nathan Koenig and Andrew Howard
 * Date: 8 May 2003
 * SVN: $Id: Model.cc 75 2007-07-25 15:47:11Z natepak $
 */

//#include <boost/python.hpp>

#include <sstream>
#include <iostream>
#include <Ogre.h>

#include "Global.hh"
#include "GazeboError.hh"
#include "GazeboMessage.hh"
#include "OgreAdaptor.hh"
#include "XMLConfig.hh"
#include "World.hh"
#include "Body.hh"
#include "HingeJoint.hh"
#include "Hinge2Joint.hh"
#include "BallJoint.hh"
#include "UniversalJoint.hh"
#include "SliderJoint.hh"
#include "PhysicsEngine.hh"
#include "Controller.hh"
#include "ControllerFactory.hh"
#include "IfaceFactory.hh"
#include "Model.hh"

using namespace gazebo;

////////////////////////////////////////////////////////////////////////////////
// Constructor
Model::Model()
  : Entity()
{
  this->type = "";
  this->joint = NULL;
  this->parentModel = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor
Model::~Model()
{
  std::map< std::string, Body* >::iterator biter;
  std::map< std::string, Joint* >::iterator jiter;
  std::map< std::string, Controller* >::iterator citer;

  for (biter=this->bodies.begin(); biter != this->bodies.end(); biter++)
  {
    delete biter->second;
  }
  this->bodies.clear();

  for (jiter = this->joints.begin(); jiter != this->joints.end(); jiter++)
  {
    delete jiter->second;
  }
  this->joints.clear();

  for (citer = this->controllers.begin(); 
       citer != this->controllers.end(); citer++)
  {
    delete citer->second;
  }
  this->controllers.clear();
}

////////////////////////////////////////////////////////////////////////////////
// Load the model
int Model::Load(XMLConfigNode *node)
{
  XMLConfigNode *childNode;

  this->SetName(node->GetString("name","",1));
  this->SetStatic(node->GetBool("static",false,0));

  if (this->GetType() == "physical")
  {
    this->LoadPhysical(node);
  }
  else if (this->GetType() == "renderable")
  {
    this->LoadRenderable(node);
  }
  else if (this->GetType() != "empty")
  {
    std::ostringstream stream;
    stream << "Invalid model type[" << this->GetType() << "]\n";
    gzthrow(stream.str());
  }

  // Load controllers
  childNode = node->GetChildByNSPrefix("controller");
  while (childNode)
  {
    this->LoadController(childNode);
    childNode = childNode->GetNextByNSPrefix("controller");
  }

  // Create a default body if one does not exist in the XML file
  if (this->bodies.size() <= 0)
  {
    std::ostringstream bodyName;

    bodyName << this->GetName() << "_body";

    // Create an empty body for the model
    Body *body = this->CreateBody();
    body->SetName(bodyName.str());

    // Store the pointer to this body
    this->bodies[body->GetName()] = body;

    this->canonicalBodyName = bodyName.str();
  }


  if (this->canonicalBodyName.empty())
  {
    this->canonicalBodyName = this->bodies.begin()->first;
  }

  // Get the name of the python module
  /*this->pName.reset(PyString_FromString(node->GetString("python","",0).c_str()));
  //this->pName.reset(PyString_FromString("pioneer2dx"));

  // Import the python module
  if (this->pName)
  {
    this->pModule.reset(PyImport_Import(this->pName));
    Py_DECREF(this->pName);
  }

  // Get the Update function from the module
  if (this->pModule) 
  {
    this->pFuncUpdate.reset(PyObject_GetAttrString(this->pModule, "Update"));
    if (this->pFuncUpdate && !PyCallable_Check(this->pFuncUpdate))
      this->pFuncUpdate = NULL;
  }
  */

  return this->LoadChild(node);
}

////////////////////////////////////////////////////////////////////////////////
// Initialize the model
int Model::Init()
{
  std::map<std::string, Body* >::iterator biter;
  std::map<std::string, Controller* >::iterator contIter;

  for (biter = this->bodies.begin(); biter!=this->bodies.end(); biter++)
    biter->second->Init();

  for (contIter=this->controllers.begin(); 
       contIter!=this->controllers.end(); contIter++)
  {
      contIter->second->Init();
  }

  /*this->mtext = new MovableText(this->GetName(), "this is the caption");
  this->mtext->setTextAlignment(MovableText::H_CENTER, MovableText::V_ABOVE);
  this->mtext->setAdditionalHeight(0.5);
  this->sceneNode->attachObject(this->mtext);
  */

  return this->InitChild();
}

////////////////////////////////////////////////////////////////////////////////
// Update the model
int Model::Update(UpdateParams &params)
{
  std::map<std::string, Body* >::iterator bodyIter;
  std::map<std::string, Controller* >::iterator contIter;

  Pose3d bodyPose, newPose, oldPose;

  for (bodyIter=this->bodies.begin(); bodyIter!=this->bodies.end(); bodyIter++)
  {
    if (bodyIter->second)
    {
      bodyIter->second->Update(params);
    }
  }

  for (contIter=this->controllers.begin(); 
       contIter!=this->controllers.end(); contIter++)
  {
    if (contIter->second)
      contIter->second->Update(params);
  }

  // Call the model's python update function, if one exists
  /*if (this->pFuncUpdate)
  {
    boost::python::call<void>(this->pFuncUpdate, this);
  }*/
 
  Body *b = NULL;
  if (!this->canonicalBodyName.empty())
  {
    this->pose = this->bodies[this->canonicalBodyName]->GetPose();
    b = this->bodies[this->canonicalBodyName];
  }

  /*if (this->GetName() == "light2")
  {
    Ogre::Vector3 ppos = this->sceneNode->getPosition();
    ppos.z += 0.1;
    ppos.y = 1.0;
    this->sceneNode->setPosition(ppos);
  }*/

  return this->UpdateChild();
}

////////////////////////////////////////////////////////////////////////////////
// Finalize the model
int Model::Fini()
{
  std::map<std::string, Body* >::iterator bodyIter;
  std::map<std::string, Iface* >::iterator ifaceIter;
  std::map<std::string, Controller* >::iterator contIter;

  for (contIter = this->controllers.begin(); 
       contIter != this->controllers.end(); contIter++)
  {
    contIter->second->Fini();
  }

  return this->FiniChild();
}

////////////////////////////////////////////////////////////////////////////////
// Reset the model
void Model::Reset()
{
  if (this->initPose.pos != this->pose.pos)
  {
    this->SetPose(this->initPose);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Set the name of the model
void Model::SetType(const std::string &type)
{
  this->type = type;
}

////////////////////////////////////////////////////////////////////////////////
// Get the name of the model
const std::string &Model::GetType() const
{
  return this->type;
}

////////////////////////////////////////////////////////////////////////////////
// Set the XMLConfig node this model was loaded from
void Model::SetXMLConfigNode( XMLConfigNode *node )
{
  this->node = node;
}

////////////////////////////////////////////////////////////////////////////////
// Get the XML Conig node this model was loaded from
XMLConfigNode *Model::GetXMLConfigNode() const
{
  return this->node;
}

////////////////////////////////////////////////////////////////////////////////
// Set the initial pose
void Model::SetInitPose(const Pose3d &pose)
{
  this->initPose = pose;
}

////////////////////////////////////////////////////////////////////////////////
// Get the initial pose
const Pose3d &Model::GetInitPose() const
{
  return this->initPose;
}

////////////////////////////////////////////////////////////////////////////////
// Set the current pose
void Model::SetPose(const Pose3d &setPose)
{
  Body *body;
  std::map<std::string, Body* >::iterator iter;

  Pose3d bodyPose, origPose;

  origPose = this->pose;
  this->pose = setPose;

  for (iter=this->bodies.begin(); iter!=this->bodies.end(); iter++)
  {
    if (!iter->second)
      continue;

    body = iter->second;

    // Compute the pose relative to the model
    bodyPose = body->GetPose() - origPose;

    // Compute the new pose
    bodyPose += this->pose;

    body->SetPose(bodyPose);
  }

}

////////////////////////////////////////////////////////////////////////////////
// Get the current pose
const Pose3d &Model::GetPose() const
{
  return this->pose;
}

////////////////////////////////////////////////////////////////////////////////
// Create and return a new body
Body *Model::CreateBody()
{
  // Create a new body
  return World::Instance()->GetPhysicsEngine()->CreateBody(this); 
}

////////////////////////////////////////////////////////////////////////////////
// Create and return a new joint
Joint *Model::CreateJoint(Joint::Type type)
{
  return World::Instance()->GetPhysicsEngine()->CreateJoint(type);
}

////////////////////////////////////////////////////////////////////////////////
Joint *Model::GetJoint(std::string name)
{
  std::map<std::string, Joint* >::const_iterator iter;
  iter = this->joints.find(name);

  if (iter != this->joints.end())
    return iter->second;
  else
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Load a new body helper function
int Model::LoadBody(XMLConfigNode *node)
{
  if (!node)
    return -1;

  // Create a new body
  Body *body = this->CreateBody();

  // Load the body using the config node. This also loads all of the 
  // bodies geometries
  body->Load(node);

  // Store this body
  if (this->bodies[body->GetName()])
    std::cerr << "Body with name[" << body->GetName() << "] already exists!!\n";

  // Store the pointer to this body
  this->bodies[body->GetName()] = body;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Load a new joint helper function
int Model::LoadJoint(XMLConfigNode *node)
{
  if (!node)
    return -1;

  Joint *joint;

  Body *body1(this->bodies[node->GetString("body1","",1)]);
  Body *body2(this->bodies[node->GetString("body2","",1)]);
  Body *anchorBody(this->bodies[node->GetString("anchor","",0)]);
  Vector3 anchorVec = node->GetVector3("anchor",Vector3(0,0,0));

  if (!body1)
  {
    std::cerr << "Couldn't Find Body[" << node->GetString("body1","",1);
    return -1;
  }
  if (!body2)
  {
    std::cerr << "Couldn't Find Body[" << node->GetString("body2","",1);
    return -1;
  }

  // Create a Hinge Joint
  if (node->GetName() == "hinge")
    joint = this->CreateJoint(Joint::HINGE);
  else if (node->GetName() == "ball")
    joint = this->CreateJoint(Joint::BALL);
  else if (node->GetName() == "slider")
    joint = this->CreateJoint(Joint::SLIDER);
  else if (node->GetName() == "hinge2")
    joint = this->CreateJoint(Joint::HINGE2);
  else if (node->GetName() == "universal")
    joint = this->CreateJoint(Joint::UNIVERSAL);
  else
  {
    std::cerr << "Uknown joint[" << node->GetName() << "]\n";
    return -1;
  }
 
  // Attach two bodies 
  joint->Attach(body1,body2);

  // Set the anchor vector
  if (anchorBody)
  {
    joint->SetAnchor(anchorBody->GetPosition());
  }
  else
  {
    joint->SetAnchor(anchorVec);
    this->bodies.erase(node->GetString("anchor","",0));
  }

  // Set the axis of the hing joint
  if (node->GetName() == "hinge")
  {
    HingeJoint *hinge = (HingeJoint*)(joint);
    hinge->Load(node);
  }
  else if (node->GetName() == "hinge2")
  {
    Hinge2Joint *hinge = (Hinge2Joint*)(joint);
    hinge->Load(node);
  }
  else if (node->GetName() == "universal")
  {
    UniversalJoint *uni = (UniversalJoint*)(joint);

    uni->SetAxis1(node->GetVector3("axis1",Vector3(0,0,1)));
    uni->SetAxis2(node->GetVector3("axis2",Vector3(0,0,1)));
  }
  else if (node->GetName() == "slider")
  {
    SliderJoint *slider = (SliderJoint*)(joint);

    double lowStop = node->GetDouble("lowStop",0,0);
    double hiStop = node->GetDouble("hiStop",0,0);

    slider->SetParam(dParamLoStop, lowStop);
    slider->SetParam(dParamHiStop, hiStop);
  }

  // Set joint parameters
  joint->SetParam(dParamSuspensionERP, node->GetDouble("erp",0.4,0));
  joint->SetParam(dParamSuspensionCFM, node->GetDouble("cfm",0.8,0));

  // Name the joint
  joint->SetName(node->GetString("name","",1));

  if (this->joints.find(joint->GetName()) == this->joints.end())
    gzthrow( "can't have two joint with the same name");

  this->joints[joint->GetName()] = joint;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
/// Load a controller helper function
void Model::LoadController(XMLConfigNode *node)
{
  if (!node)
    gzthrow( "node parameter is NULL" );

  Controller *controller;
  std::ostringstream stream;

  // Get the controller's type
  std::string controllerType = node->GetName();

  // Get the unique name of the controller
  std::string controllerName = node->GetString("name","",1);

  // Create the controller based on it's type
  controller = ControllerFactory::NewController(controllerType, this);

  if (controller)
  {
    // Load the controller
    try
    {
      controller->Load(node);
    }
    catch (GazeboError e)
    {
      std::cerr << "Error Loading Controller[" <<  controllerName
        << "]\n" << e << std::endl;
      delete controller;
      return;
    }

    // Store the controller
    this->controllers[controllerName] = controller;
  }
  else
  {
    gzmsg(0) << "Unknown controller[" << controllerType << "]\n";
  }
}

////////////////////////////////////////////////////////////////////////////////
// Return the default body
Body *Model::GetBody()
{
  return this->bodies.begin()->second;
}

////////////////////////////////////////////////////////////////////////////////
// Attach this model to its parent
void Model::Attach()
{
  this->parentModel = dynamic_cast<Model*>(this->parent);

  if (this->parentModel == NULL)
    gzthrow("Parent cannot be NULL when attaching two models");

  this->joint = (HingeJoint*)this->CreateJoint(Joint::HINGE);

  Body *myBody = this->bodies[canonicalBodyName];
  Body *pBody = this->parentModel->GetCanonicalBody();

  if (myBody == NULL)
    gzthrow("No canonical body set.");

  if (pBody == NULL)
    gzthrow("Parent has no canonical body");


  this->joint->Attach(myBody, pBody);
  this->joint->SetAnchor( myBody->GetPosition() );
  this->joint->SetAxis( Vector3(0,1,0) );
  this->joint->SetParam( dParamHiStop, 0);
  this->joint->SetParam( dParamLoStop, 0);
}

////////////////////////////////////////////////////////////////////////////////
/// Get the canonical body. Used for connected Model heirarchies
Body *Model::GetCanonicalBody()
{
  return this->bodies[this->canonicalBodyName];
}

////////////////////////////////////////////////////////////////////////////////
// Load a renderable model (like a light source).
void Model::LoadRenderable(XMLConfigNode *node)
{
  XMLConfigNode *childNode = NULL;

  // We still need a canonical body so that this model can be attached to
  // others
  Body *body = this->CreateBody();
  body->SetName(this->GetName() + "_RenderableBody");
  body->SetGravityMode(false);
  body->SetPose(Pose3d());
  this->bodies[body->GetName()] = body;

  if ((childNode = node->GetChild("light")))
  {
    OgreAdaptor::Instance()->CreateLight(childNode, body);
  }

}

////////////////////////////////////////////////////////////////////////////////
// Load a physical model
void Model::LoadPhysical(XMLConfigNode *node)
{
  XMLConfigNode *childNode = NULL;

  // Load the bodies
  childNode = node->GetChildByNSPrefix("body");

  while (childNode)
  {
    if (this->LoadBody(childNode) != 0)
      std::cerr << "Error Loading body[" << childNode->GetName() << "]\n";

    childNode = childNode->GetNextByNSPrefix("body");
  }

  // Load the joints
  childNode = node->GetChildByNSPrefix("joint");

  while (childNode)
  {
    if (this->LoadJoint(childNode) != 0)
      std::cerr << "Error Loading Joint[" << childNode->GetName() << "]\n";

    childNode = childNode->GetNextByNSPrefix("joint");
  }

  this->canonicalBodyName = node->GetString("canonicalBody","",0);
}
