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
/* Desc: The ODE base joint class
 * Author: Nate Koenig, Andrew Howard
 * Date: 12 Oct 2009
 */

#include "common/Exception.hh"
#include "common/Console.hh"

#include "gazebo/physics/World.hh"
#include "gazebo/physics/Link.hh"
#include "gazebo/physics/PhysicsEngine.hh"
#include "gazebo/physics/ode/ODELink.hh"
#include "gazebo/physics/ode/ODEJoint.hh"
#include "gazebo/physics/ScrewJoint.hh"
#include "gazebo/physics/JointWrench.hh"

using namespace gazebo;
using namespace physics;


//////////////////////////////////////////////////
ODEJoint::ODEJoint(BasePtr _parent)
  : Joint(_parent)
{
  this->jointId = NULL;
  this->cfmDampingState[0] = 0;
  this->cfmDampingState[1] = 0;
}

//////////////////////////////////////////////////
ODEJoint::~ODEJoint()
{
  this->Detach();
  dJointDestroy(this->jointId);
}

//////////////////////////////////////////////////
void ODEJoint::Load(sdf::ElementPtr _sdf)
{
  Joint::Load(_sdf);

  if (this->sdf->HasElement("physics") &&
      this->sdf->GetElement("physics")->HasElement("ode"))
  {
    sdf::ElementPtr elem = this->sdf->GetElement("physics")->GetElement("ode");

    if (elem->HasElement("provide_feedback"))
    {
      this->provideFeedback = elem->GetValueBool("provide_feedback");
    }

    if (elem->HasElement("limit"))
    {
      this->SetParam(dParamStopERP,
          elem->GetElement("limit")->GetValueDouble("erp"));
      this->SetParam(dParamStopCFM,
          elem->GetElement("limit")->GetValueDouble("cfm"));
    }

    if (elem->HasElement("suspension"))
    {
      this->SetParam(dParamSuspensionERP,
          elem->GetElement("suspension")->GetValueDouble("erp"));
      this->SetParam(dParamSuspensionCFM,
          elem->GetElement("suspension")->GetValueDouble("cfm"));
    }

    if (elem->HasElement("fudge_factor"))
      this->SetParam(dParamFudgeFactor,
          elem->GetElement("fudge_factor")->GetValueDouble());

    if (elem->HasElement("cfm"))
        this->SetParam(dParamCFM, elem->GetElement("cfm")->GetValueDouble());

    if (elem->HasElement("bounce"))
        this->SetParam(dParamBounce,
          elem->GetElement("bounce")->GetValueDouble());

    if (elem->HasElement("max_force"))
      this->SetParam(dParamFMax,
          elem->GetElement("max_force")->GetValueDouble());

    if (elem->HasElement("velocity"))
      this->SetParam(dParamVel,
          elem->GetElement("velocity")->GetValueDouble());
  }

  if (this->sdf->HasElement("axis"))
  {
    sdf::ElementPtr axisElem = this->sdf->GetElement("axis");
    if (axisElem->HasElement("dynamics"))
    {
      sdf::ElementPtr dynamicsElem = axisElem->GetElement("dynamics");

      if (dynamicsElem->HasElement("damping"))
      {
        this->SetDamping(0, dynamicsElem->GetValueDouble("damping"));
      }
      if (dynamicsElem->HasElement("friction"))
      {
        sdf::ElementPtr frictionElem = dynamicsElem->GetElement("friction");
        gzlog << "joint friction not implemented\n";
      }
    }
  }


  if (this->provideFeedback)
  {
    this->feedback = new dJointFeedback;
    dJointSetFeedback(this->jointId, this->feedback);
  }
}

//////////////////////////////////////////////////
LinkPtr ODEJoint::GetJointLink(int _index) const
{
  LinkPtr result;

  if (_index == 0 || _index == 1)
  {
    ODELinkPtr odeLink1 = boost::shared_static_cast<ODELink>(this->childLink);
    ODELinkPtr odeLink2 = boost::shared_static_cast<ODELink>(this->parentLink);
    if (odeLink1 != NULL &&
        dJointGetBody(this->jointId, _index) == odeLink1->GetODEId())
      result = this->childLink;
    else if (odeLink2)
      result = this->parentLink;
  }

  return result;
}

//////////////////////////////////////////////////
bool ODEJoint::AreConnected(LinkPtr _one, LinkPtr _two) const
{
  ODELinkPtr odeLink1 = boost::shared_dynamic_cast<ODELink>(_one);
  ODELinkPtr odeLink2 = boost::shared_dynamic_cast<ODELink>(_two);

  if (odeLink1 == NULL || odeLink2 == NULL)
    gzthrow("ODEJoint requires ODE bodies\n");

  return dAreConnected(odeLink1->GetODEId(), odeLink2->GetODEId());
}

//////////////////////////////////////////////////
// child classes where appropriate
double ODEJoint::GetParam(int /*parameter*/) const
{
  return 0;
}

//////////////////////////////////////////////////
void ODEJoint::Attach(LinkPtr _parent, LinkPtr _child)
{
  Joint::Attach(_parent, _child);

  ODELinkPtr odechild = boost::shared_dynamic_cast<ODELink>(this->childLink);
  ODELinkPtr odeparent = boost::shared_dynamic_cast<ODELink>(this->parentLink);

  if (odechild == NULL && odeparent == NULL)
    gzthrow("ODEJoint requires at least one ODE link\n");


  if (!odechild && odeparent)
  {
    dJointAttach(this->jointId, 0, odeparent->GetODEId());
  }
  else if (odechild && !odeparent)
  {
    dJointAttach(this->jointId, odechild->GetODEId(), 0);
  }
  else if (odechild && odeparent)
  {
    if (this->HasType(Base::HINGE2_JOINT))
      dJointAttach(this->jointId, odeparent->GetODEId(), odechild->GetODEId());
    else
      dJointAttach(this->jointId, odechild->GetODEId(), odeparent->GetODEId());
  }
}

//////////////////////////////////////////////////
void ODEJoint::Detach()
{
  this->childLink.reset();
  this->parentLink.reset();
  dJointAttach(this->jointId, 0, 0);
}

//////////////////////////////////////////////////
// where appropriate
void ODEJoint::SetParam(int /*parameter*/, double /*value*/)
{
  if (this->childLink)
    this->childLink->SetEnabled(true);
  if (this->parentLink)
    this->parentLink->SetEnabled(true);
}

//////////////////////////////////////////////////
void ODEJoint::SetERP(double _newERP)
{
  this->SetParam(dParamSuspensionERP, _newERP);
}

//////////////////////////////////////////////////
double ODEJoint::GetERP()
{
  return this->GetParam(dParamSuspensionERP);
}

//////////////////////////////////////////////////
void ODEJoint::SetCFM(double _newCFM)
{
  this->SetParam(dParamSuspensionCFM, _newCFM);
}

//////////////////////////////////////////////////
double ODEJoint::GetCFM()
{
  return this->GetParam(dParamSuspensionCFM);
}

//////////////////////////////////////////////////
dJointFeedback *ODEJoint::GetFeedback()
{
  return dJointGetFeedback(this->jointId);
}

//////////////////////////////////////////////////
void ODEJoint::SetHighStop(int _index, const math::Angle &_angle)
{
  Joint::SetHighStop(_index, _angle);
  switch (_index)
  {
    case 0:
      this->SetParam(dParamHiStop, _angle.Radian());
      break;
    case 1:
      this->SetParam(dParamHiStop2, _angle.Radian());
      break;
    case 2:
      this->SetParam(dParamHiStop3, _angle.Radian());
      break;
    default:
      gzerr << "Invalid index[" << _index << "]\n";
      break;
  };
}

//////////////////////////////////////////////////
void ODEJoint::SetLowStop(int _index, const math::Angle &_angle)
{
  Joint::SetLowStop(_index, _angle);
  switch (_index)
  {
    case 0:
      this->SetParam(dParamLoStop, _angle.Radian());
      break;
    case 1:
      this->SetParam(dParamLoStop2, _angle.Radian());
      break;
    case 2:
      this->SetParam(dParamLoStop3, _angle.Radian());
      break;
    default:
      gzerr << "Invalid index[" << _index << "]\n";
  };
}

//////////////////////////////////////////////////
math::Angle ODEJoint::GetHighStop(int _index)
{
  switch (_index)
  {
    case 0:
      return this->GetParam(dParamHiStop);
    case 1:
      return this->GetParam(dParamHiStop2);
    case 2:
      return this->GetParam(dParamHiStop3);
    default:
      gzerr << "Invalid index[" << _index << "]\n";
  };

  return 0;
}

//////////////////////////////////////////////////
math::Angle ODEJoint::GetLowStop(int _index)
{
  switch (_index)
  {
    case 0:
      return this->GetParam(dParamLoStop);
    case 1:
      return this->GetParam(dParamLoStop2);
    case 2:
      return this->GetParam(dParamLoStop3);
    default:
      gzerr << "Invalid index[" << _index << "]\n";
  };

  return 0;
}

//////////////////////////////////////////////////
math::Vector3 ODEJoint::GetLinkForce(unsigned int _index) const
{
  math::Vector3 result;
  dJointFeedback *jointFeedback = dJointGetFeedback(this->jointId);

  if (_index == 0)
    result.Set(jointFeedback->f1[0], jointFeedback->f1[1],
               jointFeedback->f1[2]);
  else
    result.Set(jointFeedback->f2[0], jointFeedback->f2[1],
               jointFeedback->f2[2]);

  return result;
}

//////////////////////////////////////////////////
math::Vector3 ODEJoint::GetLinkTorque(unsigned int _index) const
{
  math::Vector3 result;
  dJointFeedback *jointFeedback = dJointGetFeedback(this->jointId);

  if (_index == 0)
    result.Set(jointFeedback->t1[0], jointFeedback->t1[1],
               jointFeedback->t1[2]);
  else
    result.Set(jointFeedback->t2[0], jointFeedback->t2[1],
               jointFeedback->t2[2]);

  return result;
}

//////////////////////////////////////////////////
void ODEJoint::SetAttribute(Attribute _attr, int /*_index*/, double _value)
{
  switch (_attr)
  {
    case FUDGE_FACTOR:
      this->SetParam(dParamFudgeFactor, _value);
      break;
    case SUSPENSION_ERP:
      this->SetParam(dParamSuspensionERP, _value);
      break;
    case SUSPENSION_CFM:
      this->SetParam(dParamSuspensionCFM, _value);
      break;
    case STOP_ERP:
      this->SetParam(dParamStopERP, _value);
      break;
    case STOP_CFM:
      this->SetParam(dParamStopCFM, _value);
      break;
    case ERP:
      this->SetParam(dParamERP, _value);
      break;
    case CFM:
      this->SetParam(dParamCFM, _value);
      break;
    case FMAX:
      this->SetParam(dParamFMax, _value);
      break;
    case VEL:
      this->SetParam(dParamVel, _value);
      break;
    case HI_STOP:
      this->SetParam(dParamHiStop, _value);
      break;
    case LO_STOP:
      this->SetParam(dParamLoStop, _value);
      break;
    default:
      gzerr << "Unable to handle joint attribute[" << _attr << "]\n";
      break;
  };
}

//////////////////////////////////////////////////
void ODEJoint::SetAttribute(const std::string &_key, int /*_index*/,
                            const boost::any &_value)
{
  if (_key == "fudge_factor")
  {
    try
    {
      this->SetParam(dParamFudgeFactor, boost::any_cast<double>(_value));
    }
    catch(boost::bad_any_cast &e)
    {
      gzerr << "boost any_cast error:" << e.what() << "\n";
    }
  }
  else if (_key == "suspension_erp")
  {
    try
    {
      this->SetParam(dParamSuspensionERP, boost::any_cast<double>(_value));
    }
    catch(boost::bad_any_cast &e)
    {
      gzerr << "boost any_cast error:" << e.what() << "\n";
    }
  }
  else if (_key == "suspension_cfm")
  {
    try
    {
      this->SetParam(dParamSuspensionCFM, boost::any_cast<double>(_value));
    }
    catch(boost::bad_any_cast &e)
    {
      gzerr << "boost any_cast error:" << e.what() << "\n";
    }
  }
  else if (_key == "stop_erp")
  {
    try
    {
      this->SetParam(dParamStopERP, boost::any_cast<double>(_value));
    }
    catch(boost::bad_any_cast &e)
    {
      gzerr << "boost any_cast error:" << e.what() << "\n";
    }
  }
  else if (_key == "stop_cfm")
  {
    try
    {
      this->SetParam(dParamStopCFM, boost::any_cast<double>(_value));
    }
    catch(boost::bad_any_cast &e)
    {
      gzerr << "boost any_cast error:" << e.what() << "\n";
    }
  }
  else if (_key == "erp")
  {
    try
    {
      this->SetParam(dParamERP, boost::any_cast<double>(_value));
    }
    catch(boost::bad_any_cast &e)
    {
      gzerr << "boost any_cast error:" << e.what() << "\n";
    }
  }
  else if (_key == "cfm")
  {
    try
    {
      this->SetParam(dParamCFM, boost::any_cast<double>(_value));
    }
    catch(boost::bad_any_cast &e)
    {
      gzerr << "boost any_cast error:" << e.what() << "\n";
    }
  }
  else if (_key == "fmax")
  {
    try
    {
      this->SetParam(dParamFMax, boost::any_cast<double>(_value));
    }
    catch(boost::bad_any_cast &e)
    {
      gzerr << "boost any_cast error:" << e.what() << "\n";
    }
  }
  else if (_key == "vel")
  {
    try
    {
      this->SetParam(dParamVel, boost::any_cast<double>(_value));
    }
    catch(boost::bad_any_cast &e)
    {
      gzerr << "boost any_cast error:" << e.what() << "\n";
    }
  }
  else if (_key == "hi_stop")
  {
    try
    {
      this->SetParam(dParamHiStop, boost::any_cast<double>(_value));
    }
    catch(boost::bad_any_cast &e)
    {
      gzerr << "boost any_cast error:" << e.what() << "\n";
    }
  }
  else if (_key == "lo_stop")
  {
    try
    {
      this->SetParam(dParamLoStop, boost::any_cast<double>(_value));
    }
    catch(boost::bad_any_cast &e)
    {
      gzerr << "boost any_cast error:" << e.what() << "\n";
    }
  }
  else if (_key == "thread_pitch")
  {
    ScrewJoint<ODEJoint>* screwJoint =
      dynamic_cast<ScrewJoint<ODEJoint>* >(this);
    if (screwJoint != NULL)
    {
      try
      {
        screwJoint->SetThreadPitch(0, boost::any_cast<double>(_value));
      }
      catch(boost::bad_any_cast &e)
      {
        gzerr << "boost any_cast error:" << e.what() << "\n";
      }
    }
  }
  else
  {
    try
    {
      gzerr << "Unable to handle joint attribute["
            << boost::any_cast<std::string>(_value) << "]\n";
    }
    catch(boost::bad_any_cast &e)
    {
      gzerr << "boost any_cast error:" << e.what() << "\n";
    }
  }
}

//////////////////////////////////////////////////
void ODEJoint::Reset()
{
  dJointReset(this->jointId);
  Joint::Reset();
}

//////////////////////////////////////////////////
JointWrench ODEJoint::GetForceTorque(int /*_index*/)
{
  JointWrench wrench;
  // Note that:
  // f2, t2 are the force torque measured on parent body's cg
  // f1, t1 are the force torque measured on child body's cg
  dJointFeedback* fb = this->GetFeedback();
  if (fb)
  {
    wrench.body1Force.Set(fb->f1[0], fb->f1[1], fb->f1[2]);
    wrench.body1Torque.Set(fb->t1[0], fb->t1[1], fb->t1[2]);
    wrench.body2Force.Set(fb->f2[0], fb->f2[1], fb->f2[2]);
    wrench.body2Torque.Set(fb->t2[0], fb->t2[1], fb->t2[2]);

    if (this->childLink)
    {
      // convert torque from about child CG to joint anchor location
      // cg position specified in child link frame
      math::Vector3 cgPos = this->childLink->GetInertial()->GetPose().pos;
      // moment arm rotated into world frame (given feedback is in world frame)
      math::Vector3 childMomentArm =
        this->childLink->GetWorldPose().rot.RotateVector(
        this->anchorPos - cgPos);

      // gzerr << "anchor [" << anchorPos
      //       << "] iarm[" << this->childLink->GetInertial()->GetPose().pos
      //       << "] childMomentArm[" << childMomentArm
      //       << "] f1[" << wrench.body1Force
      //       << "] t1[" << wrench.body1Torque
      //       << "] fxp[" << wrench.body1Force.Cross(childMomentArm)
      //       << "]\n";

      wrench.body1Torque += wrench.body1Force.Cross(childMomentArm);
    }

    // convert torque from about parent CG to joint anchor location
    if (this->parentLink)
    {
      // parent cg specified in child link frame
      math::Vector3 cgPos = ((this->parentLink->GetInertial()->GetPose() +
                            this->parentLink->GetWorldPose()) -
                            this->childLink->GetWorldPose()).pos;

      // rotate moement arms into world frame
      math::Vector3 parentMomentArm =
        this->childLink->GetWorldPose().rot.RotateVector(
        this->anchorPos - cgPos);

      wrench.body2Torque -= wrench.body2Force.Cross(parentMomentArm);

      // A good check is that
      // the computed body2Torque shoud in fact be opposite of body1Torque
    }
    else
    {
      // convert torque from about child CG to joint anchor location
      // or simply use equal opposite force as body1 wrench
      wrench.body2Force = -wrench.body1Force;
      wrench.body2Torque = -wrench.body1Torque;
    }
  }
  else
  {
    // forgot to set provide_feedback?
    gzwarn << "GetForceTorque: forget to set <provide_feedback>?\n";
  }

  return wrench;
}

//////////////////////////////////////////////////
void ODEJoint::CFMDamping()
{
  // check if we are violating joint limits
  for (unsigned int i = 0; i < this->GetAngleCount(); ++i)
  {
    if (this->cfmDampingState[i] != 1 &&
        (this->GetAngle(i) >= this->upperLimit[i].Radian() ||
         this->GetAngle(i) <= this->lowerLimit[i].Radian() ||
         math::equal(this->dampingCoefficient, 0.0)))
    {
      this->cfmDampingState[i] = 1;
      // no damping and use hard stop cfm & erp
      this->SetAttribute("stop_erp", i, 0.2);
      this->SetAttribute("stop_cfm", i, 0.0);
      this->SetHighStop(i, this->upperLimit[i].Radian());
      this->SetLowStop(i, this->lowerLimit[i].Radian());
      this->SetHighStop(i, this->upperLimit[i].Radian());
    }
    else if (this->cfmDampingState[i] != 2 &&
         !math::equal(this->dampingCoefficient, 0.0))
    {
      this->cfmDampingState[i] = 2;
      this->SetHighStop(i, 0.0);
      this->SetLowStop(i, 0.0);
      this->SetHighStop(i, 0.0);
      this->SetAttribute("stop_erp", i, 0.0);
      this->SetAttribute("stop_cfm", i, 1.0 / this->dampingCoefficient);
    }
  }
}

