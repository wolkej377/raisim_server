// Advanced Custom Environment with Multiple Scene Creation Methods
#include "raisim/RaisimServer.hpp"
#include "raisim/World.hpp"
#include <random>
#include <cmath>

int main(int argc, char* argv[]) {
  auto binaryPath = raisim::Path::setFromArgv(argv[0]);
  raisim::World::setActivationKey(binaryPath.getDirectory() + "\\rsc\\activation.raisim");

  /// create raisim world
  raisim::World world;
  world.setTimeStep(0.001);

  /// ========== METHOD 1: Dynamic HeightMap (Procedural Terrain) ==========
  std::vector<double> terrainHeight(10000, 0.);  // 100x100 grid
  std::vector<raisim::ColorRGB> terrainColors(10000, {0, 255, 0});  // Green terrain
  
  // Generate procedural terrain
  for (int x = 0; x < 100; x++) {
    for (int y = 0; y < 100; y++) {
      int idx = x * 100 + y;
      // Create hills and valleys using sine waves
      double height = 2.0 * std::sin(x * 0.1) * std::cos(y * 0.1) + 
                     1.0 * std::sin(x * 0.3) * std::sin(y * 0.2) +
                     0.5 * std::cos(x * 0.5) * std::cos(y * 0.4);
      terrainHeight[idx] = height;
      
      // Color based on height (blue=low, green=mid, brown=high)
      if (height < -1.0) {
        terrainColors[idx] = {0, 100, 255};    // Blue (water)
      } else if (height < 1.0) {
        terrainColors[idx] = {0, 255, 100};    // Green (grass)
      } else {
        terrainColors[idx] = {139, 69, 19};    // Brown (mountains)
      }
    }
  }
  
  auto heightMap = world.addHeightMap(100, 100, 20.0, 20.0, 0.0, 0.0, terrainHeight);
  heightMap->setName("custom_terrain");
  heightMap->setColor(terrainColors);

  /// ========== METHOD 2: Structured Environment ==========
  // Create a maze-like structure
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      if ((i + j) % 3 == 0) {  // Create a pattern
        auto wall = world.addBox(1.0, 1.0, 2.0, 1.0);
        wall->setPosition(raisim::Vec<3>{-15.0 + i * 2.0, -15.0 + j * 2.0, 1.0});
        wall->setAppearance("gray");
      }
    }
  }

  /// ========== METHOD 3: Complex Structures ==========
  // Create a bridge
  auto bridge_support1 = world.addCylinder(0.5, 8.0, 1.0);
  bridge_support1->setPosition(raisim::Vec<3>{-5.0, 0.0, 4.0});
  bridge_support1->setAppearance("brown");
  
  auto bridge_support2 = world.addCylinder(0.5, 8.0, 1.0);
  bridge_support2->setPosition(raisim::Vec<3>{5.0, 0.0, 4.0});
  bridge_support2->setAppearance("brown");
  
  auto bridge_deck = world.addBox(12.0, 2.0, 0.2, 1.0);
  bridge_deck->setPosition(raisim::Vec<3>{0.0, 0.0, 8.2});
  bridge_deck->setAppearance("wood");

  // Create stairs to the bridge
  for (int i = 0; i < 8; i++) {
    auto step = world.addBox(2.0, 2.0, 0.2, 1.0);
    step->setPosition(raisim::Vec<3>{-8.0 + i * 0.5, 3.0, i * 1.0 + 0.5});
    step->setAppearance("concrete");
  }

  /// ========== METHOD 4: Interactive Objects ==========
  // Create moving platforms (these will be controlled in the loop)
  std::vector<raisim::SingleBodyObject*> movingPlatforms;
  for (int i = 0; i < 3; i++) {
    auto platform = world.addBox(3.0, 3.0, 0.3, 1.0);
    platform->setPosition(raisim::Vec<3>{10.0 + i * 5.0, 0.0, 2.0});
    platform->setAppearance("metal");
    movingPlatforms.push_back(platform);
  }

  /// ========== METHOD 5: Particle/Debris Field ==========
  std::vector<raisim::SingleBodyObject*> debris;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> pos_dist(-25.0, 25.0);
  std::uniform_real_distribution<> size_dist(0.1, 0.8);
  
  for (int i = 0; i < 50; i++) {
    double size = size_dist(gen);
    auto rock = world.addSphere(size, 0.5);
    rock->setPosition(raisim::Vec<3>{pos_dist(gen), pos_dist(gen), 10.0 + i * 0.5});
    rock->setLinearVelocity(raisim::Vec<3>{
      (gen() % 20 - 10) * 0.1, 
      (gen() % 20 - 10) * 0.1, 
      -5.0
    });
    debris.push_back(rock);
  }

  /// ========== Add Robot ==========
  auto robot = world.addArticulatedSystem(binaryPath.getDirectory() + "\\rsc\\aliengo\\aliengo.urdf");
  
  // Robot controller
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

  /// ========== Launch Server ==========
  raisim::RaisimServer server(&world);
  server.setMap("simple");  // You can also use "mountain1", "office1", etc.
  server.focusOn(robot);
  server.launchServer();

  std::cout << "🌍 Advanced Custom Environment Created!" << std::endl;
  std::cout << "Features:" << std::endl;
  std::cout << "  🏔️  Dynamic procedural terrain with color mapping" << std::endl;
  std::cout << "  🏗️  Maze-like wall structures" << std::endl;
  std::cout << "  🌉  Bridge with support pillars and stairs" << std::endl;
  std::cout << "  📦  Moving platforms (animated)" << std::endl;
  std::cout << "  💎  Falling debris field" << std::endl;
  std::cout << "  🤖  Robot with PD controller" << std::endl;

  /// ========== Dynamic Environment Loop ==========
  for (int i = 0; i < 2000000; i++) {
    RS_TIMED_LOOP(int(world.getTimeStep()*1e6))
    
    // Update moving platforms
    for (size_t j = 0; j < movingPlatforms.size(); j++) {
      double time = world.getWorldTime();
      double offset = j * 2.0 * M_PI / movingPlatforms.size();
      raisim::Vec<3> newPos = {
        10.0 + j * 5.0 + 3.0 * std::sin(time * 0.5 + offset),
        3.0 * std::cos(time * 0.3 + offset),
        2.0 + 1.0 * std::sin(time * 0.8 + offset)
      };
      movingPlatforms[j]->setPosition(newPos);
    }
    
    // Update terrain dynamically (optional - creates waves)
    if (i % 100 == 0) {  // Update every 100 steps
      server.lockVisualizationServerMutex();
      for (int x = 0; x < 100; x++) {
        for (int y = 0; y < 100; y++) {
          int idx = x * 100 + y;
          double waveOffset = world.getWorldTime() * 2.0;
          terrainHeight[idx] += 0.1 * std::sin(waveOffset + x * 0.2 + y * 0.2);
        }
      }
      heightMap->update(0.0, 0.0, 20.0, 20.0, terrainHeight);
      server.unlockVisualizationServerMutex();
    }
    
    server.integrateWorldThreadSafe();
  }

  server.killServer();
  return 0;
}
