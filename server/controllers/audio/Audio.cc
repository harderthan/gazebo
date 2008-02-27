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
 * Desc: Audio controller
 * Author: Jordi Polo
 * Date: 23 Feb 2008
 */

#include "Global.hh"
#include "XMLConfig.hh"
#include "Model.hh"
#include "Simulator.hh"
#include "gazebo.h"
#include "GazeboError.hh"
#include "ControllerFactory.hh"
#include "Audio.hh"
#include "OgreVisual.hh"
#include <string.h>
#include <OgreAL.h>

using namespace gazebo;

GZ_REGISTER_STATIC_CONTROLLER("audio", AudioController);

enum {DRIVE, STEER, FULL};

////////////////////////////////////////////////////////////////////////////////
// Constructor
AudioController::AudioController(Entity *parent )
  : Controller(parent)
{
  this->soundManager =0;
 // this->myParent = dynamic_cast<Model*>(this->parent);
  
//  if (!this->myParent)
//    gzthrow("AudioController controller requires a Model as its parent");

}

////////////////////////////////////////////////////////////////////////////////
// Destructor
AudioController::~AudioController()
{
  GZ_DELETE(this->soundManager)
}

////////////////////////////////////////////////////////////////////////////////
// Load the controller
void AudioController::LoadChild(XMLConfigNode *node)
{

  this->myIface = dynamic_cast<AudioIface*>(this->ifaces[0]);

  if (!this->myIface)
    gzthrow("Audio controller requires an audio interface");

  this->loopSound= node->GetBool("loop",false,0);
  this->stream=node->GetBool("stream",false,0);

 }

////////////////////////////////////////////////////////////////////////////////
// Initialize the controller
void AudioController::InitChild()
{

    this->soundManager = new OgreAL::SoundManager();
/*  try
  {
  }
  catch()
  {
    gzthrow("The 3d Sound manager can not be initialized, check your OpenAL and OgreAL installation\n");
  }
*/
}

////////////////////////////////////////////////////////////////////////////////
// Reset the controller
void AudioController::ResetChild()
{
}

////////////////////////////////////////////////////////////////////////////////
// Update the controller
void AudioController::UpdateChild(UpdateParams &params)
{

  OgreAL::Sound *sound;
  this->GetAudioCmd();
 
  if (cmdPlay)
  {
    if (!soundManager->hasSound(this->url))
      sound = soundManager->createSound(this->url, this->url, this->loopSound, this->stream);
    else
      sound = soundManager->getSound(this->url);

    this->parent->GetVisualNode()->AttachObject(sound);
    sound->play();
  }
  if (cmdStop)
  {}
  //more stuff here

  this->PutAudioData();
}

////////////////////////////////////////////////////////////////////////////////
// Finalize the controller
void AudioController::FiniChild()
{
}


//////////////////////////////////////////////////////////////////////////////
// Get commands from the external interface
void AudioController::GetAudioCmd()
{

  if (this->myIface->Lock(1))
  {
    
    this->cmdPlay = this->myIface->data->cmd_play;
    this->cmdPause = this->myIface->data->cmd_pause;
    this->cmdStop = this->myIface->data->cmd_stop;
    this->gain = this->myIface->data->gain;
    this->url = ((const char *)this->myIface->data->url);

    this->myIface->Unlock();
  }
  std::cout << this->url << std::endl;
}

//////////////////////////////////////////////////////////////////////////////
// Update the data in the interface
void AudioController::PutAudioData()
{
  if (this->myIface->Lock(1))
  {
    this->myIface->data->time = Simulator::Instance()->GetSimTime();
    this->myIface->data->state = this->state;


    this->myIface->Unlock();
  }
}
