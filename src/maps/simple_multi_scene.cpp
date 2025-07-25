// Simple Multi-Scene Demo
// 简单的多场景演示

#include "raisim/RaisimServer.hpp"
#include "raisim/World.hpp"

void createScene1(raisim::World& world) {
    std::cout << "🏔️ Loading Mountain Scene..." << std::endl;
    
    auto binaryPath = raisim::Path::setFromArgv("");
    auto heightmap = world.addHeightMap(binaryPath.getDirectory() + "\\rsc\\raisimUnrealMaps\\hill1.png",
                                       0, 0, 504, 504, 38.0/(37312-32482), -32650 * 38.0/(37312-32482), "grass");
    heightmap->setAppearance("mountain");
    
    // 添加一些岩石
    auto rock1 = world.addSphere(2.0, 100.0, "rock");
    rock1->setPosition(raisim::Vec<3>{10, 10, 15});
    rock1->setAppearance("gray");
    
    auto rock2 = world.addBox(3.0, 3.0, 2.0, 200.0, "rock");
    rock2->setPosition(raisim::Vec<3>{-15, 5, 10});
    rock2->setAppearance("darkgray");
}

void createScene2(raisim::World& world) {
    std::cout << "🏙️ Loading Urban Scene..." << std::endl;
    
    // 平地
    auto ground = world.addGround(0.0, "concrete");
    
    // 建筑物
    for (int i = 0; i < 5; i++) {
        auto building = world.addBox(10.0, 10.0, 20.0, 1000.0, "concrete");
        building->setPosition(raisim::Vec<3>{i * 25 - 50, 0, 10});
        building->setAppearance("building");
    }
    
    // 道路障碍
    for (int i = 0; i < 8; i++) {
        auto car = world.addBox(4.0, 2.0, 1.5, 50.0, "metal");
        car->setPosition(raisim::Vec<3>{i * 8 - 30, 15, 1});
        car->setAppearance("red");
    }
}

void createScene3(raisim::World& world) {
    std::cout << "🏜️ Loading Desert Scene..." << std::endl;
    
    // 沙地
    std::vector<double> sandHeight(10000);
    for (int x = 0; x < 100; x++) {
        for (int y = 0; y < 100; y++) {
            int idx = x * 100 + y;
            sandHeight[idx] = 2.0 * std::sin(x * 0.1) * std::cos(y * 0.1);
        }
    }
    
    auto desert = world.addHeightMap(100, 100, 100, 100, 0.0, 0.0, sandHeight, "sand");
    desert->setAppearance("yellow");
    
    // 仙人掌
    for (int i = 0; i < 6; i++) {
        auto cactus = world.addCylinder(0.5, 4.0, 20.0, "plant");
        cactus->setPosition(raisim::Vec<3>{-30 + i * 10, -20 + (i % 2) * 40, 2});
        cactus->setAppearance("green");
    }
    
    // 沙漠材质 (低摩擦)
    world.setMaterialPairProp("sand", "steel", 0.2, 0.05, 0.001);
}

int main(int argc, char* argv[]) {
    auto binaryPath = raisim::Path::setFromArgv(argv[0]);
    raisim::World::setActivationKey(binaryPath.getDirectory() + "\\rsc\\activation.raisim");

    std::cout << "🎮 Welcome to Multi-Scene RaiSim Demo!" << std::endl;
    std::cout << "Choose a scene:" << std::endl;
    std::cout << "1 - Mountain Environment" << std::endl;
    std::cout << "2 - Urban Environment" << std::endl;
    std::cout << "3 - Desert Environment" << std::endl;
    std::cout << "Enter choice (1-3): ";
    
    int choice;
    std::cin >> choice;

    /// 创建世界
    raisim::World world;
    world.setTimeStep(0.001);
    
    /// 根据选择创建场景
    switch(choice) {
        case 1:
            createScene1(world);
            world.setMaterialPairProp("grass", "steel", 0.8, 0.1, 0.001);
            break;
        case 2:
            createScene2(world);
            world.setMaterialPairProp("concrete", "metal", 0.7, 0.2, 0.001);
            break;
        case 3:
            createScene3(world);
            world.setMaterialPairProp("sand", "plant", 0.5, 0.1, 0.001);
            break;
        default:
            std::cout << "Invalid choice, loading mountain scene..." << std::endl;
            createScene1(world);
            break;
    }

    /// 添加机器人
    auto aliengo = world.addArticulatedSystem(binaryPath.getDirectory() + "\\rsc\\aliengo\\aliengo.urdf");
    
    // 机器人控制器
    Eigen::VectorXd jointNominalConfig(aliengo->getGeneralizedCoordinateDim()), jointVelocityTarget(aliengo->getDOF());
    jointNominalConfig << 0, 0, 15.24, 1.0, 0.0, 0.0, 0.0, 0.03, 0.4, -0.8, -0.03, 0.4, -0.8, 0.03, -0.4, 0.8, -0.03, -0.4, 0.8;
    jointVelocityTarget.setZero();

    Eigen::VectorXd jointPgain(aliengo->getDOF()), jointDgain(aliengo->getDOF());
    jointPgain.tail(12).setConstant(100.0);
    jointDgain.tail(12).setConstant(1.0);

    aliengo->setGeneralizedCoordinate(jointNominalConfig);
    aliengo->setGeneralizedForce(Eigen::VectorXd::Zero(aliengo->getDOF()));
    aliengo->setPdGains(jointPgain, jointDgain);
    aliengo->setPdTarget(jointNominalConfig, jointVelocityTarget);
    aliengo->setName("aliengo");

    /// 启动服务器
    raisim::RaisimServer server(&world);
    
    // 根据场景设置不同的地图
    switch(choice) {
        case 1: server.setMap("hill1"); break;
        case 2: server.setMap("simple"); break;
        case 3: server.setMap("simple"); break;
        default: server.setMap("hill1"); break;
    }
    
    server.focusOn(aliengo);
    server.launchServer();

    std::cout << "✅ Scene loaded successfully! Press Ctrl+C to exit." << std::endl;

    /// 仿真循环
    for (int i = 0; i < 2000000; i++) {
        RS_TIMED_LOOP(int(world.getTimeStep()*1e6))
        server.integrateWorldThreadSafe();
    }

    std::cout << "Robot mass: " << aliengo->getMassMatrix()[0] << std::endl;
    server.killServer();
    return 0;
}
