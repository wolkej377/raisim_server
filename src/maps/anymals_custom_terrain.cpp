#include "raisim/RaisimServer.hpp"
#include <random>
#include <iostream>

int main(int argc, char* argv[]) {
  auto binaryPath = raisim::Path::setFromArgv(argv[0]);
  raisim::World::setActivationKey(binaryPath.getDirectory() + "\\rsc\\activation.raisim");
  raisim::RaiSimMsg::setFatalCallback([](){throw;});

  // 创建 Raisim 世界
  raisim::World world;
  world.setTimeStep(0.001);

  // 生成高度图数据（例如：正弦波地形）
  const int xSize = 100, ySize = 100;
  std::vector<double> heightMap(xSize * ySize, 0.0);
  for (int x = 0; x < xSize; ++x) {
    for (int y = 0; y < ySize; ++y) {
      heightMap[x * ySize + y] = 0.5 * std::sin(x * 0.2) * std::cos(y * 0.2);
    }
  }

  // 添加高度图地形
  auto hm = world.addHeightMap(xSize, ySize, 20.0, 20.0, 0.0, 0.0, heightMap);
  hm->setName("custom_terrain");

  // 添加机器人
  auto anymalC = world.addArticulatedSystem(binaryPath.getDirectory() + "\\rsc\\anymal_c\\urdf\\anymal_sensored.urdf");
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

  // 设置传感器
  auto front_depthSensor = anymalC->getSensorSet("depth_camera_front_camera_parent")->getSensor<raisim::DepthCamera>("depth");
  front_depthSensor->setMeasurementSource(raisim::Sensor::MeasurementSource::VISUALIZER);
  auto front_rgbCamera = anymalC->getSensorSet("depth_camera_front_camera_parent")->getSensor<raisim::RGBCamera>("color");
  front_rgbCamera->setMeasurementSource(raisim::Sensor::MeasurementSource::VISUALIZER);
  auto rear_depthSensor = anymalC->getSensorSet("depth_camera_rear_camera_parent")->getSensor<raisim::DepthCamera>("depth");
  rear_depthSensor->setMeasurementSource(raisim::Sensor::MeasurementSource::VISUALIZER);
  auto rear_rgbCamera = anymalC->getSensorSet("depth_camera_rear_camera_parent")->getSensor<raisim::RGBCamera>("color");
  rear_rgbCamera->setMeasurementSource(raisim::Sensor::MeasurementSource::VISUALIZER);

  // 启动 RaisimServer
  raisim::RaisimServer server(&world);
  server.launchServer();
  server.focusOn(anymalC);

  // 添加关节图表
  std::vector<std::string> jointNames = {"LF_HAA", "LF_HFE", "LF_KFE", "RF_HAA", "RF_HFE", "RF_KFE",
                                         "LH_HAA", "LH_HFE", "LH_KFE", "RH_HAA", "RH_HFE", "RH_KFE"};
  auto jcGraph = server.addTimeSeriesGraph("joint position", jointNames, "time", "position");
  auto jvGraph = server.addTimeSeriesGraph("joint velocity", jointNames, "time", "velocity");
  auto jfGraph = server.addTimeSeriesGraph("joint torque", jointNames, "time", "torque");

  // 主循环
  for (int i=0; i<2000000; i++) {
    RS_TIMED_LOOP(int(world.getTimeStep()*1e6))
    server.integrateWorldThreadSafe();
    if (i % 10 == 0) {
      auto gc = anymalC->getGeneralizedCoordinate();
      auto gv = anymalC->getGeneralizedVelocity();
      auto gf = anymalC->getGeneralizedForce();
      raisim::Vec<3> base_pos = {gc[0], gc[1], gc[2]};
      // 可在此添加更多状态处理
    }
  }
  server.killServer();
  return 0;
}