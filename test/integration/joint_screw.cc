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
#include "gazebo/physics/physics.hh"
// #include "gazebo/physics/Joint.hh"
// #include "gazebo/physics/ScrewJoint.hh"
#include "ServerFixture.hh"
#include "helper_physics_generator.hh"

using namespace gazebo;

class JointTestScrew : public ServerFixture,
                       public testing::WithParamInterface<const char*>
{
  /// \brief Test screw joint implementation with SetWorldPose.
  /// Set link poses in world frame, check joint angles and joint axis.
  /// \param[in] _physicsEngine Type of physics engine to use.
  public: void ScrewJointSetWorldPose(const std::string &_physicsEngine);

  /// \brief Test screw joint implementation with forces.
  /// Apply force to screw joint links, check velocity.
  /// \param[in] _physicsEngine Type of physics engine to use.
  public: void ScrewJointForce(const std::string &_physicsEngine);
};

//////////////////////////////////////////////////
void JointTestScrew::ScrewJointSetWorldPose(const std::string &_physicsEngine)
{
  if (_physicsEngine == "bullet")
  {
    gzerr << "Bullet Screw Joint will not work until pull request #1008.\n";
    return;
  }

  if (_physicsEngine == "simbody")
  {
    gzerr << "Simbody Screw Joint will not work with Link::SetWorldPose."
          << " See issue #857.\n";
    return;
  }

  // Load our screw joint test world
  Load("worlds/screw_joint_test.world", true, _physicsEngine);

  // Get a pointer to the world, make sure world loads
  physics::WorldPtr world = physics::get_world("default");
  ASSERT_TRUE(world != NULL);

  // Verify physics engine type
  physics::PhysicsEnginePtr physics = world->GetPhysicsEngine();
  ASSERT_TRUE(physics != NULL);
  EXPECT_EQ(physics->GetType(), _physicsEngine);

  physics->SetGravity(math::Vector3(0, 0, 0));

  // simulate 1 step
  world->Step(1);
  double t = world->GetSimTime().Double();

  // get time step size
  double dt = world->GetPhysicsEngine()->GetMaxStepSize();
  EXPECT_GT(dt, 0);
  gzlog << "dt : " << dt << "\n";

  // verify that time moves forward
  EXPECT_DOUBLE_EQ(t, dt);
  gzlog << "t after one step : " << t << "\n";

  // get model, joints and get links
  physics::ModelPtr model_1 = world->GetModel("model_1");
  physics::LinkPtr link_00 = model_1->GetLink("link_00");
  physics::LinkPtr link_01 = model_1->GetLink("link_01");
  physics::JointPtr joint_00 = model_1->GetJoint("joint_00");
  physics::JointPtr joint_01 = model_1->GetJoint("joint_01");

  // both initial angles should be zero
  EXPECT_EQ(joint_00->GetAngle(0), 0);
  EXPECT_EQ(joint_00->GetAngle(1), 0);

  // move child link to it's initial location
  link_00->SetWorldPose(math::Pose(0, 0, 2, 0, 0, 0));
  EXPECT_EQ(joint_00->GetAngle(0), 0);
  EXPECT_EQ(joint_00->GetAngle(1), 0);
  EXPECT_EQ(joint_00->GetGlobalAxis(0), math::Vector3(1, 0, 0));
  EXPECT_EQ(joint_00->GetGlobalAxis(1), math::Vector3(1, 0, 0));
  gzdbg << "joint angles [" << joint_00->GetAngle(0)
        << ", " << joint_00->GetAngle(1)
        << "] axis1 [" << joint_00->GetGlobalAxis(0)
        << "] axis2 [" << joint_00->GetGlobalAxis(1)
        << "]\n";

  // move child link 45deg about x
  double pitch_00 = joint_00->GetAttribute("thread_pitch", 0);
  math::Pose pose_00 = math::Pose(0.25*M_PI*pitch_00, 0, 2, 0.25*M_PI, 0, 0);
  math::Pose pose_01 = math::Pose(0, 0, -1, 0, 0, 0) + pose_00;
  link_00->SetWorldPose(pose_00);
  link_01->SetWorldPose(pose_01);
  EXPECT_EQ(joint_00->GetAngle(0), 0.25*M_PI);
  EXPECT_EQ(joint_00->GetAngle(1), 0.25*M_PI*pitch_00);
  EXPECT_EQ(joint_00->GetGlobalAxis(0), math::Vector3(1, 0, 0));
  EXPECT_EQ(joint_00->GetGlobalAxis(1), math::Vector3(1, 0, 0));
  gzdbg << "joint angles [" << joint_00->GetAngle(0)
        << ", " << joint_00->GetAngle(1)
        << "] axis1 [" << joint_00->GetGlobalAxis(0)
        << "] axis2 [" << joint_00->GetGlobalAxis(1)
        << "] pitch_00 [" << pitch_00
        << "]\n";

  // move child link 45deg about y
  double pitch_01 = joint_01->GetAttribute("thread_pitch", 0);
  link_00->SetWorldPose(math::Pose(0, 0, 2, 0, 0.25*M_PI, 0));
  pose_00 = math::Pose(0.25*M_PI*pitch_00, 0, 2, 0.25*M_PI, 0, 0);
  pose_01 = math::Pose(0.3*M_PI*pitch_01, 0, -1, 0.3*M_PI, 0, 0) + pose_00;
  link_00->SetWorldPose(pose_00);
  link_01->SetWorldPose(pose_01);
  EXPECT_EQ(joint_00->GetAngle(0), 0.25*M_PI);
  EXPECT_EQ(joint_00->GetAngle(1), 0.25*M_PI*pitch_00);
  EXPECT_EQ(joint_01->GetAngle(0), 0.3*M_PI);
  EXPECT_EQ(joint_01->GetAngle(1), 0.3*M_PI*pitch_01);
  EXPECT_EQ(joint_00->GetGlobalAxis(0), math::Vector3(1, 0, 0));
  EXPECT_EQ(joint_00->GetGlobalAxis(1), math::Vector3(1, 0, 0));
  gzdbg << "joint angles [" << joint_00->GetAngle(0)
        << ", " << joint_00->GetAngle(1)
        << "] axis1 [" << joint_00->GetGlobalAxis(0)
        << "] axis2 [" << joint_00->GetGlobalAxis(1)
        << "] pitch_00 [" << pitch_00
        << "] pitch_01 [" << pitch_01
        << "]\n";

  // new poses should not violate the constraint.  take a few steps
  // and make sure nothing moves.
  world->Step(10);

  // move child link 90deg about both x and "rotated y axis" (z)
  EXPECT_EQ(joint_00->GetAngle(0), 0.25*M_PI);
  EXPECT_EQ(joint_00->GetAngle(1), 0.25*M_PI*pitch_00);
  EXPECT_EQ(joint_01->GetAngle(0), 0.3*M_PI);
  EXPECT_EQ(joint_01->GetAngle(1), 0.3*M_PI*pitch_01);
  EXPECT_EQ(joint_00->GetGlobalAxis(0), math::Vector3(1, 0, 0));
  EXPECT_EQ(joint_00->GetGlobalAxis(1), math::Vector3(1, 0, 0));
  gzdbg << "joint angles [" << joint_00->GetAngle(0)
        << ", " << joint_00->GetAngle(1)
        << "] axis1 [" << joint_00->GetGlobalAxis(0)
        << "] axis2 [" << joint_00->GetGlobalAxis(1)
        << "] pitch_00 [" << pitch_00
        << "] pitch_01 [" << pitch_01
        << "]\n";
}

TEST_P(JointTestScrew, ScrewJointSetWorldPose)
{
  ScrewJointSetWorldPose(GetParam());
}

//////////////////////////////////////////////////
void JointTestScrew::ScrewJointForce(const std::string &_physicsEngine)
{
  if (_physicsEngine == "bullet")
  {
    gzerr << "Bullet Screw Joint will not work until pull request #1008.\n";
    return;
  }

  // Load our screw joint test world
  Load("worlds/screw_joint_test.world", true, _physicsEngine);

  // Get a pointer to the world, make sure world loads
  physics::WorldPtr world = physics::get_world("default");
  ASSERT_TRUE(world != NULL);

  // Verify physics engine type
  physics::PhysicsEnginePtr physics = world->GetPhysicsEngine();
  ASSERT_TRUE(physics != NULL);
  EXPECT_EQ(physics->GetType(), _physicsEngine);

  physics->SetGravity(math::Vector3(0, 0, 0));

  // simulate 1 step
  world->Step(1);
  double t = world->GetSimTime().Double();

  // get time step size
  double dt = world->GetPhysicsEngine()->GetMaxStepSize();
  EXPECT_GT(dt, 0);
  gzlog << "dt : " << dt << "\n";

  // verify that time moves forward
  EXPECT_DOUBLE_EQ(t, dt);
  gzlog << "t after one step : " << t << "\n";

  // get model, joints and get links
  physics::ModelPtr model_1 = world->GetModel("model_1");
  physics::LinkPtr link_00 = model_1->GetLink("link_00");
  physics::LinkPtr link_01 = model_1->GetLink("link_01");
  physics::JointPtr joint_00 = model_1->GetJoint("joint_00");
  physics::JointPtr joint_01 = model_1->GetJoint("joint_01");
  double pitch_00 = joint_00->GetAttribute("thread_pitch", 0);
  double pitch_01 = joint_01->GetAttribute("thread_pitch", 0);

  // both initial angles should be zero
  EXPECT_EQ(joint_00->GetAngle(0), 0);
  EXPECT_EQ(joint_00->GetAngle(1), 0);

  // set new upper limit for joint_00
  joint_00->SetHighStop(0, 0.3);
  // push joint_00 till it hits new upper limit
  while (joint_00->GetAngle(0) < 0.3)
  {
    joint_00->SetForce(0, 0.1);
    world->Step(1);
    // check link pose
    double angle_00_angular = joint_00->GetAngle(0).Radian();
    EXPECT_EQ(link_00->GetWorldPose(),
      math::Pose(angle_00_angular * pitch_00, 0, 2, angle_00_angular, 0, 0));

    if (_physicsEngine == "simbody")
    {
      double angle_00_linear = joint_00->GetAngle(1).Radian();
      gzerr << "issue #857 in simbody screw joint linear angle:"
            << " joint_00 " << angle_00_linear
            << " shoudl be 0.3\n";
    }
  }
  // lock joint at this location by setting lower limit here too
  joint_00->SetLowStop(0, 0.3);

  // set joint_01 upper limit to 1.0
  joint_01->SetHighStop(0, 1.0);
  // push joint_01 until limit is reached
  while (joint_01->GetAngle(0) < 1.0)
  {
    joint_01->SetForce(0, 0.1);
    world->Step(1);

    // check link pose
    math::Pose pose_00 = link_00->GetWorldPose();
    math::Pose pose_01 = link_01->GetWorldPose();
    double angle_00_angular = joint_00->GetAngle(0).Radian();
    double angle_00_linear = joint_00->GetAngle(1).Radian();
    double angle_01_angular = joint_01->GetAngle(0).Radian();
    double angle_01_linear = joint_01->GetAngle(1).Radian();

    EXPECT_EQ(pose_00, math::Pose(
      angle_00_angular * pitch_00, 0, 2, angle_00_angular, 0, 0));
    if (_physicsEngine == "simbody")
    {
      gzerr << "issue #857 in simbody screw joint linear angle:"
            << " joint_00 " << angle_00_linear
            << " should be 0.3. "
            << " joint_01 " << angle_01_linear
            << " is off too.\n";
    }
    else
    {
      EXPECT_NEAR(pose_01.pos.x, angle_00_linear + angle_01_linear, 1e-8);
    }
    EXPECT_NEAR(pose_01.pos.x,
      angle_00_angular * pitch_00 + angle_01_angular * pitch_01, 1e-8);
    EXPECT_NEAR(pose_01.rot.GetAsEuler().x,
      angle_00_angular + angle_01_angular, 1e-8);
  }

  // push joint_01 the other way until -1 is reached
  while (joint_01->GetAngle(0) > -1.0)
  {
    joint_01->SetForce(0, -0.1);
    world->Step(1);

    // check link pose
    math::Pose pose_00 = link_00->GetWorldPose();
    math::Pose pose_01 = link_01->GetWorldPose();
    double angle_00_angular = joint_00->GetAngle(0).Radian();
    double angle_00_linear = joint_00->GetAngle(1).Radian();
    double angle_01_angular = joint_01->GetAngle(0).Radian();
    double angle_01_linear = joint_01->GetAngle(1).Radian();

    EXPECT_EQ(pose_00, math::Pose(
      angle_00_angular * pitch_00, 0, 2, angle_00_angular, 0, 0));
    if (_physicsEngine == "simbody")
    {
      gzerr << "issue #857 in simbody screw joint linear angle:"
            << " joint_00 " << angle_00_linear
            << " should be 0.3. "
            << " joint_01 " << angle_01_linear
            << " is off too.\n";
    }
    else
    {
      EXPECT_NEAR(pose_01.pos.x, angle_00_linear + angle_01_linear, 1e-8);
    }
    EXPECT_NEAR(pose_01.pos.x,
      angle_00_angular * pitch_00 + angle_01_angular * pitch_01, 1e-8);
    EXPECT_NEAR(pose_01.rot.GetAsEuler().x,
      angle_00_angular + angle_01_angular, 1e-8);
  }
}

TEST_P(JointTestScrew, ScrewJointForce)
{
  ScrewJointForce(GetParam());
}

INSTANTIATE_TEST_CASE_P(PhysicsEngines, JointTestScrew,
  PHYSICS_ENGINE_VALUES);

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
