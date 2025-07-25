// This file is part of RaiSim. You must obtain a valid license from RaiSim Tech
// Inc. prior to usage.

#include "raisim/RaisimServer.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
  auto binaryPath = raisim::Path::setFromArgv(argv[0]);
  raisim::World::setActivationKey(binaryPath.getDirectory() + "\\rsc\\activation.raisim");
  raisim::RaiSimMsg::setFatalCallback([](){throw;});

  /// create raisim world
  raisim::World world;
  world.setTimeStep(0.001);

  /// create objects
  auto ground = world.addGround(0, "gnd");
  ground->setAppearance("hidden");
  auto anymalB = world.addArticulatedSystem(binaryPath.getDirectory() + "\\rsc\\anymal\\urdf\\anymal.urdf");
  auto anymalC = world.addArticulatedSystem(binaryPath.getDirectory() + "\\rsc\\anymal_c\\urdf\\anymal_sensored.urdf");

  /// anymalC joint PD controller
  Eigen::VectorXd jointNominalConfig(anymalC->getGeneralizedCoordinateDim()), jointVelocityTarget(anymalC->getDOF());
  jointNominalConfig << 0, 0, 0.54, 1.0, 0.0, 0.0, 0.0, 0.03, 0.4, -0.8, -0.03, 0.4, -0.8, 0.03, -0.4, 0.8, -0.03, -0.4, 0.8;
  jointVelocityTarget.setZero();

  Eigen::VectorXd jointPgain(anymalC->getDOF()), jointDgain(anymalC->getDOF());
  jointPgain.tail(12).setConstant(100.0);
  jointDgain.tail(12).setConstant(1.0);

  anymalC->setGeneralizedCoordinate(jointNominalConfig);
  anymalC->setGeneralizedForce(Eigen::VectorXd::Zero(anymalC->getDOF()));
  anymalC->setPdGains(jointPgain, jointDgain);
  anymalC->setPdTarget(jointNominalConfig, jointVelocityTarget);
  anymalC->setName("anymalC");

  jointNominalConfig[1] = 1.0;
  anymalB->setGeneralizedCoordinate(jointNominalConfig);
  anymalB->setGeneralizedForce(Eigen::VectorXd::Zero(anymalB->getDOF()));
  anymalB->setPdGains(jointPgain, jointDgain);
  anymalB->setPdTarget(jointNominalConfig, jointVelocityTarget);
  anymalB->setName("anymalB");

  /// Setup cameras and sensors for anymalC
  // Get front camera sensors
  auto front_depthSensor = anymalC->getSensorSet("depth_camera_front_camera_parent")->getSensor<raisim::DepthCamera>("depth");
  front_depthSensor->setMeasurementSource(raisim::Sensor::MeasurementSource::VISUALIZER);
  
  auto front_rgbCamera = anymalC->getSensorSet("depth_camera_front_camera_parent")->getSensor<raisim::RGBCamera>("color");
  front_rgbCamera->setMeasurementSource(raisim::Sensor::MeasurementSource::VISUALIZER);
  
  // Get rear camera sensors  
  auto rear_depthSensor = anymalC->getSensorSet("depth_camera_rear_camera_parent")->getSensor<raisim::DepthCamera>("depth");
  rear_depthSensor->setMeasurementSource(raisim::Sensor::MeasurementSource::VISUALIZER);
  
  auto rear_rgbCamera = anymalC->getSensorSet("depth_camera_rear_camera_parent")->getSensor<raisim::RGBCamera>("color");
  rear_rgbCamera->setMeasurementSource(raisim::Sensor::MeasurementSource::VISUALIZER);

  /// friction example. uncomment it to see the effect
//  anymalB->getCollisionBody("LF_FOOT/0").setMaterial("LF_FOOT");
//  world.setMaterialPairProp("gnd", "LF_FOOT", 0.01, 0, 0);

  /// launch raisim server
  raisim::RaisimServer server(&world);
  server.setMap("wheat");
  server.launchServer();
  server.focusOn(anymalC);

  /// graphs
  std::vector<std::string> jointNames = {"LF_HAA", "LF_HFE", "LF_KFE", "RF_HAA", "RF_HFE", "RF_KFE",
                                         "LH_HAA", "LH_HFE", "LH_KFE", "RH_HAA", "RH_HFE", "RH_KFE"};
  auto jcGraph = server.addTimeSeriesGraph("joint position", jointNames, "time", "position");
  auto jvGraph = server.addTimeSeriesGraph("joint velocity", jointNames, "time", "velocity");
  auto jfGraph = server.addTimeSeriesGraph("joint torque", jointNames, "time", "torque");

  // Declare variables for linear acceleration, angular velocity, and orientation
  raisim::Vec<3> linear_accel, angular_vel;
  raisim::Vec<4> orientation;
  
  raisim::VecDyn jc(12), jv(12), jf(12);

  for (int i=0; i<200000000; i++) {
    RS_TIMED_LOOP(int(world.getTimeStep()*1e6))
    server.integrateWorldThreadSafe();
    if (i % 10 == 0) {
      // Update robot state variables using generalized coordinates and velocities
      auto gc = anymalC->getGeneralizedCoordinate();
      auto gv = anymalC->getGeneralizedVelocity();
      auto gf = anymalC->getGeneralizedForce();
      
      // Base position is gc[0:3], quaternion is gc[3:7], angular velocity is gv[3:6]
      raisim::Vec<3> base_pos = {gc[0], gc[1], gc[2]};
      orientation = {gc[3], gc[4], gc[5], gc[6]}; // quaternion (w, x, y, z)
      angular_vel = {gv[3], gv[4], gv[5]};
      
      // For linear acceleration, we'll use a simple finite difference approximation
      static raisim::Vec<3> prev_vel = {gv[0], gv[1], gv[2]};
      static double prev_time = world.getWorldTime();
      double dt = world.getWorldTime() - prev_time;
      if (dt > 0) {
        linear_accel = {(gv[0] - prev_vel[0])/dt, (gv[1] - prev_vel[1])/dt, (gv[2] - prev_vel[2])/dt};
      }
      prev_vel = {gv[0], gv[1], gv[2]};
      prev_time = world.getWorldTime();
      
      jc = anymalC->getGeneralizedCoordinate().e().tail(12);
      jv = anymalC->getGeneralizedVelocity().e().tail(12);
      jf = anymalC->getGeneralizedForce().e().tail(12);
      jcGraph->addDataPoints(world.getWorldTime(), jc);
      jvGraph->addDataPoints(world.getWorldTime(), jv);
      jfGraph->addDataPoints(world.getWorldTime(), jf);
      
      // Output the robot state information
      if (i % 1000 == 0) { // Print every 1000th iteration to avoid too much output
        std::cout << "Time: " << world.getWorldTime() << "s" << std::endl;
        std::cout << "Linear Acceleration: [" << linear_accel[0] << ", " << linear_accel[1] << ", " << linear_accel[2] << "]" << std::endl;
        std::cout << "Angular Velocity: [" << angular_vel[0] << ", " << angular_vel[1] << ", " << angular_vel[2] << "]" << std::endl;
        std::cout << "Orientation (Quaternion): [" << orientation[0] << ", " << orientation[1] << ", " << orientation[2] << ", " << orientation[3] << "]" << std::endl;
        
        // Display camera information
        std::cout << "Camera Information:" << std::endl;
        
        // Front RGB camera
        front_rgbCamera->lockMutex();
        const auto& frontRgbBuffer = front_rgbCamera->getImageBuffer();
        std::cout << "  Front RGB Camera: " << frontRgbBuffer.size() << " bytes" << std::endl;
        front_rgbCamera->unlockMutex();
        
        // Front depth camera  
        front_depthSensor->lockMutex();
        const auto& frontDepthArray = front_depthSensor->getDepthArray();
        std::cout << "  Front Depth Camera: " << frontDepthArray.size() << " depth values" << std::endl;
        front_depthSensor->unlockMutex();
        
        // Rear RGB camera
        rear_rgbCamera->lockMutex();
        const auto& rearRgbBuffer = rear_rgbCamera->getImageBuffer();
        std::cout << "  Rear RGB Camera: " << rearRgbBuffer.size() << " bytes" << std::endl;
        rear_rgbCamera->unlockMutex();
        
        // Rear depth camera
        rear_depthSensor->lockMutex();
        const auto& rearDepthArray = rear_depthSensor->getDepthArray();
        std::cout << "  Rear Depth Camera: " << rearDepthArray.size() << " depth values" << std::endl;
        rear_depthSensor->unlockMutex();
        
        std::cout << "---" << std::endl;
      }
    }
  }

  server.killServer();
}
