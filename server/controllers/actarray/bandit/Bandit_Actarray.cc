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
/*
 * Desc: Actuator array controller for a Bandit robot.
 * Author: Nathan Koenig
 * Date: 19 Sep 2007
 * SVN info: $Id$
 */

#include "Global.hh"
#include "XMLConfig.hh"
#include "Model.hh"
#include "HingeJoint.hh"
#include "World.hh"
#include "gazebo.h"
#include "GazeboError.hh"
#include "ControllerFactory.hh"
#include "Bandit_Actarray.hh"

using namespace gazebo;

GZ_REGISTER_STATIC_CONTROLLER("bandit_actarray", Bandit_Actarray);

////////////////////////////////////////////////////////////////////////////////
// Constructor
Bandit_Actarray::Bandit_Actarray(Entity *parent )
  : Controller(parent)
{
  this->myParent = dynamic_cast<Model*>(this->parent);

  if (!this->myParent)
    gzthrow("Bandit_Actarray controller requires a Model as its parent");

}

////////////////////////////////////////////////////////////////////////////////
// Destructor
Bandit_Actarray::~Bandit_Actarray()
{
}

////////////////////////////////////////////////////////////////////////////////
// Load the controller
void Bandit_Actarray::LoadChild(XMLConfigNode *node)
{
  this->myIface = dynamic_cast<ActarrayIface*>(this->ifaces[0]);

  if (!this->myIface)
    gzthrow("Bandit_Actarray controller requires a Actarray Iface");

  std::string headNeckJointName = node->GetString("head_neck_joint", "", 1);

  this->joints[0] = dynamic_cast<HingeJoint*>(this->myParent->GetJoint(headNeckJointName));

  if (!this->joints[0])
    gzthrow("couldn't get head->neck hinge joint");


}

////////////////////////////////////////////////////////////////////////////////
// Initialize the controller
void Bandit_Actarray::InitChild()
{
}

////////////////////////////////////////////////////////////////////////////////
// Update the controller
void Bandit_Actarray::UpdateChild(UpdateParams &params)
{
  this->joints[0]->SetParam( dParamVel, -0.1);
  this->joints[0]->SetParam( dParamFMax, 10.1 );
}

////////////////////////////////////////////////////////////////////////////////
// Finalize the controller
void Bandit_Actarray::FiniChild()
{
}
