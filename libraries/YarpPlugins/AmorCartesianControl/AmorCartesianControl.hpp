// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __AMOR_CARTESIAN_CONTROL_HPP__
#define __AMOR_CARTESIAN_CONTROL_HPP__

#include <yarp/os/Searchable.h>
#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/PolyDriver.h>

#include <amor.h>

#include "ICartesianControl.h"
#include "ICartesianSolver.h"

#define DEFAULT_CAN_LIBRARY "libeddriver.so"
#define DEFAULT_CAN_PORT 0

#define MAX_ANG_VEL 10.0

namespace roboticslab
{

/**
 * @ingroup YarpPlugins
 * @defgroup AmorCartesianControl
 * @brief Contains teo::AmorCartesianControl.
 */

/**
 * @ingroup AmorCartesianControl
 * @brief The AmorCartesianControl class implements ICartesianControl.
 *
 * Uses the roll-pitch-yaw (RPY) angle representation.
 */

class AmorCartesianControl : public yarp::dev::DeviceDriver, public ICartesianControl
{
public:

    AmorCartesianControl() : handle(AMOR_INVALID_HANDLE),
                             ownsHandle(false),
                             iCartesianSolver(NULL),
                             currentState(VOCAB_CC_NOT_CONTROLLING)
    {}

    // -- ICartesianControl declarations. Implementation in ICartesianControlImpl.cpp --

    virtual bool stat(int &state, std::vector<double> &x);

    virtual bool inv(const std::vector<double> &xd, std::vector<double> &q);

    virtual bool movj(const std::vector<double> &xd);

    virtual bool relj(const std::vector<double> &xd);

    virtual bool movl(const std::vector<double> &xd);

    virtual bool movv(const std::vector<double> &xdotd);

    virtual bool gcmp();

    virtual bool forc(const std::vector<double> &td);

    virtual bool stopControl();

    virtual bool tool(const std::vector<double> &x);

    virtual void fwd(const std::vector<double> &rot, double step);

    virtual void bkwd(const std::vector<double> &rot, double step);

    virtual void rot(const std::vector<double> &rot);

    virtual void pan(const std::vector<double> &transl);

    virtual void vmos(const std::vector<double> &xdot);

    virtual void eff(const std::vector<double> &xdotee);

    virtual void pose(const std::vector<double> &x, double interval);

    // -------- DeviceDriver declarations. Implementation in DeviceDriverImpl.cpp --------

    /**
    * Open the DeviceDriver.
    * @param config is a list of parameters for the device.
    * Which parameters are effective for your device can vary.
    * See \ref dev_examples "device invocation examples".
    * If there is no example for your device,
    * you can run the "yarpdev" program with the verbose flag
    * set to probe what parameters the device is checking.
    * If that fails too,
    * you'll need to read the source code (please nag one of the
    * yarp developers to add documentation for your device).
    * @return true/false upon success/failure
    */
    virtual bool open(yarp::os::Searchable& config);

    /**
    * Close the DeviceDriver.
    * @return true/false on success/failure.
    */
    virtual bool close();

private:

    bool waitForCompletion(int vocab);

    bool checkJointVelocities(const std::vector<double> &qdot);

    AMOR_HANDLE handle;
    bool ownsHandle;
    
    yarp::dev::PolyDriver cartesianDevice;
    roboticslab::ICartesianSolver *iCartesianSolver;

    int currentState;
};

}  // namespace roboticslab

#endif  // __AMOR_CARTESIAN_CONTROL_HPP__