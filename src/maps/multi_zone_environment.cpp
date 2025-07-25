// Multi-Zone Environment - 多区域大场景
// 在同一个世界中创建多个不同的区域

#include "raisim/RaisimServer.hpp"
#include "raisim/World.hpp"

int main(int argc, char* argv[]) {
    auto binaryPath = raisim::Path::setFromArgv(argv[0]);
    raisim::World::setActivationKey(binaryPath.getDirectory() + "\\rsc\\activation.raisim");

    /// 创建世界
    raisim::World world;
    world.setTimeStep(0.001);

    std::cout << "🌍 Creating Multi-Zone Environment..." << std::endl;

    /// ========== 区域1: 山地区域 (坐标: -100 to 0, -100 to 0) ==========
    std::cout << "🏔️ Zone 1: Mountain Area" << std::endl;
    
    std::vector<double> mountainHeight(10000);
    for (int x = 0; x < 100; x++) {
        for (int y = 0; y < 100; y++) {
            int idx = x * 100 + y;
            mountainHeight[idx] = 5.0 * std::sin(x * 0.1) * std::cos(y * 0.1) + 
                                 2.0 * std::sin(x * 0.3) * std::sin(y * 0.2);
        }
    }
    
    auto mountainZone = world.addHeightMap(100, 100, 100, 100, -50, -50, mountainHeight, "grass");
    mountainZone->setAppearance("mountain");
    
    // 山地中的岩石
    for (int i = 0; i < 8; i++) {
        auto rock = world.addSphere(1.0 + i * 0.2, 50.0, "rock");
        rock->setPosition(raisim::Vec<3>{-80 + i * 10, -80 + i * 8, 10 + i});
        rock->setAppearance("gray");
    }

    /// ========== 区域2: 城市区域 (坐标: 0 to 100, -100 to 0) ==========
    std::cout << "🏙️ Zone 2: Urban Area" << std::endl;
    
    // 城市地面
    auto urbanGround = world.addBox(100, 100, 1.0, 1000.0, "concrete");
    urbanGround->setPosition(raisim::Vec<3>{50, -50, -0.5});
    urbanGround->setAppearance("gray");
    
    // 建筑群
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 4; y++) {
            double height = 10 + (x + y) * 3;
            auto building = world.addBox(15.0, 15.0, height, 1000.0, "concrete");
            building->setPosition(raisim::Vec<3>{10 + x * 25, -90 + y * 25, height / 2});
            building->setAppearance("building");
        }
    }

    /// ========== 区域3: 沙漠区域 (坐标: -100 to 0, 0 to 100) ==========
    std::cout << "🏜️ Zone 3: Desert Area" << std::endl;
    
    std::vector<double> desertHeight(10000);
    for (int x = 0; x < 100; x++) {
        for (int y = 0; y < 100; y++) {
            int idx = x * 100 + y;
            desertHeight[idx] = 2.0 * std::sin(x * 0.15) * std::cos(y * 0.12) + 
                               1.0 * std::cos(x * 0.4) * std::sin(y * 0.35);
        }
    }
    
    auto desertZone = world.addHeightMap(100, 100, 100, 100, -50, 50, desertHeight, "sand");
    desertZone->setAppearance("yellow");
    
    // 沙漠中的仙人掌
    for (int i = 0; i < 6; i++) {
        auto cactus = world.addCylinder(0.4, 5.0, 25.0, "plant");
        cactus->setPosition(raisim::Vec<3>{-80 + i * 15, 20 + i * 10, 3});
        cactus->setAppearance("green");
    }

    /// ========== 区域4: 冰雪区域 (坐标: 0 to 100, 0 to 100) ==========
    std::cout << "❄️ Zone 4: Ice Area" << std::endl;
    
    std::vector<double> iceHeight(10000);
    for (int x = 0; x < 100; x++) {
        for (int y = 0; y < 100; y++) {
            int idx = x * 100 + y;
            iceHeight[idx] = 1.0 * std::sin(x * 0.2) * std::cos(y * 0.18);
        }
    }
    
    auto iceZone = world.addHeightMap(100, 100, 100, 100, 50, 50, iceHeight, "ice");
    iceZone->setAppearance("lightblue");
    
    // 冰柱
    for (int i = 0; i < 5; i++) {
        auto icicle = world.addCylinder(0.6, 8.0, 40.0, "ice");
        icicle->setPosition(raisim::Vec<3>{20 + i * 15, 20 + i * 12, 4});
        icicle->setAppearance("blue");
    }

    /// ========== 区域连接: 桥梁和道路 ==========
    std::cout << "🌉 Adding Bridges and Roads..." << std::endl;
    
    // 山地到城市的桥梁
    auto bridge1 = world.addBox(20.0, 5.0, 1.0, 500.0, "wood");
    bridge1->setPosition(raisim::Vec<3>{-10, -50, 8});
    bridge1->setAppearance("brown");
    
    // 城市到冰雪的桥梁
    auto bridge2 = world.addBox(5.0, 20.0, 1.0, 500.0, "wood");
    bridge2->setPosition(raisim::Vec<3>{50, -10, 5});
    bridge2->setAppearance("brown");
    
    // 沙漠到冰雪的桥梁
    auto bridge3 = world.addBox(20.0, 5.0, 1.0, 500.0, "wood");
    bridge3->setPosition(raisim::Vec<3>{-10, 50, 6});
    bridge3->setAppearance("brown");

    /// ========== 材质设置 ==========
    world.setMaterialPairProp("grass", "steel", 0.8, 0.1, 0.001);      // 山地
    world.setMaterialPairProp("concrete", "steel", 0.7, 0.2, 0.001);   // 城市
    world.setMaterialPairProp("sand", "steel", 0.3, 0.05, 0.001);      // 沙漠 (低摩擦)
    world.setMaterialPairProp("ice", "steel", 0.05, 0.9, 0.001);       // 冰雪 (极低摩擦)
    world.setMaterialPairProp("wood", "steel", 0.5, 0.3, 0.001);       // 桥梁

    /// ========== 添加机器人 ==========
    auto aliengo = world.addArticulatedSystem(binaryPath.getDirectory() + "\\rsc\\aliengo\\aliengo.urdf");
    
    // 机器人控制器
    Eigen::VectorXd jointNominalConfig(aliengo->getGeneralizedCoordinateDim()), jointVelocityTarget(aliengo->getDOF());
    jointNominalConfig << -50, -50, 15.24, 1.0, 0.0, 0.0, 0.0, 0.03, 0.4, -0.8, -0.03, 0.4, -0.8, 0.03, -0.4, 0.8, -0.03, -0.4, 0.8;
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
    server.setMap("simple");
    server.focusOn(aliengo);
    server.launchServer();

    std::cout << "\n🎮 Multi-Zone Environment Created!" << std::endl;
    std::cout << "Zone Layout:" << std::endl;
    std::cout << "  🏔️ Mountain Zone: (-100,-100) to (0,0)" << std::endl;
    std::cout << "  🏙️ Urban Zone: (0,-100) to (100,0)" << std::endl;
    std::cout << "  🏜️ Desert Zone: (-100,0) to (0,100)" << std::endl;
    std::cout << "  ❄️ Ice Zone: (0,0) to (100,100)" << std::endl;
    std::cout << "  🌉 Bridges connect all zones" << std::endl;
    std::cout << "\n🤖 Robot starts in Mountain Zone" << std::endl;
    std::cout << "💡 Move robot to experience different materials!" << std::endl;

    /// ========== 仿真循环 ==========
    for (int i = 0; i < 2000000; i++) {
        RS_TIMED_LOOP(int(world.getTimeStep()*1e6))
        
        // 每1000步显示机器人位置信息
        if (i % 1000 == 0) {
            auto pos = aliengo->getGeneralizedCoordinate();
            double x = pos[0], y = pos[1];
            
            std::string zone = "Unknown";
            if (x < 0 && y < 0) zone = "🏔️ Mountain";
            else if (x >= 0 && y < 0) zone = "🏙️ Urban";
            else if (x < 0 && y >= 0) zone = "🏜️ Desert";
            else if (x >= 0 && y >= 0) zone = "❄️ Ice";
            
            if (i % 5000 == 0) {
                std::cout << "Robot in " << zone << " zone at (" 
                         << std::fixed << std::setprecision(1) << x << ", " << y << ")" << std::endl;
            }
        }
        
        server.integrateWorldThreadSafe();
    }

    std::cout << "Robot mass: " << aliengo->getMassMatrix()[0] << std::endl;
    server.killServer();
    return 0;
}
