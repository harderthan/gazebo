/*
 * Copyright 2011 Nate Koenig
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
/* Desc: A timer class
 * Author: Nate Koenig
 * Date: 22 Nov 2009
 */

#ifndef TIMER_HH
#define TIMER_HH

#include "common/Console.hh"
#include "common/Time.hh"

namespace gazebo
{
  namespace common
  {
    /// \addtogroup gazebo_common
    /// \{

    /// \brief A timer class, used to time things in real world walltime
    class Timer
    {
      /// \brief Constructor
      public: Timer();

      /// \brief Destructor
      public: virtual ~Timer();

      /// \brief Start the timer
      public: void Start();

      /// \brief Get the elapsed time
      public: Time GetElapsed() const;

      /// \brief stream operator friendly
      public: friend std::ostream &operator<<(std::ostream &out,
                                              const gazebo::common::Timer &t)
              {
                out << t.GetElapsed();
                return out;
              }

      private: Time start; /// \brief the time of the last call to Start
    };
    /// \}
  }
}
#endif


