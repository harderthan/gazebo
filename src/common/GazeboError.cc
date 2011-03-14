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
/*
 * Desc: Gazebo Error
 * Author: Nathan Koenig
 * Date: 07 May 2007
 * SVN info: $Id$
 */

#include "common/GazeboError.hh"

using namespace gazebo;
using namespace common;

using namespace std;


////////////////////////////////////////////////////////////////////////////////
/// Default constructor
GazeboError::GazeboError(const char *file, int line, std::string msg)
{
  this->file = file;
  this->line = line;
  this->str = msg;
  std::cerr << *this << "\n";
}

////////////////////////////////////////////////////////////////////////////////
/// Destructor
GazeboError::~GazeboError()
{
}

////////////////////////////////////////////////////////////////////////////////
/// Return the error file
std::string GazeboError::GetErrorFile() const
{
  return this->file;
}

////////////////////////////////////////////////////////////////////////////////
/// Return the error string
std::string GazeboError::GetErrorStr() const
{
  return this->str;
}

////////////////////////////////////////////////////////////////////////////////
/// Return the error code
int GazeboError::GetErrorLine() const
{
  return this->line;
}

