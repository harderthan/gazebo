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
#include "gazebo/gui/qt.h"
#include "gazebo/gui/GuiIface.hh"

//////////////////////////////////////////////////
int main(int _argc, char **_argv)
{
  Q_INIT_RESOURCE(resources);
  int result = 0;
  if (!gazebo::gui::run(_argc, _argv))
    result = -1;
  return result;
}
