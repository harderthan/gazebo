/*
 * Copyright 2012 Open Source Robotics Foundation
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
#ifndef _GZMODIFY_HH_
#define _GZMODIFY_HH_

#include <string>
#include <boost/program_options.hpp>

namespace gazebo
{
  namespace po = boost::program_options;

  /// \brief Base class for all commands
  class Command
  {
    /// \brief Constructor
    /// \param[in] _name Name of the command.
    /// \param[in] _brief One line command description.
    public: Command(const std::string &_name, const std::string &_brief);

    /// \brief Print help information.
    public: virtual void Help();

    /// \brief Print detailed help.
    public: virtual void HelpDetailed() = 0;

    /// \brief Get the one description.
    /// \return The brief description of the command.
    public: std::string GetBrief() const;

    /// \brief Execute the command.
    /// \param[in] _argc Number of command line arguments.
    /// \param[in] _argv The line arguments.
    public: bool Run(int _argc, char **_argv);

    /// \brief Implementation of Run
    protected: virtual bool RunImpl() = 0;

    /// \brief Initialize transport. 
    protected: bool TransportInit();

    /// \brief Finalized transport. 
    protected: bool TransportFini();

    /// \brief Name of the command.
    protected: std::string name;

    /// \brief One line description of the command.
    protected: std::string brief;

    /// \brief Options that are visible to the user
    protected: po::options_description visibleOptions;

    /// \brief Variable map
    protected: po::variables_map vm;
  };

  /// \brief World command
  class WorldCommand : public Command
  {
    /// \brief Constructor
    public: WorldCommand();

    // Documentation inherited
    public: virtual void HelpDetailed();

    // Documentation inherited
    protected: virtual bool RunImpl();
  };

  /// \brief Physics command
  class PhysicsCommand : public Command
  {
    /// \brief Constructor
    public: PhysicsCommand();

    // Documentation inherited
    public: virtual void HelpDetailed();

    // Documentation inherited
    protected: virtual bool RunImpl();
  };

  /// \brief Model command
  class ModelCommand : public Command
  {
    /// \brief Constructor
    public: ModelCommand();

    // Documentation inherited
    public: virtual void HelpDetailed();

    // Documentation inherited
    protected: virtual bool RunImpl();
  };

  /// \brief Joint command
  class JointCommand : public Command
  {
    /// \brief Constructor
    public: JointCommand();

    // Documentation inherited
    public: virtual void HelpDetailed();

    // Documentation inherited
    protected: virtual bool RunImpl();
  };

  /// \brief Camera command
  class CameraCommand : public Command
  {
    /// \brief Constructor
    public: CameraCommand();

    // Documentation inherited
    public: virtual void HelpDetailed();

    // Documentation inherited
    protected: virtual bool RunImpl();
  };
}
#endif
