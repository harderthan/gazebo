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

#include <functional>

#include <boost/algorithm/string.hpp>
#include <ignition/math/Pose3.hh>

#include "gazebo/common/Exception.hh"
#include "gazebo/common/Image.hh"

#include "gazebo/physics/World.hh"

#include "gazebo/transport/transport.hh"
#include "gazebo/msgs/msgs.hh"

#include "gazebo/rendering/RenderEngine.hh"
#include "gazebo/rendering/Camera.hh"
#include "gazebo/rendering/Scene.hh"
#include "gazebo/rendering/RenderingIface.hh"

#include "gazebo/sensors/SensorTypes.hh"
#include "gazebo/sensors/Noise.hh"
#include "gazebo/sensors/SensorFactory.hh"
#include "gazebo/sensors/MultiCameraSensorPrivate.hh"
#include "gazebo/sensors/MultiCameraSensor.hh"

using namespace gazebo;
using namespace sensors;

GZ_REGISTER_STATIC_SENSOR("multicamera", MultiCameraSensor)

//////////////////////////////////////////////////
MultiCameraSensor::MultiCameraSensor()
: Sensor(*new MultiCameraSensorPrivate, sensors::IMAGE),
  dataPtr(std::static_pointer_cast<MultiCameraSensorPrivate>(this->dPtr))
{
  this->dataPtr->rendered = false;
  this->dataPtr->connections.push_back(
      event::Events::ConnectRender(
        std::bind(&MultiCameraSensor::Render, this)));
}

//////////////////////////////////////////////////
MultiCameraSensor::~MultiCameraSensor()
{
}

//////////////////////////////////////////////////
std::string MultiCameraSensor::Topic() const
{
  std::string topic = Sensor::Topic();

  // Create a topic name if one has not been specified.
  if (topic.empty())
  {
    topic = "~/";
    topic += this->ParentName() + "/" + this->Name() + "/images";
    boost::replace_all(topic, "::", "/");
  }

  return topic;
}

//////////////////////////////////////////////////
void MultiCameraSensor::Load(const std::string &_worldName)
{
  Sensor::Load(_worldName);

  // Create the publisher of image data.
  this->dataPtr->imagePub =
    this->dataPtr->node->Advertise<msgs::ImagesStamped>(this->Topic(), 50);
}

//////////////////////////////////////////////////
void MultiCameraSensor::Init()
{
  if (rendering::RenderEngine::Instance()->GetRenderPathType() ==
      rendering::RenderEngine::NONE)
  {
    gzerr << "Unable to create MultiCameraSensor. Rendering is disabled.\n";
    return;
  }

  std::string worldName = this->dataPtr->world->GetName();

  if (worldName.empty())
  {
    gzerr << "No world name\n";
    return;
  }

  this->dataPtr->scene = rendering::get_scene(worldName);

  if (!this->dataPtr->scene)
  {
    this->dataPtr->scene = rendering::create_scene(worldName, false, true);

    // This usually means rendering is not available
    if (!this->dataPtr->scene)
    {
      gzerr << "Unable to create MultiCameraSensor.\n";
      return;
    }
  }

  // Each camera has its own noise pointer
  common::EnumIterator<SensorNoiseType> noiseIndex = SENSOR_NOISE_TYPE_BEGIN;

  // Create and initialize all the cameras
  sdf::ElementPtr cameraSdf = this->dataPtr->sdf->GetElement("camera");
  while (cameraSdf)
  {
    rendering::CameraPtr camera = this->dataPtr->scene->CreateCamera(
          cameraSdf->Get<std::string>("name"), false);

    if (!camera)
    {
      gzthrow("Unable to create multicamera sensor[" +
              cameraSdf->Get<std::string>("name"));
      return;
    }

    camera->SetCaptureData(true);
    camera->Load(cameraSdf);

    // Do some sanity checks
    if (camera->GetImageWidth() == 0 || camera->GetImageHeight() == 0)
      gzthrow("Image has zero size");

    camera->Init();
    camera->CreateRenderTexture(camera->GetName() + "_RttTex");

    ignition::math::Pose3d cameraPose = this->dataPtr->pose;
    if (cameraSdf->HasElement("pose"))
      cameraPose = cameraSdf->Get<ignition::math::Pose3d>("pose") + cameraPose;
    camera->SetWorldPose(cameraPose);
    camera->AttachToVisual(this->dataPtr->parentId, true);

    if (cameraSdf->HasElement("noise"))
    {
      // Create a noise model and attach the camera
      this->dataPtr->noises[*noiseIndex] = NoiseFactory::NewNoiseModel(
        cameraSdf->GetElement("noise"), this->Type());
      this->dataPtr->noises[*noiseIndex]->SetCamera(camera);
    }
    else
    {
      this->dataPtr->noises[*noiseIndex].reset(
          new sensors::Noise(sensors::Noise::NONE));
    }

    // Increment the noise index -- one for each camera in the setup
    ++noiseIndex;

    {
      std::lock_guard<std::mutex> lock(this->dataPtr->cameraMutex);
      this->dataPtr->cameras.push_back(camera);
    }

    msgs::Image *image = this->dataPtr->msg.add_image();
    image->set_width(camera->GetImageWidth());
    image->set_height(camera->GetImageHeight());
    image->set_pixel_format(common::Image::ConvertPixelFormat(
          camera->GetImageFormat()));
    image->set_step(camera->GetImageWidth() * camera->GetImageDepth());

    cameraSdf = cameraSdf->GetNextElement("camera");
  }

  // Disable clouds and moon on server side until fixed and also to improve
  // performance
  this->dataPtr->scene->SetSkyXMode(rendering::Scene::GZ_SKYX_ALL &
      ~rendering::Scene::GZ_SKYX_CLOUDS &
      ~rendering::Scene::GZ_SKYX_MOON);

  Sensor::Init();
}

//////////////////////////////////////////////////
void MultiCameraSensor::Fini()
{
  this->dataPtr->imagePub.reset();
  Sensor::Fini();

  std::lock_guard<std::mutex> lock(this->dataPtr->cameraMutex);

  for (std::vector<rendering::CameraPtr>::iterator iter =
      this->dataPtr->cameras.begin();
      iter != this->dataPtr->cameras.end(); ++iter)
  {
    (*iter)->GetScene()->RemoveCamera((*iter)->GetName());
  }
  this->dataPtr->cameras.clear();
  this->dataPtr->scene.reset();
}

//////////////////////////////////////////////////
rendering::CameraPtr MultiCameraSensor::GetCamera(unsigned int _index) const
{
  return this->Camera(_index);
}

//////////////////////////////////////////////////
rendering::CameraPtr MultiCameraSensor::Camera(const unsigned int _index) const
{
  std::lock_guard<std::mutex> lock(this->dataPtr->cameraMutex);

  if (_index < this->dataPtr->cameras.size())
    return this->dataPtr->cameras[_index];
  else
    gzthrow("camera index out of range. Valid range[0.." +
        boost::lexical_cast<std::string>(this->dataPtr->cameras.size()-1));
}

//////////////////////////////////////////////////
void MultiCameraSensor::Render()
{
  if (this->dataPtr->cameras.empty() || !this->IsActive() ||
      !this->NeedsUpdate())
  {
    return;
  }

  // Update all the cameras
  for (auto iter = this->dataPtr->cameras.begin();
      iter != this->dataPtr->cameras.end(); ++iter)
  {
    (*iter)->Render();
  }

  this->dataPtr->rendered = true;
  this->dataPtr->lastMeasurementTime = this->dataPtr->scene->GetSimTime();
}

//////////////////////////////////////////////////
bool MultiCameraSensor::UpdateImpl(const bool /*_force*/)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->cameraMutex);

  if (!this->dataPtr->rendered)
    return false;

  bool publish = this->dataPtr->imagePub->HasConnections();

  msgs::Set(this->dataPtr->msg.mutable_time(),
            this->dataPtr->lastMeasurementTime);

  int index = 0;
  for (auto iter = this->dataPtr->cameras.begin();
       iter != this->dataPtr->cameras.end(); ++iter, ++index)
  {
    (*iter)->PostRender();

    if (publish)
    {
      msgs::Image *image = this->dataPtr->msg.mutable_image(index);
      image->set_data((*iter)->GetImageData(0),
          image->width() * (*iter)->GetImageDepth() * image->height());
    }
  }

  if (publish)
    this->dataPtr->imagePub->Publish(this->dataPtr->msg);

  this->dataPtr->rendered = false;
  return true;
}

//////////////////////////////////////////////////
unsigned int MultiCameraSensor::GetCameraCount() const
{
  return this->CameraCount();
}

//////////////////////////////////////////////////
unsigned int MultiCameraSensor::CameraCount() const
{
  std::lock_guard<std::mutex> lock(this->dataPtr->cameraMutex);
  return this->dataPtr->cameras.size();
}

//////////////////////////////////////////////////
unsigned int MultiCameraSensor::GetImageWidth(unsigned int _index) const
{
  return this->ImageWidth(_index);
}

//////////////////////////////////////////////////
unsigned int MultiCameraSensor::ImageWidth(const unsigned int _index) const
{
  return this->Camera(_index)->GetImageWidth();
}

//////////////////////////////////////////////////
unsigned int MultiCameraSensor::GetImageHeight(unsigned int _index) const
{
  return this->ImageHeight(_index);
}

//////////////////////////////////////////////////
unsigned int MultiCameraSensor::ImageHeight(const unsigned int _index) const
{
  return this->Camera(_index)->GetImageHeight();
}

//////////////////////////////////////////////////
const unsigned char *MultiCameraSensor::GetImageData(unsigned int _index)
{
  return this->ImageData(_index);
}

//////////////////////////////////////////////////
const unsigned char *MultiCameraSensor::ImageData(const unsigned int _index)
{
  return this->Camera(_index)->GetImageData(0);
}

//////////////////////////////////////////////////
bool MultiCameraSensor::SaveFrame(const std::vector<std::string> &_filenames)
{
  this->SetActive(true);

  std::lock_guard<std::mutex> lock(this->dataPtr->cameraMutex);
  if (_filenames.size() != this->dataPtr->cameras.size())
  {
    gzerr << "Filename count[" << _filenames.size() << "] does not match "
          << "camera count[" << this->dataPtr->cameras.size() << "]\n";
    return false;
  }

  bool result = true;

  auto citer = this->dataPtr->cameras.begin();
  for (std::vector<std::string>::const_iterator fiter = _filenames.begin();
       fiter != _filenames.end(); ++fiter, ++citer)
  {
    result &= (*citer)->SaveFrame(*fiter);
  }

  return result;
}

//////////////////////////////////////////////////
bool MultiCameraSensor::IsActive() const
{
  return Sensor::IsActive() ||
    (this->dataPtr->imagePub && this->dataPtr->imagePub->HasConnections());
}
