# Copyright: (C) 2010 Universidad Carlos III de Madrid
# Copyright: (C) 2009 RobotCub Consortium
# Author: Juan G. Victores
# Contrib: Ugo; Paul Fitzpatrick (YARP device examples and tutorials)
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

yarp_prepare_plugin(CartesianControlServer
                    CATEGORY device
                    TYPE roboticslab::CartesianControlServer
                    INCLUDE CartesianControlServer.hpp
                    EXTRA_CONFIG WRAPPER=CartesianControlServer
                    DEFAULT ON
                    DEPENDS ENABLE_KinematicRepresentationLib)

if(NOT SKIP_CartesianControlServer)

    if(NOT YARP_VERSION VERSION_GREATER_EQUAL 3.4)
        set(CMAKE_INCLUDE_CURRENT_DIR TRUE) # yarp plugin builder needs this
    endif()

    yarp_add_plugin(CartesianControlServer CartesianControlServer.hpp
                                           DeviceDriverImpl.cpp
                                           PeriodicThreadImpl.cpp
                                           RpcResponder.cpp
                                           StreamResponder.cpp)

    target_link_libraries(CartesianControlServer YARP::YARP_OS
                                                 YARP::YARP_dev
                                                 ROBOTICSLAB::ColorDebug
                                                 KinematicRepresentationLib
                                                 KinematicsDynamicsInterfaces)

    yarp_install(TARGETS CartesianControlServer
                 LIBRARY DESTINATION ${ROBOTICSLAB-KINEMATICS-DYNAMICS_DYNAMIC_PLUGINS_INSTALL_DIR}
                 ARCHIVE DESTINATION ${ROBOTICSLAB-KINEMATICS-DYNAMICS_STATIC_PLUGINS_INSTALL_DIR}
                 YARP_INI DESTINATION ${ROBOTICSLAB-KINEMATICS-DYNAMICS_PLUGIN_MANIFESTS_INSTALL_DIR})

endif()
