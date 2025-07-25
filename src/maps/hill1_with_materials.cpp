// Enhanced hill1.cpp with Material Settings
// This file is part of RaiSim. You must obtain a valid license from RaiSim Tech
// Inc. prior to usage.

#include "raisim/RaisimServer.hpp"
#include "raisim/World.hpp"

int main(int argc, char* argv[]) {
  auto binaryPath = raisim::Path::setFromArgv(argv[0]);
  raisim::World::setActivationKey(binaryPath.getDirectory() + "\\rsc\\activation.raisim");

  /// create raisim world
  raisim::World world;
  world.setTimeStep(0.001);

  /// ========== 材质设置 (Material Settings) ==========
  
  // 方法1: 直接指定材质名称创建高度图
  auto heightmap = world.addHeightMap(binaryPath.getDirectory() + "\\rsc\\raisimUnrealMaps\\hill1.png",
                                      0, 0, 504, 504, 38.0/(37312-32482), -32650 * 38.0/(37312-32482),
                                      "grass");  // 指定材质名称
  
  // 方法2: 设置材质对之间的物理属性
  // setMaterialPairProp(material1, material2, friction, restitution, threshold)
  world.setMaterialPairProp("grass", "steel", 0.8, 0.1, 0.001);    // 草地-钢铁
  world.setMaterialPairProp("grass", "rubber", 1.2, 0.3, 0.001);   // 草地-橡胶
  world.setMaterialPairProp("steel", "steel", 0.6, 0.2, 0.001);    // 钢铁-钢铁
  
  // 方法3: 设置默认材质属性
  world.setDefaultMaterial(0.8, 0.0, 0.001);  // friction, restitution, threshold
  
  // 设置高度图外观（如果不想隐藏的话）
  // heightmap->setAppearance("grass");  // 替代 "hidden"
  heightmap->setAppearance("hidden");  // 保持原来的隐藏设置

  /// 添加机器人
  auto aliengo = world.addArticulatedSystem(binaryPath.getDirectory() + "\\rsc\\aliengo\\aliengo.urdf");

  /// ========== 额外的环境物体与材质 ==========
  
  // 添加一些具有不同材质的物体来测试交互
  auto metalBox = world.addBox(2.0, 2.0, 1.0, 100.0, "steel");
  metalBox->setPosition(raisim::Vec<3>{5.0, 5.0, 20.0});
  metalBox->setAppearance("gray");
  
  auto rubberBall = world.addSphere(1.0, 50.0, "rubber");
  rubberBall->setPosition(raisim::Vec<3>{-5.0, -5.0, 25.0});
  rubberBall->setAppearance("red");
  
  // 添加一个木制平台
  auto woodPlatform = world.addBox(4.0, 4.0, 0.2, 200.0, "wood");
  woodPlatform->setPosition(raisim::Vec<3>{0.0, 10.0, 15.0});
  woodPlatform->setAppearance("brown");
  
  // 设置木材相关的材质属性
  world.setMaterialPairProp("wood", "steel", 0.4, 0.3, 0.001);    // 木材-钢铁
  world.setMaterialPairProp("grass", "wood", 0.6, 0.1, 0.001);    // 草地-木材

  /// ========== 机器人控制器设置 ==========
  Eigen::VectorXd jointNominalConfig(aliengo->getGeneralizedCoordinateDim()), jointVelocityTarget(aliengo->getDOF());
  jointNominalConfig << 0, 0, 10.24, 1.0, 0.0, 0.0, 0.0, 0.03, 0.4, -0.8, -0.03, 0.4, -0.8, 0.03, -0.4, 0.8, -0.03, -0.4, 0.8;
  jointVelocityTarget.setZero();

  Eigen::VectorXd jointPgain(aliengo->getDOF()), jointDgain(aliengo->getDOF());
  jointPgain.tail(12).setConstant(100.0);
  jointDgain.tail(12).setConstant(1.0);

  aliengo->setGeneralizedCoordinate(jointNominalConfig);
  aliengo->setGeneralizedForce(Eigen::VectorXd::Zero(aliengo->getDOF()));
  aliengo->setPdGains(jointPgain, jointDgain);
  aliengo->setPdTarget(jointNominalConfig, jointVelocityTarget);
  aliengo->setName("aliengo");

  /// ========== 启动服务器 ==========
  raisim::RaisimServer server(&world);
  server.setMap("hill1");
  server.focusOn(aliengo);
  server.launchServer();

  std::cout << "🏔️ Hill1 Environment with Materials Loaded!" << std::endl;
  std::cout << "Material Properties:" << std::endl;
  std::cout << "  🌱 Grass (heightmap): Default terrain material" << std::endl;
  std::cout << "  🔩 Steel Box: High friction, low restitution" << std::endl;
  std::cout << "  🔴 Rubber Ball: High friction, medium restitution" << std::endl;
  std::cout << "  🪵 Wood Platform: Medium friction, low restitution" << std::endl;

  /// ========== 仿真循环 ==========
  for (int i=0; i<2000000; i++) {
    RS_TIMED_LOOP(int(world.getTimeStep()*1e6))
    
    // 可以在这里添加动态材质更新或其他逻辑
    server.integrateWorldThreadSafe();
  }

  std::cout<<"Robot mass: "<<aliengo->getMassMatrix()[0]<<std::endl;
  server.killServer();
  return 0;
}
