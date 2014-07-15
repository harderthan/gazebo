/*
 * Copyright (C) 2012-2014 Open Source Robotics Foundation
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

#include <gtest/gtest.h>
#include "gazebo/math/Kmeans.hh"
#include "test/util.hh"

using namespace gazebo;

class KmeansTest : public gazebo::testing::AutoLogFixture { };

//////////////////////////////////////////////////
TEST_F(KmeansTest, Kmeans)
{
  // Create some observations.
  std::vector<math::Vector3> obs;
  obs.push_back(math::Vector3(1.0, 1.0, 0.0));
  obs.push_back(math::Vector3(1.1, 1.0, 0.0));
  obs.push_back(math::Vector3(1.2, 1.0, 0.0));
  obs.push_back(math::Vector3(1.3, 1.0, 0.0));
  obs.push_back(math::Vector3(1.4, 1.0, 0.0));
  obs.push_back(math::Vector3(5.0, 1.0, 0.0));
  obs.push_back(math::Vector3(5.1, 1.0, 0.0));
  obs.push_back(math::Vector3(5.2, 1.0, 0.0));
  obs.push_back(math::Vector3(5.3, 1.0, 0.0));
  obs.push_back(math::Vector3(5.4, 1.0, 0.0));

  // Initialize Kmeans with two partitions.
  math::Kmeans kmeans(obs, 2);

  // ::GetObservations()
  std::vector<math::Vector3> obsCopy;
  obsCopy = kmeans.GetObservations();
  for (size_t i = 0; i < obsCopy.size(); ++i)
    EXPECT_EQ(obsCopy[i], obs[i]);

  // ::SetObservations()
  for (size_t i = 0; i < obsCopy.size(); ++i)
    obsCopy[i] += math::Vector3(0.1, 0.2, 0.0);

  kmeans.SetObservations(obsCopy);

  obsCopy = kmeans.GetObservations();
  for (size_t i = 0; i < obsCopy.size(); ++i)
    EXPECT_EQ(obsCopy[i], obs[i] + math::Vector3(0.1, 0.2, 0.0));
  kmeans.SetObservations(obs);

  // ::GetClusters()
  EXPECT_EQ(kmeans.GetNumClusters(), 2);

  // ::SetClusters()
  kmeans.SetNumClusters(3);
  EXPECT_EQ(kmeans.GetNumClusters(), 3);
  kmeans.SetNumClusters(2);

  // ::Cluster()
  std::vector<math::Vector3> centroids;
  std::vector<unsigned int> labels;
  kmeans.Cluster(centroids, labels);

  // Check that there are two centroids.
  EXPECT_EQ(centroids.size(), 2);

  // Check that the observations are clustered properly.
  EXPECT_EQ(labels[0], labels[1]);
  EXPECT_EQ(labels[1], labels[2]);
  EXPECT_EQ(labels[2], labels[3]);
  EXPECT_EQ(labels[3], labels[4]);

  EXPECT_NE(labels[4], labels[5]);

  EXPECT_EQ(labels[5], labels[6]);
  EXPECT_EQ(labels[6], labels[7]);
  EXPECT_EQ(labels[7], labels[8]);
  EXPECT_EQ(labels[8], labels[9]);

  // Check the centroids
  math::Vector3 expectedCentroid1(1.2, 1.0, 0.0);
  math::Vector3 expectedCentroid2(5.2, 1.0, 0.0);
  if (centroids[0] == expectedCentroid1)
    EXPECT_EQ(centroids[1], expectedCentroid2);
  else if (centroids[0] == expectedCentroid2)
    EXPECT_EQ(centroids[1], expectedCentroid1);
  else
    FAIL();
}
