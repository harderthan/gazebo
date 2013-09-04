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

#include <gtest/gtest.h>
#include "gazebo/physics/physics.hh"
#include "gazebo/physics/Joint.hh"
#include "gazebo/physics/Joint_TEST.hh"

#define TOL 1e-6
#define TOL_CONT 2.0

using namespace gazebo;

void Joint_TEST::ForceTorque1(const std::string &_physicsEngine)
{
  // Load our force torque test world
  Load("worlds/force_torque_test.world", true, _physicsEngine);

  // Get a pointer to the world, make sure world loads
  physics::WorldPtr world = physics::get_world("default");
  ASSERT_TRUE(world != NULL);

  // Verify physics engine type
  physics::PhysicsEnginePtr physics = world->GetPhysicsEngine();
  ASSERT_TRUE(physics != NULL);
  EXPECT_EQ(physics->GetType(), _physicsEngine);

  physics->SetGravity(math::Vector3(0, 0, -50));

  // simulate 1 step
  world->StepWorld(1);
  double t = world->GetSimTime().Double();

  // get time step size
  double dt = world->GetPhysicsEngine()->GetMaxStepSize();
  EXPECT_GT(dt, 0);
  gzdbg << "dt : " << dt << "\n";

  // verify that time moves forward
  EXPECT_EQ(t, dt);
  gzdbg << "t after one step : " << t << "\n";

  // get joint and get force torque
  physics::ModelPtr model_1 = world->GetModel("model_1");
  physics::LinkPtr link_1 = model_1->GetLink("link_1");
  physics::LinkPtr link_2 = model_1->GetLink("link_2");
  physics::JointPtr joint_01 = model_1->GetJoint("joint_01");
  physics::JointPtr joint_12 = model_1->GetJoint("joint_12");

  gzdbg << "-------------------Test 1-------------------\n";
  for (unsigned int i = 0; i < 10; ++i)
  {
    world->StepWorld(1);
    // test joint_01 wrench
    physics::JointWrench wrench_01 = joint_01->GetForceTorque(0u);
    EXPECT_DOUBLE_EQ(wrench_01.body1Force.x,    0.0);
    EXPECT_DOUBLE_EQ(wrench_01.body1Force.y,    0.0);
    EXPECT_DOUBLE_EQ(wrench_01.body1Force.z, 1000.0);
    EXPECT_DOUBLE_EQ(wrench_01.body1Torque.x,   0.0);
    EXPECT_DOUBLE_EQ(wrench_01.body1Torque.y,   0.0);
    EXPECT_DOUBLE_EQ(wrench_01.body1Torque.z,   0.0);

    EXPECT_DOUBLE_EQ(wrench_01.body2Force.x,  -wrench_01.body1Force.x);
    EXPECT_DOUBLE_EQ(wrench_01.body2Force.y,  -wrench_01.body1Force.y);
    EXPECT_DOUBLE_EQ(wrench_01.body2Force.z,  -wrench_01.body1Force.z);
    EXPECT_DOUBLE_EQ(wrench_01.body2Torque.x, -wrench_01.body1Torque.x);
    EXPECT_DOUBLE_EQ(wrench_01.body2Torque.y, -wrench_01.body1Torque.y);
    EXPECT_DOUBLE_EQ(wrench_01.body2Torque.z, -wrench_01.body1Torque.z);

    gzdbg << "link_1 pose [" << link_1->GetWorldPose()
          << "] velocity [" << link_1->GetWorldLinearVel()
          << "]\n";
    gzdbg << "link_2 pose [" << link_2->GetWorldPose()
          << "] velocity [" << link_2->GetWorldLinearVel()
          << "]\n";
    gzdbg << "joint_01 force torque : "
          << "force1 [" << wrench_01.body1Force
          << " / 0 0 1000"
          << "] torque1 [" << wrench_01.body1Torque
          << " / 0 0 0"
          << "] force2 [" << wrench_01.body2Force
          << " / 0 0 -1000"
          << "] torque2 [" << wrench_01.body2Torque
          << " / 0 0 0"
          << "]\n";

    // test joint_12 wrench
    physics::JointWrench wrench_12 = joint_12->GetForceTorque(0u);
    EXPECT_DOUBLE_EQ(wrench_12.body1Force.x,    0.0);
    EXPECT_DOUBLE_EQ(wrench_12.body1Force.y,    0.0);
    EXPECT_DOUBLE_EQ(wrench_12.body1Force.z,  500.0);
    EXPECT_DOUBLE_EQ(wrench_12.body1Torque.x,   0.0);
    EXPECT_DOUBLE_EQ(wrench_12.body1Torque.y,   0.0);
    EXPECT_DOUBLE_EQ(wrench_12.body1Torque.z,   0.0);

    EXPECT_DOUBLE_EQ(wrench_12.body2Force.x,  -wrench_12.body1Force.x);
    EXPECT_DOUBLE_EQ(wrench_12.body2Force.y,  -wrench_12.body1Force.y);
    EXPECT_DOUBLE_EQ(wrench_12.body2Force.z,  -wrench_12.body1Force.z);
    EXPECT_DOUBLE_EQ(wrench_12.body2Torque.x, -wrench_12.body1Torque.x);
    EXPECT_DOUBLE_EQ(wrench_12.body2Torque.y, -wrench_12.body1Torque.y);
    EXPECT_DOUBLE_EQ(wrench_12.body2Torque.z, -wrench_12.body1Torque.z);

    gzdbg << "link_1 pose [" << link_1->GetWorldPose()
          << "] velocity [" << link_1->GetWorldLinearVel()
          << "]\n";
    gzdbg << "link_2 pose [" << link_2->GetWorldPose()
          << "] velocity [" << link_2->GetWorldLinearVel()
          << "]\n";
    gzdbg << "joint_12 force torque : "
          << "force1 [" << wrench_12.body1Force
          << " / 0 0 500"
          << "] torque1 [" << wrench_12.body1Torque
          << " / 0 0 0"
          << "] force2 [" << wrench_12.body2Force
          << " / 0 0 -500"
          << "] torque2 [" << wrench_12.body2Torque
          << " / 0 0 0"
          << "]\n";
  }
}

TEST_F(Joint_TEST, ForceTorque1ODE)
{
  ForceTorque1("ode");
}

#ifdef HAVE_SIMBODY
TEST_F(Joint_TEST, ForceTorque1Simbody)
{
  ForceTorque1("simbody");
}
#endif  // HAVE_SIMBODY

#ifdef HAVE_BULLET

/// bullet collision parameters needs tweaking?
TEST_F(Joint_TEST, ForceTorque1Bullet)
{
  ForceTorque1("bullet");
}
#endif  // HAVE_BULLET

void Joint_TEST::ForceTorque2(const std::string &_physicsEngine)
{
  // Load our force torque test world
  Load("worlds/force_torque_test.world", true, _physicsEngine);

  // Get a pointer to the world, make sure world loads
  physics::WorldPtr world = physics::get_world("default");
  ASSERT_TRUE(world != NULL);

  // Verify physics engine type
  physics::PhysicsEnginePtr physics = world->GetPhysicsEngine();
  ASSERT_TRUE(physics != NULL);
  EXPECT_EQ(physics->GetType(), _physicsEngine);

  physics->SetGravity(math::Vector3(0, 0, -50));

  // simulate 1 step
  world->StepWorld(1);
  double t = world->GetSimTime().Double();

  // get time step size
  double dt = world->GetPhysicsEngine()->GetMaxStepSize();
  EXPECT_GT(dt, 0);
  gzdbg << "dt : " << dt << "\n";

  // verify that time moves forward
  EXPECT_EQ(t, dt);
  gzdbg << "t after one step : " << t << "\n";

  // get joint and get force torque
  physics::ModelPtr model_1 = world->GetModel("model_1");
  physics::LinkPtr link_1 = model_1->GetLink("link_1");
  physics::LinkPtr link_2 = model_1->GetLink("link_2");
  physics::JointPtr joint_01 = model_1->GetJoint("joint_01");
  physics::JointPtr joint_12 = model_1->GetJoint("joint_12");

  // perturbe joints so top link topples over, then remeasure
  physics->SetGravity(math::Vector3(-30, 10, -50));
  // tune joint stop properties
  joint_01->SetAttribute("stop_erp", 0, 0.02);
  joint_12->SetAttribute("stop_erp", 0, 0.02);
  // wait for dynamics to stabilize
  world->StepWorld(2000);
  // check force torques in new system
  gzdbg << "\n-------------------Test 2-------------------\n";
  for (unsigned int i = 0; i < 5; ++i)
  {
    world->StepWorld(1);
    // Dbg joint_01 force torque :
    //   force1 [600 -200 999.99999600000001 / 600 -1000 -200]
    //   torque1 [749.999819 82.840868 -450.00009699999998 / 750 450 0]
    //   force2 [-600 999.99976200000003 200.00117299999999 / -600 1000 200]
    //   torque2 [-749.999819 -450 -82.841396000000003 / -750 -450 0]
    // Dbg joint_12 force torque :
    //   force1 [300 -499.99987900000002 -100.000587 / 300 -500 -100]
    //   torque1 [249.99994000000001 150 82.841396000000003 / 250 150 0]
    //   force2 [-300.000407 499.99963500000001 100.000587 / -300 500 100]
    //   torque2 [-249.999818 -150.000203 -82.841396000000003 / -250 -150 0]

    // test joint_01 wrench
    physics::JointWrench wrench_01 = joint_01->GetForceTorque(0u);
    EXPECT_NEAR(wrench_01.body1Force.x,   600.0,  6.0);
    EXPECT_NEAR(wrench_01.body1Force.y,  -200.0, 10.0);
    EXPECT_NEAR(wrench_01.body1Force.z,  1000.0,  2.0);
    EXPECT_NEAR(wrench_01.body1Torque.x,  750.0,  7.5);
    EXPECT_NEAR(wrench_01.body1Torque.y,    0.0,  4.5);
    EXPECT_NEAR(wrench_01.body1Torque.z, -450.0,  0.1);

    EXPECT_NEAR(wrench_01.body2Force.x,  -600.0,  6.0);
    EXPECT_NEAR(wrench_01.body2Force.y,  1000.0, 10.0);
    EXPECT_NEAR(wrench_01.body2Force.z,   200.0,  2.0);
    EXPECT_NEAR(wrench_01.body2Torque.x, -750.0,  7.5);
    EXPECT_NEAR(wrench_01.body2Torque.y, -450.0,  4.5);
    EXPECT_NEAR(wrench_01.body2Torque.z,    0.0,  0.1);

    gzdbg << "joint_01 force torque : "
          << "force1 [" << wrench_01.body1Force
          << " / 600 -200 1000"
          << "] torque1 [" << wrench_01.body1Torque
          << " / 750 0 450"
          << "] force2 [" << wrench_01.body2Force
          << " / -600 1000 200"
          << "] torque2 [" << wrench_01.body2Torque
          << " / -750 -450 0"
          << "]\n";

    gzdbg << "joint angle1[" << std::setprecision(17) << joint_01->GetAngle(0)
          << "] angle2[" << joint_12->GetAngle(0) << "]\n";

    // test joint_12 wrench
    physics::JointWrench wrench_12 = joint_12->GetForceTorque(0u);
    EXPECT_NEAR(wrench_12.body1Force.x,   300.0,  3.0);
    EXPECT_NEAR(wrench_12.body1Force.y,  -500.0,  5.0);
    EXPECT_NEAR(wrench_12.body1Force.z,  -100.0,  1.0);
    EXPECT_NEAR(wrench_12.body1Torque.x,  250.0,  5.0);
    EXPECT_NEAR(wrench_12.body1Torque.y,  150.0,  3.0);
    EXPECT_NEAR(wrench_12.body1Torque.z,    0.0,  0.1);

    // A good check is that
    // the computed body1Torque shoud in fact be opposite of body1Torque
    EXPECT_NEAR(wrench_12.body2Force.x,  -wrench_12.body1Force.x,  1e-1);
    EXPECT_NEAR(wrench_12.body2Force.y,  -wrench_12.body1Force.y,  1e-1);
    EXPECT_NEAR(wrench_12.body2Force.z,  -wrench_12.body1Force.z,  1e-1);
    EXPECT_NEAR(wrench_12.body2Torque.x, -wrench_12.body1Torque.x, 1e-1);
    EXPECT_NEAR(wrench_12.body2Torque.y, -wrench_12.body1Torque.y, 1e-1);
    EXPECT_NEAR(wrench_12.body2Torque.z, -wrench_12.body1Torque.z, 1e-1);

    gzdbg << "joint_12 force torque : "
          << "force1 [" << wrench_12.body1Force
          << " / 300 -500 -100"
          << "] torque1 [" << wrench_12.body1Torque
          << " / 250 150 0"
          << "] force2 [" << wrench_12.body2Force
          << " / -300 500 100"
          << "] torque2 [" << wrench_12.body2Torque
          << " / -250 -150 0"
          << "]\n";
  }

  // simulate a few steps
  int steps = 20;
  world->StepWorld(steps);
  t = world->GetSimTime().Double();
  EXPECT_GT(t, 0.99*dt*static_cast<double>(steps+1));
  gzdbg << "t after 20 steps : " << t << "\n";
}

TEST_F(Joint_TEST, ForceTorque2ODE)
{
  ForceTorque2("ode");
}

#ifdef HAVE_SIMBODY
TEST_F(Joint_TEST, ForceTorque2Simbody)
{
  ForceTorque2("simbody");
}
#endif  // HAVE_SIMBODY

#ifdef HAVE_BULLET
TEST_F(Joint_TEST, ForceTorque2Bullet)
{
  ForceTorque2("bullet");
}
#endif  // HAVE_BULLET

void Joint_TEST::GetForceTorqueWithAppliedForce(
  const std::string &_physicsEngine)
{
  // Load our force torque test world
  Load("worlds/force_torque_test2.world", true, _physicsEngine);

  // Get a pointer to the world, make sure world loads
  physics::WorldPtr world = physics::get_world("default");
  ASSERT_TRUE(world != NULL);

  // Verify physics engine type
  physics::PhysicsEnginePtr physics = world->GetPhysicsEngine();
  ASSERT_TRUE(physics != NULL);
  EXPECT_EQ(physics->GetType(), _physicsEngine);

  physics->SetGravity(math::Vector3(0, 0, -50));

  // simulate 1 step
  world->StepWorld(1);
  double t = world->GetSimTime().Double();

  // get time step size
  double dt = world->GetPhysicsEngine()->GetMaxStepSize();
  EXPECT_GT(dt, 0);
  gzdbg << "dt : " << dt << "\n";

  // verify that time moves forward
  EXPECT_GT(t, 0);
  gzdbg << "t after one step : " << t << "\n";

  // get joint and get force torque
  physics::ModelPtr model_1 = world->GetModel("boxes");
  physics::JointPtr joint_01 = model_1->GetJoint("joint1");
  physics::JointPtr joint_12 = model_1->GetJoint("joint2");

  gzdbg << "------------------- PD CONTROL -------------------\n";
  static const double kp1 = 50000.0;
  static const double kp2 = 10000.0;
  static const double target1 = 0.0;
  static const double target2 = -0.25*M_PI;
  for (unsigned int i = 0; i < 3388; ++i)
  {
    // pd control
    double j1State = joint_01->GetAngle(0u).Radian();
    double j2State = joint_12->GetAngle(0u).Radian();
    double p1Error = target1 - j1State;
    double p2Error = target2 - j2State;
    double effort1 = kp1 * p1Error;
    double effort2 = kp2 * p2Error;
    joint_01->SetForce(0u, effort1);
    joint_12->SetForce(0u, effort2);

    world->StepWorld(1);
    // test joint_01 wrench
    physics::JointWrench wrench_01 = joint_01->GetForceTorque(0u);

    if (i == 3387)
    {
      EXPECT_NEAR(wrench_01.body1Force.x,     0.0, TOL_CONT);
      EXPECT_NEAR(wrench_01.body1Force.y,     0.0, TOL_CONT);
      EXPECT_NEAR(wrench_01.body1Force.z,   300.0, TOL_CONT);
      EXPECT_NEAR(wrench_01.body1Torque.x,   25.0, TOL_CONT);
      EXPECT_NEAR(wrench_01.body1Torque.y, -175.0, TOL_CONT);
      EXPECT_NEAR(wrench_01.body1Torque.z,    0.0, TOL_CONT);

      EXPECT_NEAR(wrench_01.body2Force.x,  -wrench_01.body1Force.x,  TOL_CONT);
      EXPECT_NEAR(wrench_01.body2Force.y,  -wrench_01.body1Force.y,  TOL_CONT);
      EXPECT_NEAR(wrench_01.body2Force.z,  -wrench_01.body1Force.z,  TOL_CONT);
      EXPECT_NEAR(wrench_01.body2Torque.x, -wrench_01.body1Torque.x, TOL_CONT);
      EXPECT_NEAR(wrench_01.body2Torque.y, -wrench_01.body1Torque.y, TOL_CONT);
      EXPECT_NEAR(wrench_01.body2Torque.z, -wrench_01.body1Torque.z, TOL_CONT);

      gzdbg << "joint_01 force torque : "
            << "step [" << i
            << "] GetForce [" << joint_01->GetForce(0u)
            << "] command [" << effort1
            << "] force1 [" << wrench_01.body1Force
            << "] torque1 [" << wrench_01.body1Torque
            << "] force2 [" << wrench_01.body2Force
            << "] torque2 [" << wrench_01.body2Torque
            << "]\n";
    }

    // test joint_12 wrench
    physics::JointWrench wrench_12 = joint_12->GetForceTorque(0u);
    if (i == 3387)
    {
      EXPECT_NEAR(wrench_12.body1Force.x,     0.0, TOL_CONT);
      EXPECT_NEAR(wrench_12.body1Force.y,     0.0, TOL_CONT);
      EXPECT_NEAR(wrench_12.body1Force.z,    50.0, TOL_CONT);
      EXPECT_NEAR(wrench_12.body1Torque.x,   25.0, TOL_CONT);
      EXPECT_NEAR(wrench_12.body1Torque.y,    0.0, TOL_CONT);
      EXPECT_NEAR(wrench_12.body1Torque.z,    0.0, TOL_CONT);

      EXPECT_NEAR(wrench_12.body2Force.x,   -35.355, TOL_CONT);
      EXPECT_NEAR(wrench_12.body2Force.y,     0.000, TOL_CONT);
      EXPECT_NEAR(wrench_12.body2Force.z,   -35.355, TOL_CONT);
      EXPECT_NEAR(wrench_12.body2Torque.x,  -17.678, TOL_CONT);
      EXPECT_NEAR(wrench_12.body2Torque.y,    0.000, TOL_CONT);
      EXPECT_NEAR(wrench_12.body2Torque.z,   17.678, TOL_CONT);

      gzdbg << "joint_12 force torque : "
            << "step [" << i
            << "] GetForce [" << joint_12->GetForce(0u)
            << "] command [" << effort2
            << "] force1 [" << wrench_12.body1Force
            << "] torque1 [" << wrench_12.body1Torque
            << "] force2 [" << wrench_12.body2Force
            << "] torque2 [" << wrench_12.body2Torque
            << "]\n";
    }
    gzdbg << "angles[" << i << "] 1[" << joint_01->GetAngle(0)
          << "] 2[" << joint_12->GetAngle(0)
          << "]\n";
  }
}

TEST_F(Joint_TEST, GetForceTorqueWithAppliedForceODE)
{
  GetForceTorqueWithAppliedForce("ode");
}

#ifdef HAVE_SIMBODY
TEST_F(Joint_TEST, GetForceTorqueWithAppliedForceSimbody)
{
  GetForceTorqueWithAppliedForce("simbody");
}
#endif  // HAVE_SIMBODY

#ifdef HAVE_BULLET
/// bullet collision parameters needs tweaking
TEST_F(Joint_TEST, GetForceTorqueWithAppliedForceBullet)
{
  GetForceTorqueWithAppliedForce("bullet");
}
#endif  // HAVE_BULLET

void Joint_TEST::SpawnJointTypes(const std::string &_physicsEngine)
{
  // Load an empty world
  Load("worlds/empty.world", true, _physicsEngine);
  physics::WorldPtr world = physics::get_world("default");
  ASSERT_TRUE(world != NULL);

  // Verify physics engine type
  physics::PhysicsEnginePtr physics = world->GetPhysicsEngine();
  ASSERT_TRUE(physics != NULL);
  EXPECT_EQ(physics->GetType(), _physicsEngine);

  std::vector<std::string> types;
  types.push_back("revolute");
  types.push_back("prismatic");
  types.push_back("screw");
  types.push_back("universal");
  types.push_back("ball");
  types.push_back("revolute2");

  physics::JointPtr joint;
  for (std::vector<std::string>::iterator iter = types.begin();
       iter != types.end(); ++iter)
  {
    gzdbg << "SpawnJoint " << *iter << " child parent" << std::endl;
    joint = SpawnJoint(*iter, false, false);
    EXPECT_TRUE(joint != NULL);

    gzdbg << "SpawnJoint " << *iter << " child world" << std::endl;
    joint = SpawnJoint(*iter, false, true);
    EXPECT_TRUE(joint != NULL);

    gzdbg << "SpawnJoint " << *iter << " world parent" << std::endl;
    joint = SpawnJoint(*iter, true, false);
    EXPECT_TRUE(joint != NULL);
  }
}

TEST_F(Joint_TEST, SpawnJointTypesODE)
{
  SpawnJointTypes("ode");
}

#ifdef HAVE_BULLET
TEST_F(Joint_TEST, SpawnJointTypesBullet)
{
  SpawnJointTypes("bullet");
}
#endif  // HAVE_BULLET

#ifdef HAVE_SIMBODY
TEST_F(Joint_TEST, SpawnJointTypesSimbody)
{
  SpawnJointTypes("simbody");
}
#endif  // HAVE_SIMBODY

void Joint_TEST::JointTorqueTest(const std::string &_physicsEngine)
{
  // Load our inertial test world
  Load("worlds/joint_test.world", true, _physicsEngine);

  // Get a pointer to the world, make sure world loads
  physics::WorldPtr world = physics::get_world("default");
  ASSERT_TRUE(world != NULL);

  // Verify physics engine type
  physics::PhysicsEnginePtr physics = world->GetPhysicsEngine();
  ASSERT_TRUE(physics != NULL);
  EXPECT_EQ(physics->GetType(), "ode");

  // create some fake links
  physics::ModelPtr model = world->GetModel("model_1");
  ASSERT_TRUE(model != NULL);
  physics::LinkPtr link = model->GetLink("link_1");
  ASSERT_TRUE(link != NULL);

  physics::LinkPtr parentLink;
  physics::LinkPtr childLink(link);
  physics::JointPtr joint;
  math::Pose anchor;
  double upper = M_PI;
  double lower = -M_PI;

  {
    // create a joint
    {
      joint = world->GetPhysicsEngine()->CreateJoint(
        "revolute", model);
      joint->Attach(parentLink, childLink);
      // load adds the joint to a vector of shared pointers kept
      // in parent and child links, preventing joint from being destroyed.
      joint->Load(parentLink, childLink, anchor);
      // joint->SetAnchor(0, anchor);
      joint->SetAxis(0, math::Vector3(1, 0, 0));
      joint->SetHighStop(0, upper);
      joint->SetLowStop(0, lower);

      if (parentLink)
        joint->SetName(parentLink->GetName() + std::string("_") +
                       childLink->GetName() + std::string("_joint"));
      else
        joint->SetName(std::string("world_") +
                       childLink->GetName() + std::string("_joint"));
      joint->Init();
    }

    double lastV = 0;
    double dt = world->GetPhysicsEngine()->GetMaxStepSize();
    for (unsigned int i = 0; i < 10; ++i)
    {
      double torque = 1.3;
      joint->SetForce(0, torque);
      world->StepWorld(1);
      double curV = joint->GetVelocity(0);
      double accel = (curV - lastV) / dt;
      gzdbg << i << " : " << curV << " : " << (curV - lastV) / dt << "\n";
      lastV = curV;
      EXPECT_NEAR(accel, torque / link->GetInertial()->GetIXX(), TOL);
    }

    // remove the joint
    {
      bool paused = world->IsPaused();
      world->SetPaused(true);
      if (joint)
      {
        // reenable collision between the link pair
        physics::LinkPtr parent = joint->GetParent();
        physics::LinkPtr child = joint->GetChild();
        if (parent)
          parent->SetCollideMode("all");
        if (child)
          child->SetCollideMode("all");

        joint->Detach();
        joint.reset();
      }
      world->SetPaused(paused);
    }
  }

  {
    // create a joint
    {
      joint = world->GetPhysicsEngine()->CreateJoint(
        "revolute", model);
      joint->Attach(parentLink, childLink);
      // load adds the joint to a vector of shared pointers kept
      // in parent and child links, preventing joint from being destroyed.
      joint->Load(parentLink, childLink, anchor);
      // joint->SetAnchor(0, anchor);
      joint->SetAxis(0, math::Vector3(0, 0, 1));
      joint->SetHighStop(0, upper);
      joint->SetLowStop(0, lower);

      if (parentLink)
        joint->SetName(parentLink->GetName() + std::string("_") +
                       childLink->GetName() + std::string("_joint"));
      else
        joint->SetName(std::string("world_") +
                       childLink->GetName() + std::string("_joint"));
      joint->Init();
    }

    double lastV = 0;
    double dt = world->GetPhysicsEngine()->GetMaxStepSize();
    for (unsigned int i = 0; i < 10; ++i)
    {
      double torque = 1.3;
      joint->SetForce(0, torque);
      world->StepWorld(1);
      double curV = joint->GetVelocity(0);
      double accel = (curV - lastV) / dt;
      gzdbg << i << " : " << curV << " : " << (curV - lastV) / dt << "\n";
      lastV = curV;
      EXPECT_NEAR(accel, torque / link->GetInertial()->GetIZZ(), TOL);
    }

    // remove the joint
    {
      bool paused = world->IsPaused();
      world->SetPaused(true);
      if (joint)
      {
        // reenable collision between the link pair
        physics::LinkPtr parent = joint->GetParent();
        physics::LinkPtr child = joint->GetChild();
        if (parent)
          parent->SetCollideMode("all");
        if (child)
          child->SetCollideMode("all");

        joint->Detach();
        joint.reset();
      }
      world->SetPaused(paused);
    }
  }
}

TEST_F(Joint_TEST, JointTorqueTestODE)
{
  JointTorqueTest("ode");
}

#ifdef HAVE_SIMBODY
TEST_F(Joint_TEST, JointTorqueTestSimbody)
{
  JointTorqueTest("simbody");
}
#endif  // HAVE_SIMBODY

#ifdef HAVE_BULLET
/// bullet collision parameters needs tweaking
TEST_F(Joint_TEST, JointTorqueTestBullet)
{
  gzerr << "JointTorqueTestBullet fails because dynamic joint manipulation "
        << "is not yet working\n";
  JointTorqueTest("bullet");
}
#endif  // HAVE_BULLET

void Joint_TEST::JointCreationDestructionTest(const std::string &_physicsEngine)
{
  // Load our inertial test world
  Load("worlds/joint_test.world", true, _physicsEngine);

  // Get a pointer to the world, make sure world loads
  physics::WorldPtr world = physics::get_world("default");
  ASSERT_TRUE(world != NULL);

  // Verify physics engine type
  physics::PhysicsEnginePtr physics = world->GetPhysicsEngine();
  ASSERT_TRUE(physics != NULL);
  EXPECT_EQ(physics->GetType(), "ode");

  // create some fake links
  physics::ModelPtr model = world->GetModel("model_1");
  ASSERT_TRUE(model != NULL);
  physics::LinkPtr link = model->GetLink("link_1");
  ASSERT_TRUE(link != NULL);

  physics::LinkPtr parentLink;
  physics::LinkPtr childLink(link);
  physics::JointPtr joint;
  math::Pose anchor;
  math::Vector3 axis(1, 0, 0);
  double upper = M_PI;
  double lower = -M_PI;

  double residentLast = 0, shareLast = 0;
  double residentCur = 0, shareCur = 0;

  for (unsigned int i = 0; i < 100; ++i)
  {
    // try creating a joint
    {
      joint = world->GetPhysicsEngine()->CreateJoint(
        "revolute", model);
      joint->Attach(parentLink, childLink);
      // load adds the joint to a vector of shared pointers kept
      // in parent and child links, preventing joint from being destroyed.
      joint->Load(parentLink, childLink, anchor);
      // joint->SetAnchor(0, anchor);
      joint->SetAxis(0, axis);
      joint->SetHighStop(0, upper);
      joint->SetLowStop(0, lower);

      if (parentLink)
        joint->SetName(parentLink->GetName() + std::string("_") +
                       childLink->GetName() + std::string("_joint"));
      else
        joint->SetName(std::string("world_") +
                       childLink->GetName() + std::string("_joint"));
      joint->Init();
      joint->SetAxis(0, axis);
    }
    // remove the joint
    {
      bool paused = world->IsPaused();
      world->SetPaused(true);
      if (joint)
      {
        // reenable collision between the link pair
        physics::LinkPtr parent = joint->GetParent();
        physics::LinkPtr child = joint->GetChild();
        if (parent)
          parent->SetCollideMode("all");
        if (child)
          child->SetCollideMode("all");

        joint->Detach();
        joint.reset();
      }
      world->SetPaused(paused);
    }

    gazebo::common::Time::MSleep(10);

    this->GetMemInfo(residentCur, shareCur);
    if (i > 1)  // give it 2 cycles to stabilize
    {
      EXPECT_LE(residentCur, residentLast);
      EXPECT_LE(shareCur, shareLast);
    }
    // gzdbg << "memory res[" << residentCur
    //       << "] shr[" << shareCur
    //       << "] res[" << residentLast
    //       << "] shr[" << shareLast
    //       << "]\n";
    residentLast = residentCur;
    shareLast = shareCur;
  }
}

TEST_F(Joint_TEST, JointCreationDestructionTestODE)
{
  JointCreationDestructionTest("ode");
}

#ifdef HAVE_SIMBODY
TEST_F(Joint_TEST, JointCreationDestructionTestSimbody)
{
  JointCreationDestructionTest("simbody");
}
#endif  // HAVE_SIMBODY

#ifdef HAVE_BULLET
/// bullet collision parameters needs tweaking
TEST_F(Joint_TEST, JointCreationDestructionTestBullet)
{
  /// \TODO: Disable for now until functionality is implemented
  // JointCreationDestructionTest("bullet");
  gzwarn << "JointCreationDestructionTest is disabled for Bullet\n";
}
#endif  // HAVE_BULLET

TEST_F(Joint_TEST, joint_SDF14)
{
  Load("worlds/SDF_1_4.world");

  physics::WorldPtr world = physics::get_world("default");
  ASSERT_TRUE(world != NULL);

  physics::PhysicsEnginePtr physics = world->GetPhysicsEngine();
  ASSERT_TRUE(physics != NULL);

  int i = 0;
  while (!this->HasEntity("joint14_model") && i < 20)
  {
    common::Time::MSleep(100);
    ++i;
  }

  if (i > 20)
    gzthrow("Unable to get joint14_model");

  physics::PhysicsEnginePtr physicsEngine = world->GetPhysicsEngine();
  EXPECT_TRUE(physicsEngine);
  physics::ModelPtr model = world->GetModel("joint14_model");
  EXPECT_TRUE(model);
  physics::LinkPtr link1 = model->GetLink("body1");
  EXPECT_TRUE(link1);
  physics::LinkPtr link2 = model->GetLink("body2");
  EXPECT_TRUE(link2);

  EXPECT_EQ(model->GetJointCount(), 1u);
  physics::JointPtr joint = model->GetJoint("joint14_revolute_joint");
  EXPECT_TRUE(joint);

  physics::LinkPtr parent = joint->GetParent();
  EXPECT_TRUE(parent);
  physics::LinkPtr child = joint->GetChild();
  EXPECT_TRUE(child);
  EXPECT_EQ(parent->GetName(), "body2");
  EXPECT_EQ(child->GetName(), "body1");
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
