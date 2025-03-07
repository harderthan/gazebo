/*
 * Copyright (C) 2014-2015 Open Source Robotics Foundation
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
#ifndef _GAZEBO_SIGNAL_STATS_HH_
#define _GAZEBO_SIGNAL_STATS_HH_

#include <map>
#include <string>
#include "gazebo/util/system.hh"

namespace gazebo
{
  namespace math
  {
    /// \addtogroup gazebo_math
    /// \{

    /// \brief Forward declare private data class.
    class SignalStatisticPrivate;

    /// \class SignalStatistic SignalStats.hh math/gzmath.hh
    /// \brief Statistical properties of a discrete time scalar signal.
    class GZ_MATH_VISIBLE SignalStatistic
    {
      /// \brief Constructor
      public: SignalStatistic();

      /// \brief Destructor
      public: virtual ~SignalStatistic();

      /// \brief Get the current value of the statistical measure.
      /// \return Current value of the statistical measure.
      public: virtual double Value() const = 0;

      /// \brief Get a short version of the name of this statistical measure.
      /// \return Short name of the statistical measure.
      public: virtual std::string ShortName() const = 0;

      /// \brief Get number of data points in measurement.
      /// \return Number of data points in measurement.
      public: virtual size_t Count() const;

      /// \brief Add a new sample to the statistical measure.
      /// \param[in] _data New signal data point.
      public: virtual void InsertData(const double _data) = 0;

      /// \brief Forget all previous data.
      public: virtual void Reset();

      /// \brief Pointer to private data.
      protected: SignalStatisticPrivate *dataPtr;
    };
    /// \}

    /// \class SignalMean SignalStats.hh math/gzmath.hh
    /// \brief Computing the mean value of a discretely sampled signal.
    class GZ_MATH_VISIBLE SignalMean : public SignalStatistic
    {
      // Documentation inherited.
      public: virtual double Value() const;

      /// \brief Get a short version of the name of this statistical measure.
      /// \return "mean"
      public: virtual std::string ShortName() const;

      // Documentation inherited.
      public: virtual void InsertData(const double _data);
    };
    /// \}

    /// \class SignalRootMeanSquare SignalStats.hh math/gzmath.hh
    /// \brief Computing the square root of the mean squared value
    /// of a discretely sampled signal.
    class GZ_MATH_VISIBLE SignalRootMeanSquare : public SignalStatistic
    {
      // Documentation inherited.
      public: virtual double Value() const;

      /// \brief Get a short version of the name of this statistical measure.
      /// \return "rms"
      public: virtual std::string ShortName() const;

      // Documentation inherited.
      public: virtual void InsertData(const double _data);
    };
    /// \}

    /// \class SignalMaxAbsoluteValue SignalStats.hh math/gzmath.hh
    /// \brief Computing the maximum of the absolute value
    /// of a discretely sampled signal.
    /// Also known as the maximum norm, infinity norm, or supremum norm.
    class GZ_MATH_VISIBLE SignalMaxAbsoluteValue : public SignalStatistic
    {
      // Documentation inherited.
      public: virtual double Value() const;

      /// \brief Get a short version of the name of this statistical measure.
      /// \return "maxAbs"
      public: virtual std::string ShortName() const;

      // Documentation inherited.
      public: virtual void InsertData(const double _data);
    };
    /// \}

    /// \brief Forward declare private data class.
    class SignalStatsPrivate;

    /// \class SignalStats SignalStats.hh math/gzmath.hh
    /// \brief Collection of statistics for a scalar signal.
    class GZ_MATH_VISIBLE SignalStats
    {
      /// \brief Constructor
      public: SignalStats();

      /// \brief Destructor
      public: ~SignalStats();

      /// \brief Get number of data points in first statistic.
      /// Technically you can have different numbers of data points
      /// in each statistic if you call InsertStatistic after InsertData,
      /// but this is not a recommended use case.
      /// \return Number of data points in first statistic.
      public: size_t Count() const;

      /// \brief Get the current values of each statistical measure,
      /// stored in a map using the short name as the key.
      /// \return Map with short name of each statistic as key
      /// and value of statistic as the value.
      public: std::map<std::string, double> Map() const;

      /// \brief Add a new sample to the statistical measures.
      /// \param[in] _data New signal data point.
      public: void InsertData(const double _data);

      /// \brief Add a new type of statistic.
      /// \param[in] _name Short name of new statistic.
      /// Valid values include:
      ///  "maxAbs"
      ///  "mean"
      ///  "rms"
      /// \return True if statistic was successfully added,
      /// false if name was not recognized or had already
      /// been inserted.
      public: bool InsertStatistic(const std::string &_name);

      /// \brief Add multiple statistics.
      /// \param[in] _names Comma-separated list of new statistics.
      /// For example, all statistics could be added with:
      ///  "maxAbs,mean,rms"
      /// \return True if all statistics were successfully added,
      /// false if any names were not recognized or had already
      /// been inserted.
      public: bool InsertStatistics(const std::string &_names);

      /// \brief Forget all previous data.
      public: void Reset();

      /// \brief Pointer to private data.
      protected: SignalStatsPrivate *dataPtr;
    };
    /// \}
  }
}
#endif

