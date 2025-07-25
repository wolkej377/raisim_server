// Multi-Scene Environment Demonstration
// 展示如何在RaiSim中创建和切换多个场景

#include "raisim/RaisimServer.hpp"
#include "raisim/World.hpp"
#include <iostream>
#include <vector>
#include <memory>

class SceneManager {
private:
    raisim::World* world_;
    std::vector<std::vector<raisim::Object*>> scenes_;
    int currentScene_;
    
public:
    SceneManager(raisim::World* world) : world_(world), currentScene_(0) {
        scenes_.resize(4); // 4个场景
    }
    
    // 场景1: 山地环境
    void createMountainScene() {
        std::cout << "🏔️ Creating Mountain Scene..." << std::endl;
        
        auto binaryPath = raisim::Path::setFromArgv("");
        
        // 加载山地高度图
        auto heightmap = world_->addHeightMap(binaryPath.getDirectory() + "\\rsc\\raisimUnrealMaps\\hill1.png",
                                             0, 0, 504, 504, 38.0/(37312-32482), -32650 * 38.0/(37312-32482), "grass");
        heightmap->setAppearance("mountain");
        scenes_[0].push_back(heightmap);
        
        // 添加山石
        for (int i = 0; i < 10; i++) {
            auto rock = world_->addSphere(0.5 + i * 0.1, 100.0, "rock");
            rock->setPosition(raisim::Vec<3>{-20 + i * 4, -10 + i * 2, 15 + i});
            rock->setAppearance("gray");
            scenes_[0].push_back(rock);
        }
        
        // 设置山地材质
        world_->setMaterialPairProp("grass", "rock", 0.8, 0.1, 0.001);
    }
    
    // 场景2: 城市环境
    void createUrbanScene() {
        std::cout << "🏙️ Creating Urban Scene..." << std::endl;
        
        // 平坦地面
        auto ground = world_->addGround(0.0, "concrete");
        scenes_[1].push_back(ground);
        
        // 创建建筑群
        for (int x = 0; x < 5; x++) {
            for (int y = 0; y < 5; y++) {
                if ((x + y) % 2 == 0) {
                    double height = 5 + (x + y) * 2;
                    auto building = world_->addBox(8.0, 8.0, height, 1000.0, "concrete");
                    building->setPosition(raisim::Vec<3>{x * 20 - 40, y * 20 - 40, height / 2});
                    building->setAppearance("building");
                    scenes_[1].push_back(building);
                }
            }
        }
        
        // 添加道路障碍
        for (int i = 0; i < 20; i++) {
            auto car = world_->addBox(4.0, 2.0, 1.5, 50.0, "metal");
            car->setPosition(raisim::Vec<3>{-50 + i * 5, 0, 1});
            car->setAppearance("red");
            scenes_[1].push_back(car);
        }
        
        world_->setMaterialPairProp("concrete", "metal", 0.7, 0.2, 0.001);
    }
    
    // 场景3: 沙漠环境
    void createDesertScene() {
        std::cout << "🏜️ Creating Desert Scene..." << std::endl;
        
        // 沙丘地形
        std::vector<double> sandHeight(10000);
        for (int x = 0; x < 100; x++) {
            for (int y = 0; y < 100; y++) {
                int idx = x * 100 + y;
                sandHeight[idx] = 3.0 * std::sin(x * 0.2) * std::cos(y * 0.15) + 
                                 1.5 * std::sin(x * 0.4) * std::sin(y * 0.3);
            }
        }
        
        auto desert = world_->addHeightMap(100, 100, 200, 200, 0.0, 0.0, sandHeight, "sand");
        desert->setAppearance("yellow");
        scenes_[2].push_back(desert);
        
        // 添加仙人掌
        for (int i = 0; i < 15; i++) {
            auto cactus = world_->addCylinder(0.3, 3.0, 10.0, "plant");
            cactus->setPosition(raisim::Vec<3>{-80 + i * 10, -60 + (i % 3) * 40, 5});
            cactus->setAppearance("green");
            scenes_[2].push_back(cactus);
        }
        
        // 沙漠材质 (低摩擦)
        world_->setMaterialPairProp("sand", "steel", 0.3, 0.05, 0.001);
        world_->setMaterialPairProp("sand", "plant", 0.5, 0.1, 0.001);
    }
    
    // 场景4: 冰雪环境
    void createIceScene() {
        std::cout << "❄️ Creating Ice Scene..." << std::endl;
        
        // 冰面地形
        std::vector<double> iceHeight(10000);
        for (int x = 0; x < 100; x++) {
            for (int y = 0; y < 100; y++) {
                int idx = x * 100 + y;
                iceHeight[idx] = 0.5 * std::sin(x * 0.3) * std::cos(y * 0.25);
            }
        }
        
        auto ice = world_->addHeightMap(100, 100, 150, 150, 0.0, 0.0, iceHeight, "ice");
        ice->setAppearance("blue");
        scenes_[3].push_back(ice);
        
        // 添加冰柱
        for (int i = 0; i < 12; i++) {
            auto icicle = world_->addCylinder(0.5, 8.0, 50.0, "ice");
            icicle->setPosition(raisim::Vec<3>{-60 + i * 10, -30 + (i % 2) * 60, 4});
            icicle->setAppearance("lightblue");
            scenes_[3].push_back(icicle);
        }
        
        // 冰面材质 (极低摩擦)
        world_->setMaterialPairProp("ice", "steel", 0.05, 0.9, 0.001);
    }
    
    // 清除当前场景
    void clearCurrentScene() {
        if (currentScene_ < scenes_.size()) {
            for (auto* obj : scenes_[currentScene_]) {
                world_->removeObject(obj);
            }
            scenes_[currentScene_].clear();
        }
    }
    
    // 切换到指定场景
    void switchToScene(int sceneId) {
        if (sceneId < 0 || sceneId >= 4) return;
        
        std::cout << "🔄 Switching from scene " << currentScene_ << " to scene " << sceneId << std::endl;
        
        // 清除当前场景
        clearCurrentScene();
        
        currentScene_ = sceneId;
        
        // 创建新场景
        switch (sceneId) {
            case 0: createMountainScene(); break;
            case 1: createUrbanScene(); break;
            case 2: createDesertScene(); break;
            case 3: createIceScene(); break;
        }
    }
    
    int getCurrentScene() const { return currentScene_; }
};

int main(int argc, char* argv[]) {
    auto binaryPath = raisim::Path::setFromArgv(argv[0]);
    raisim::World::setActivationKey(binaryPath.getDirectory() + "\\rsc\\activation.raisim");

    /// 创建RaiSim世界
    raisim::World world;
    world.setTimeStep(0.001);
    
    /// 创建场景管理器
    SceneManager sceneManager(&world);
    
    /// 添加机器人
    auto aliengo = world.addArticulatedSystem(binaryPath.getDirectory() + "\\rsc\\aliengo\\aliengo.urdf");
    
    // 机器人控制器设置
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

    /// 启动服务器
    raisim::RaisimServer server(&world);
    server.focusOn(aliengo);
    server.launchServer();
    
    /// 初始化为山地场景
    sceneManager.switchToScene(0);
    
    std::cout << "\n🎮 Multi-Scene Environment Loaded!" << std::endl;
    std::cout << "Scenes Available:" << std::endl;
    std::cout << "  0️⃣ Mountain Scene (草地+岩石)" << std::endl;
    std::cout << "  1️⃣ Urban Scene (城市+建筑)" << std::endl;
    std::cout << "  2️⃣ Desert Scene (沙漠+仙人掌)" << std::endl;
    std::cout << "  3️⃣ Ice Scene (冰面+冰柱)" << std::endl;
    std::cout << "\n🔄 Scenes will auto-switch every 10000 steps\n" << std::endl;

    /// 仿真循环 - 自动切换场景
    int sceneChangeInterval = 10000;  // 每10000步切换场景
    
    for (int i = 0; i < 80000; i++) {
        RS_TIMED_LOOP(int(world.getTimeStep()*1e6))
        
        // 每隔一定步数切换场景
        if (i > 0 && i % sceneChangeInterval == 0) {
            int nextScene = (sceneManager.getCurrentScene() + 1) % 4;
            sceneManager.switchToScene(nextScene);
            
            // 重新设置机器人位置
            aliengo->setGeneralizedCoordinate(jointNominalConfig);
        }
        
        server.integrateWorldThreadSafe();
    }

    std::cout << "Robot mass: " << aliengo->getMassMatrix()[0] << std::endl;
    server.killServer();
    return 0;
}
