// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#ifndef __X_RPC_RESPONDER__
#define __X_RPC_RESPONDER__

#include <yarp/os/Port.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Time.h>

#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/CartesianControl.h>
#include <yarp/dev/ControlBoardInterfaces.h>

#include "CartesianRateThread.hpp"

#define MAX_NUM_MOTORS 100

#define VOCAB_HELP VOCAB4('h','e','l','p')
#define VOCAB_LOAD VOCAB4('l','o','a','d')

using namespace yarp::os;
using namespace yarp::dev;
using yarp::sig::Vector;

namespace teo
{

/**
 * @ingroup teoCartesianServer
 *
 * @brief Implements an xRpcPort responder (callback on RPC).
 */
class TeoXRpcResponder : public PortReader {

    protected:
        /**
        * Implement the actual responder (callback on RPC).
        */
        virtual bool read(ConnectionReader& connection);

        CartesianRateThread *cartesianRateThread;

    public:

        void setCartesianRateThread(CartesianRateThread* cartesianRateThread) {
            this->cartesianRateThread = cartesianRateThread;
        }

};

}  // namespace teo

#endif

