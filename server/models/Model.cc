#include <boost/python.hpp>
#include <Ogre.h>
#include "OgreAdaptor.hh"
#include "XMLConfig.hh"
#include "World.hh"
#include "Body.hh"
#include "HingeJoint.hh"
#include "Hinge2Joint.hh"
#include "BallJoint.hh"
#include "UniversalJoint.hh"
#include "PhysicsEngine.hh"
#include "Model.hh"


////////////////////////////////////////////////////////////////////////////////
// Constructor
Model::Model()
  : Entity()
{
  this->name = "";
  this->type = "";

  this->pName = NULL;
  this->pModule = NULL;
  this->pFuncUpdate = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor
Model::~Model()
{
  this->bodies.clear();
}

////////////////////////////////////////////////////////////////////////////////
// Load the model
int Model::Load(XMLConfigNode *node)
{

  if (node->GetName() == "xml")
  {
    XMLConfigNode *bodyNode = NULL;
    XMLConfigNode *jointNode = NULL;
    XMLConfigNode *ifaceNode = NULL;
    Body *body;

    this->SetName(node->GetString("name","",1));
    this->SetStatic(node->GetBool("static",false,0));

    // Load the bodies
    bodyNode = node->GetChildByNSPrefix("body");

    while (bodyNode)
    {
      if (this->LoadBody(bodyNode) != 0)
        std::cerr << "Error Loading body[" << bodyNode->GetName() << "]\n";

      bodyNode = bodyNode->GetNextByNSPrefix("body");
    }

    // Load the joints
    jointNode = node->GetChildByNSPrefix("joint");
    while (jointNode)
    {
      if (this->LoadJoint(jointNode) != 0)
        std::cerr << "Error Loading Joint[" << jointNode->GetName() << "]\n";

      jointNode = jointNode->GetNextByNSPrefix("joint");
    }

    ifaceNode = node->GetChildByNSPrefix("interface");
    while (ifaceNode)
    {
      if (this->LoadIface(ifaceNode) != 0)
        std::cerr << "Error Loading Interface[" << ifaceNode->GetName() << "]\n";
      ifaceNode = ifaceNode->GetNextByNSPrefix("interface");
    }
  }

  // Get the name of the python module
  this->pName = PyString_FromString(node->GetString("python","",0).c_str());
  //this->pName = PyString_FromString("pioneer2dx");

  // Import the python module
  if (this->pName)
  {
    printf("Load pModule\n");
    this->pModule = PyImport_Import(this->pName);
    Py_DECREF(this->pName);
  }

  // Get the Update function from the module
  if (this->pModule != NULL) 
  {
    printf("Load pFuncUpdate\n");
    this->pFuncUpdate = PyObject_GetAttrString(this->pModule, "Update");
    if (this->pFuncUpdate && !PyCallable_Check(this->pFuncUpdate))
      this->pFuncUpdate = NULL;
  }

  return this->LoadChild(node);
}

////////////////////////////////////////////////////////////////////////////////
// Initialize the model
int Model::Init()
{
  return this->InitChild();
}

////////////////////////////////////////////////////////////////////////////////
// Update the model
int Model::Update()
{
  std::map<std::string, Body*>::iterator iter;

  for (iter=this->bodies.begin(); iter!=this->bodies.end(); iter++)
  {
    iter->second->Update();
  }

  // Call the model's python update function, if one exists
  if (this->pFuncUpdate)
  {
    printf("Python Update\n");
    boost::python::call<void>(this->pFuncUpdate, this);
  }

  return this->UpdateChild();
}

////////////////////////////////////////////////////////////////////////////////
// Finalize the model
int Model::Fini()
{
  return this->FiniChild();
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
// Set the Name.
void Model::SetName(const std::string &name)
{
  this->name = name;
}

////////////////////////////////////////////////////////////////////////////////
// Get the Name
const std::string &Model::GetName() const
{
  return this->name;
}

////////////////////////////////////////////////////////////////////////////////
// Set the initial pose
void Model::SetInitPose(const Pose3d &pose)
{
  this->pose = pose;
}

////////////////////////////////////////////////////////////////////////////////
// Get the initial pose
const Pose3d &Model::GetInitPose() const
{
  return this->pose;
}

////////////////////////////////////////////////////////////////////////////////
// Set the current pose
void Model::SetPose(const Pose3d &pose)
{
  Body *body;
  std::map<std::string, Body*>::iterator iter;

  Pose3d origPose, newPose, bodyPose;

  this->pose = pose;

  origPose = this->GetPose();
  newPose = pose;

  for (iter=this->bodies.begin(); iter!=this->bodies.end(); iter++)
  {
    body = iter->second;

    bodyPose = body->GetPose();

    // Compute the pose relative to the model
    bodyPose = bodyPose - origPose;

    // Compute the new pose
    bodyPose += newPose;

    body->SetPose(bodyPose);

    //body->SetLinearVel(Vector3(0,0,0));
    //body->SetAngularVel(Vector3(0,0,0));
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
  Body *newBody = World::Instance()->GetPhysicsEngine()->CreateBody(this);

  return newBody;
}

////////////////////////////////////////////////////////////////////////////////
// Create and return a new joint
Joint *Model::CreateJoint(Joint::Type type)
{
  return World::Instance()->GetPhysicsEngine()->CreateJoint(type);
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

  Joint *joint = NULL;

  Body *body1 = this->bodies[node->GetString("body1","",1)];
  Body *body2 = this->bodies[node->GetString("body2","",1)];
  Body *anchorBody = this->bodies[node->GetString("anchor","",1)];
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
    joint->SetAnchor(anchorBody->GetPosition());
  else
    joint->SetAnchor(anchorVec);

  // Set the axis of the hing joint
  if (node->GetName() == "hinge")
  {
    ((HingeJoint*)joint)->SetAxis(node->GetVector3("axis",Vector3(0,0,1)));
  }
  else if (node->GetName() == "hinge2")
  {
    ((Hinge2Joint*)joint)->SetAxis1(node->GetVector3("axis1",Vector3(0,0,1)));
    ((Hinge2Joint*)joint)->SetAxis2(node->GetVector3("axis2",Vector3(0,0,1)));
  }
  else if (node->GetName() == "universal")
  {
    ((UniversalJoint*)joint)->SetAxis1(node->GetVector3("axis1",Vector3(0,0,1)));
    ((UniversalJoint*)joint)->SetAxis2(node->GetVector3("axis2",Vector3(0,0,1)));
  }

  // Set joint parameters
  joint->SetParam(dParamSuspensionERP, node->GetDouble("erp",0.0,0));
  joint->SetParam(dParamSuspensionCFM, node->GetDouble("cfm",0.0,0));

  this->joints.push_back(joint);

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Load a new interface helper function
int Model::LoadIface(XMLConfigNode *node)
{
  if (!node)
    return -1;

  Iface *iface = NULL;

  if (node->GetName() == "position2d")
  {
    /*iface = new PositionIface();
     if (iface->Create(World::Instance()->GetGzServer(),
           this->GetName().c_str(), "Pioneer2DX", this->GetId(), 
           this->GetParentId()) != 0)
       return -1;
       */
  }
  else if (node->GetName() == "power")
  {
  }
  else if (node->GetName() == "sonar")
  {
  }

  if (iface)
    this->ifaces.push_back(iface);

  return 0;
}
