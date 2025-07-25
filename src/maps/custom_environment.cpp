// Custom environment example for RaiSim
#include "raisim/RaisimServer.hpp"
#include "raisim/World.hpp"
#include <random>

int main(int argc, char* argv[]) {
  auto binaryPath = raisim::Path::setFromArgv(argv[0]);
  raisim::World::setActivationKey(binaryPath.getDirectory() + "\\rsc\\activation.raisim");

  /// create raisim world
  raisim::World world;
  world.setTimeStep(0.001);

  /// Method 1: Use heightmap (if you have a custom terrain image)
  // auto heightmap = world.addHeightMap("path/to/your/terrain.png",
  //                                     0, 0, 50.0, 50.0, 0.01, 0.0);
  // heightmap->setAppearance("hidden");

  /// Method 2: Programmatic environment creation
  // Ground
  auto ground = world.addGround(0.0);
  ground->setAppearance("hidden");
  
  // Random number generator
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> pos_dist(-15.0, 15.0);
  std::uniform_real_distribution<> size_dist(0.5, 2.0);
  
  // Add random obstacles
  for(int i = 0; i < 15; i++) {
    double size = size_dist(gen);
    auto obstacle = world.addBox(size, size, size, 1.0);
    obstacle->setPosition(raisim::Vec<3>{
        pos_dist(gen), 
        pos_dist(gen), 
        size/2.0
    });
    obstacle->setAppearance("red");
  }
  
  // Add some cylinders
  for(int i = 0; i < 8; i++) {
    double radius = size_dist(gen) * 0.5;
    double height = size_dist(gen) * 2.0;
    auto cylinder = world.addCylinder(radius, height, 1.0);
    cylinder->setPosition(raisim::Vec<3>{
        pos_dist(gen), 
        pos_dist(gen), 
        height/2.0
    });
    cylinder->setAppearance("blue");
  }
  
  // Add a ramp
  auto ramp = world.addBox(4.0, 2.0, 0.2, 1.0);
  ramp->setPosition(raisim::Vec<3>{8.0, 0.0, 1.0});
  ramp->setOrientation(raisim::Mat<3,3>{
      1.0, 0.0, 0.0,
      0.0, 0.866, -0.5,  // 30 degree rotation
      0.0, 0.5, 0.866
  });
  ramp->setAppearance("green");
  
  // Add robot
  auto robot = world.addArticulatedSystem(binaryPath.getDirectory() + "\\rsc\\aliengo\\aliengo.urdf");

  /// Robot controller
  Eigen::VectorXd jointNominalConfig(robot->getGeneralizedCoordinateDim()), jointVelocityTarget(robot->getDOF());
  jointNominalConfig << 0, 0, 1.24, 1.0, 0.0, 0.0, 0.0, 0.03, 0.4, -0.8, -0.03, 0.4, -0.8, 0.03, -0.4, 0.8, -0.03, -0.4, 0.8;
  jointVelocityTarget.setZero();

  Eigen::VectorXd jointPgain(robot->getDOF()), jointDgain(robot->getDOF());
  jointPgain.tail(12).setConstant(100.0);
  jointDgain.tail(12).setConstant(1.0);

  robot->setGeneralizedCoordinate(jointNominalConfig);
  robot->setGeneralizedForce(Eigen::VectorXd::Zero(robot->getDOF()));
  robot->setPdGains(jointPgain, jointDgain);
  robot->setPdTarget(jointNominalConfig, jointVelocityTarget);
  robot->setName("robot");

  /// launch raisim server
  raisim::RaisimServer server(&world);
  server.setMap("simple");  // Use simple background
  server.focusOn(robot);
  server.launchServer();

  std::cout << "Custom environment created!" << std::endl;
  std::cout << "- Ground plane" << std::endl;
  std::cout << "- 15 random box obstacles" << std::endl;
  std::cout << "- 8 cylindrical obstacles" << std::endl;
  std::cout << "- 1 ramp" << std::endl;

  for (int i=0; i<2000000; i++) {
    RS_TIMED_LOOP(int(world.getTimeStep()*1e6))
    server.integrateWorldThreadSafe();
  }

  server.killServer();
  return 0;
}
