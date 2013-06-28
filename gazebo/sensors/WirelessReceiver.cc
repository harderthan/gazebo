/*
 * Copyright 2013 Open Source Robotics Foundation
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
/* Desc: Wireless receiver
 * Author: Carlos Agüero 
 * Date: 24 June 2013
 */

#include "gazebo/physics/World.hh"
#include "gazebo/physics/Entity.hh"

#include "gazebo/common/Exception.hh"

#include "gazebo/transport/Node.hh"
#include "gazebo/transport/Publisher.hh"

#include "gazebo/msgs/msgs.hh"

#include "gazebo/math/Vector3.hh"

#include "gazebo/sensors/SensorFactory.hh"
#include "gazebo/sensors/SensorManager.hh"
#include "gazebo/sensors/WirelessReceiver.hh"
#include "gazebo/sensors/WirelessTransmitter.hh"

#include <iostream>
using namespace std;

using namespace gazebo;
using namespace sensors;

GZ_REGISTER_STATIC_SENSOR("wirelessReceiver", WirelessReceiver)

/////////////////////////////////////////////////
WirelessReceiver::WirelessReceiver()
  : Sensor(sensors::OTHER)
{
  this->active = false;
}

/////////////////////////////////////////////////
WirelessReceiver::~WirelessReceiver()
{
  // this->link.reset();
}

//////////////////////////////////////////////////                              
std::string WirelessReceiver::GetTopic() const                               
{                                                                               
  std::string topicName = "~/";                                                 
  topicName += this->parentName + "/" + this->GetName() + "/receiver";          
  boost::replace_all(topicName, "::", "/");                                     
                                                                                
  return topicName;                                                             
}

/////////////////////////////////////////////////
void WirelessReceiver::Load(const std::string &_worldName)
{
  Sensor::Load(_worldName);

  this->pub = this->node->Advertise<msgs::WirelessNodes>(this->GetTopic(), 30);
  this->entity = this->world->GetEntity(this->parentName);
}

/////////////////////////////////////////////////
void WirelessReceiver::Fini()
{
  Sensor::Fini();
}

//////////////////////////////////////////////////
void WirelessReceiver::Init()
{
  Sensor::Init();
}

//////////////////////////////////////////////////
void WirelessReceiver::UpdateImpl(bool /*_force*/)
{
  if (this->pub)                                                           
  {                                                                             
    msgs::WirelessNodes msg;

    Sensor_V sensors = SensorManager::Instance()->GetSensors();
    for (Sensor_V::iterator it = sensors.begin(); it != sensors.end(); ++it)
    {
      if ((*it)->GetType() == "wirelessTransmitter")
      {
        std::string id;
        double freq;
        math::Pose pos;

        id = boost::dynamic_pointer_cast<WirelessTransmitter>(*it)->GetESSID();
        freq = boost::dynamic_pointer_cast<WirelessTransmitter>(*it)->GetFreq();
        pos = boost::dynamic_pointer_cast<WirelessTransmitter>(*it)->GetPose();

        msgs::WirelessNode *wireless_node = msg.add_node();
        wireless_node->set_essid(id);
        wireless_node->set_frequency(freq);

        math::Pose my_pos = entity->GetWorldPose();
        double distance = my_pos.pos.Distance(pos.pos);
        
        if (distance > 0.0)
        {
          wireless_node->set_signal_level(1.0 / (distance));
        }
        else
        {
          wireless_node->set_signal_level(0.0);
        }
      }
    }
                                                                                
    this->pub->Publish(msg);                                               
  }
}

