// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "BasicCartesianControl.hpp"

#include <cmath>

#include <algorithm>

#include <yarp/os/Bottle.h>
#include <yarp/os/Vocab.h>

#include <ColorDebug.h>

using namespace roboticslab;

// -----------------------------------------------------------------------------

namespace
{
    double epsilon = 1e-5;

    // return -1 for negative numbers, +1 for positive numbers, 0 for zero
    // https://stackoverflow.com/a/4609795
    template <typename T>
    inline int sgn(T val)
    {
        return (T(0) < val) - (val < T(0));
    }
}

// -----------------------------------------------------------------------------

int BasicCartesianControl::getCurrentState() const
{
    std::lock_guard<std::mutex> lock(stateMutex);
    return currentState;
}

// -----------------------------------------------------------------------------

void BasicCartesianControl::setCurrentState(int value)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    currentState = value;
    streamingCommand = VOCAB_CC_NOT_SET;
}

// -----------------------------------------------------------------------------

bool BasicCartesianControl::checkJointLimits(const std::vector<double> &q)
{
    for (unsigned int joint = 0; joint < numSolverJoints; joint++)
    {
        double value = q[joint];

        // Report limit before reaching the actual value.
        // https://github.com/roboticslab-uc3m/kinematics-dynamics/issues/161#issuecomment-428133287
        if (value < qMin[joint] + epsilon || value > qMax[joint] - epsilon)
        {
            CD_WARNING("Joint near or out of limits: q[%d] = %f not in [%f,%f] (def).\n",
                    joint, value, qMin[joint], qMax[joint]);
            return false;
        }
    }

    return true;
}

// -----------------------------------------------------------------------------

bool BasicCartesianControl::checkJointLimits(const std::vector<double> &q, const std::vector<double> &qdot)
{
    for (unsigned int joint = 0; joint < numSolverJoints; joint++)
    {
        double value = q[joint];

        if (value < qMin[joint] + epsilon || value > qMax[joint] - epsilon)
        {
            CD_WARNING("Joint near or out of limits: q[%d] = %f not in [%f,%f] (deg).\n",
                    joint, value, qMin[joint], qMax[joint]);
            double midRange = (qMax[joint] + qMin[joint]) / 2;

            // Let the joint get away from its nearest limit.
            if (sgn(value - midRange) == sgn(qdot[joint]))
            {
                return false;
            }
        }
    }

    return true;
}

// -----------------------------------------------------------------------------

bool BasicCartesianControl::checkJointVelocities(const std::vector<double> &qdot)
{
    for (unsigned int joint = 0; joint < numSolverJoints; joint++)
    {
        double value = qdot[joint];

        if (value < qdotMin[joint] || value > qdotMax[joint])
        {
            CD_WARNING("Maximum angular velocity hit: qdot[%d] = %f not in [%f,%f] (deg/s).\n",
                    joint, value, qdotMin[joint], qdotMax[joint]);
            return false;
        }
    }

    return true;
}

// -----------------------------------------------------------------------------

bool BasicCartesianControl::checkControlModes(int mode)
{
    std::vector<int> modes(numRobotJoints);

    if (!iControlMode->getControlModes(modes.data()))
    {
        CD_WARNING("getControlModes failed.\n");
        return false;
    }

    return std::all_of(modes.begin(), modes.end(), [mode](int retrievedMode) { return retrievedMode == mode; });
}

// -----------------------------------------------------------------------------

bool BasicCartesianControl::setControlModes(int mode)
{
    std::vector<int> modes(numRobotJoints);

    if (!iControlMode->getControlModes(modes.data()))
    {
        CD_WARNING("getControlModes failed.\n");
        return false;
    }

    std::vector<int> jointIds;

    for (unsigned int i = 0; i < modes.size(); i++)
    {
        if (modes[i] != mode)
        {
            jointIds.push_back(i);
        }
    }

    if (!jointIds.empty())
    {
        modes.assign(jointIds.size(), mode);

        if (!iControlMode->setControlModes(jointIds.size(), jointIds.data(), modes.data()))
        {
            CD_WARNING("setControlModes failed (%s).\n", yarp::os::Vocab::decode(mode).c_str());
            return false;
        }
    }

    return true;
}

// -----------------------------------------------------------------------------

bool BasicCartesianControl::presetStreamingCommand(int command)
{
    setCurrentState(VOCAB_CC_NOT_CONTROLLING);

    switch (command)
    {
    case VOCAB_CC_TWIST:
    case VOCAB_CC_POSE:
        return setControlModes(VOCAB_CM_VELOCITY);
    case VOCAB_CC_MOVI:
        return setControlModes(VOCAB_CM_POSITION_DIRECT);
    default:
        CD_ERROR("Unrecognized or unsupported streaming command vocab.\n");
    }

    return false;
}

// -----------------------------------------------------------------------------

void BasicCartesianControl::computeIsocronousSpeeds(const std::vector<double> & q, const std::vector<double> & qd,
        std::vector<double> & qdot)
{
    double maxTime = 0.0;

    //-- Find out the maximum time to move

    for (int joint = 0; joint < numSolverJoints; joint++)
    {
        if (qRefSpeeds[joint] <= 0.0)
        {
            CD_WARNING("Zero or negative velocities sent at joint %d, not moving: %f.\n", joint, qRefSpeeds[joint]);
            return;
        }

        double distance = std::abs(qd[joint] - q[joint]);

        CD_INFO("Distance (joint %d): %f\n", joint, distance);

        double targetTime = distance / qRefSpeeds[joint];

        if (targetTime > maxTime)
        {
            maxTime = targetTime;
            CD_INFO("Candidate: %f\n", maxTime);
        }
    }

    //-- Compute, store old and set joint velocities given this time

    for (int joint = 0; joint < numRobotJoints; joint++)
    {
        if (joint >= numSolverJoints)
        {
            CD_INFO("qdot[%d] = 0.0 (forced)\n", joint);
        }
        else
        {
            qdot[joint] = std::abs(qd[joint] - q[joint]) / maxTime;
            CD_INFO("qdot[%d] = %f\n", joint, qdot[joint]);
        }
    }
}

// -----------------------------------------------------------------------------
