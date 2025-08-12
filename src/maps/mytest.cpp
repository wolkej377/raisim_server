#include "raisim/RaisimServer.hpp"
#include "raisim/World.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <cmath>

class SceneManager
{
private:
    raisim::World *world_;
    raisim::RaisimServer *server_;
    std::vector<std::vector<raisim::Object *>> scenes_;
    int currentScene_;
    raisim::HeightMap *currentHeightMap_;
    raisim::Path binaryPath_;
    raisim::ArticulatedSystem *robot_;
    raisim::Vec<3> robotPosition_;
    raisim::Vec<4> robotOrientation_;

public:
    SceneManager(raisim::World *world, raisim::RaisimServer *server, const raisim::Path &path) : world_(world), server_(server), currentScene_(0), binaryPath_(path)
    {
        scenes_.resize(4); // 4个场景
    }

    void createHillScene()
    {
        std::cout << "Creating Hill Scene..." << std::endl;
        auto heightmap = world_->addHeightMap(binaryPath_.getDirectory() + "\\rsc\\raisimUnrealMaps\\hill1.png",
                                              0, 0, 504, 504, 38.0 / (37312 - 32482), -32650 * 38.0 / (37312 - 32482), "grass");
        currentHeightMap_ = heightmap;
        heightmap->setAppearance("hidden");
        scenes_[0].push_back(heightmap);
        world_->setMaterialPairProp("grass", "steel", 0.8, 0.1, 0.001);
        server_->setMap("hill1");
    }

    void createLakeScene()
    {
        std::cout << "Creating Lake Scene..." << std::endl;

        auto heightmap = world_->addHeightMap(binaryPath_.getDirectory() + "\\rsc\\raisimUnrealMaps\\lake1.png",
                                              0, 0, 504, 504, 38.0 / (37312 - 32482), -32650 * 38.0 / (37312 - 32482), "grass");
        currentHeightMap_ = heightmap;
        heightmap->setAppearance("hidden");
        scenes_[1].push_back(heightmap);
        world_->setMaterialPairProp("grass", "steel", 0.8, 0.1, 0.001);
        server_->setMap("lake1");
    }

    void createMountainScene()
    {
        std::cout << "Creating Mountain Scene..." << std::endl;

        auto heightmap = world_->addHeightMap(binaryPath_.getDirectory() + "\\rsc\\raisimUnrealMaps\\mountain1.png",
                                              0, 0, 504, 504, 38.0 / (37312 - 32482), -32650 * 38.0 / (37312 - 32482), "grass");
        currentHeightMap_ = heightmap;
        heightmap->setAppearance("hidden");
        scenes_[2].push_back(heightmap);
        world_->setMaterialPairProp("grass", "steel", 0.8, 0.1, 0.001);
        server_->setMap("mountain1");
    }

    void createWheatScene()
    {
        std::cout << "Creating Wheat Scene..." << std::endl;

        std::vector<double> groundHeight(504 * 504);
        for (int x = 0; x < 504; x++)
        {
            for (int y = 0; y < 504; y++)
            {
                int idx = x * 504 + y;
                groundHeight[idx] = 0;
            }
        }
        // 这里的 504x504 是地形的尺寸
        auto ground = world_->addHeightMap(504, 504, 504, 504, 0, 0, groundHeight, "sand");
        // 设置地面外观
        ground->setAppearance("hidden");
        scenes_[3].push_back(ground);
        world_->setMaterialPairProp("sand", "steel", 0.8, 0.1, 0.001);
        server_->setMap("wheat");
    }

    // 清除当前场景
    void clearCurrentScene()
    {
        if (currentScene_ < scenes_.size())
        {
            for (auto *obj : scenes_[currentScene_])
            {
                world_->removeObject(obj);
            }
            scenes_[currentScene_].clear();
        }
    }

    // 切换到指定场景
    void switchToScene(int sceneId)
    {
        if (sceneId < 0 || sceneId >= 4)
            return;

        std::cout << "Switching from scene " << currentScene_ << " to scene " << sceneId << std::endl;

        // 清除当前场景
        clearCurrentScene();

        currentScene_ = sceneId;

        // 创建新场景
        switch (sceneId)
        {
        case 0:
            createHillScene();
            break;
        case 1:
            createLakeScene();
            break;
        case 2:
            createMountainScene();
            break;
        case 3:
            createWheatScene();
            break;
        }
    }

    // 添加机器人
    void addRobot()
    {
        /// 添加机器人
        robot_ = world_->addArticulatedSystem(binaryPath_.getDirectory() + "\\rsc\\aliengo\\aliengo.urdf");
        std::cout << "Successfully add robot!" << std::endl;
    }

    // 初始化机器人的状态
    void initializeRobot()
    {
        // 机器人控制器设置
        // 获取广义坐标维度、自由度（关节个数）
        Eigen::VectorXd jointNominalConfig(robot_->getGeneralizedCoordinateDim()), jointVelocityTarget(robot_->getDOF());
        // 位置 朝向 左前 右前 左后 右后
        jointNominalConfig << robotPosition_[0], robotPosition_[1], robotPosition_[2],
            robotOrientation_[0], robotOrientation_[1], robotOrientation_[2], robotOrientation_[3],
            0.03, 0.4, -0.8,
            -0.03, 0.4, -0.8,
            0.03, -0.4, 0.8,
            -0.03, -0.4, 0.8;
        // 所有关节目标速度设置为 0
        jointVelocityTarget.setZero();
        // 12 个关节的 PD 增益，对应 aliengo 的 12 个腿部关节
        // P = 100.0, D = 1.0 是一个常见的较硬的腿部控制参数
        Eigen::VectorXd jointPgain(robot_->getDOF()), jointDgain(robot_->getDOF());
        jointPgain.tail(12).setConstant(100.0);
        jointDgain.tail(12).setConstant(1.0);

        // 应用设置到机器人
        robot_->setGeneralizedCoordinate(jointNominalConfig);
        robot_->setGeneralizedForce(Eigen::VectorXd::Zero(robot_->getDOF()));
        robot_->setPdGains(jointPgain, jointDgain);
        robot_->setPdTarget(jointNominalConfig, jointVelocityTarget);
        robot_->setName("aliengo");

        std::cout << "Successfully initialized robot" << std::endl;
        showRobotState();
        server_->focusOn(robot_);
    }

    // 设置机器人的坐标和朝向
    void setRobotInitialState(raisim::Vec<2> &pos_xy, raisim::Vec<4> &ori)
    {
        // 机器人的位置(其中z不能指定，必须查阅高度图)
        robotPosition_[0] = pos_xy[0];
        robotPosition_[1] = pos_xy[1];
        robotPosition_[2] = getTerrainHeightAt(pos_xy[0], pos_xy[1]) + 1.0;

        robotOrientation_ = ori;
    }

    // 更新机器人的坐标和朝向
    void updateRobotState()
    {
        // 机器人的位置只更新x和y
        robotPosition_[0] = robot_->getBasePosition()[0];
        robotPosition_[1] = robot_->getBasePosition()[1];
        robotOrientation_ = robot_->getBaseOrientation();
        std::cout << "Successfully updated the robot state" << std::endl;
        showRobotState();
    }

    void showRobotState()
    {

        std::cout << "\tposition:" << robotPosition_[0] << ", " << robotPosition_[1] << ", " << robotPosition_[2] << std::endl;
        std::cout << "\torientation:" << robotOrientation_[0] << ", " << robotOrientation_[1] << ", " << robotOrientation_[2] << ", " << robotOrientation_[3] << ", " << std::endl;
    }

    // 添加场景物体
    void addObject()
    {
        // 添加一些标志物
        //  长方体x
        auto b_x = world_->addBox(252, 0.1, 0.1, 1);
        b_x->setPosition(raisim::Vec<3>{126, -0.1, 20});
        b_x->setBodyType(raisim::BodyType::STATIC);
        b_x->setAppearance("red");
        // 长方体y
        auto b_y = world_->addBox(0.1, 252, 0.1, 1);
        b_y->setPosition(raisim::Vec<3>{-0.1, 126, 20});
        b_y->setBodyType(raisim::BodyType::STATIC);
        b_y->setAppearance("green");
        // 长方体z
        auto b_z = world_->addBox(0.2, 0.2, 126, 1);
        b_z->setPosition(raisim::Vec<3>{0, 0, 83});
        b_z->setBodyType(raisim::BodyType::STATIC);
        b_z->setAppearance("blue");

        // 静态圆球用来找高度
        auto sp = world_->addSphere(0.1, 0.1);
        sp->setBodyType(raisim::BodyType::STATIC);
        sp->setAppearance("red");
        // double z = getTerrainHeightAt(20, 20);
        // sp->setPosition(raisim::Vec<3>{20, 20, z});
        sp->setPosition(robotPosition_[0], robotPosition_[1], robotPosition_[2]);
        scenes_[currentScene_].push_back(sp);

        // 随机添加一些障碍物
        int obstacleNum = 100;
        int obstacleInterval = 3;
        int obstacleAreaRadius = 15;
        int spaceAreaRadius = 2;
        Point2D obstacleAreaCenter = {robotPosition_[0], robotPosition_[1]};
        raisim::Mat<3, 3> inertia;
        inertia.setZero();
        raisim::Vec<3> com;
        com.setZero();
        auto points = generateScatteredPoints(obstacleAreaCenter, obstacleAreaRadius, obstacleNum, obstacleInterval);
        int idx = 0;
        std::cout << "Successfully generated " << points.size() << " obstacles" << std::endl;
        for (const auto &p : points)
        {
            if (distance(p, obstacleAreaCenter) < spaceAreaRadius)
            {
                continue;
            }
            raisim::Mesh *obstacleMesh;
            if (idx % 3 == 0)
            {
                obstacleMesh = world_->addMesh(binaryPath_.getDirectory() + "\\rsc\\objs\\Lowpoly_tree_sample.obj", 1, inertia, com, 0.1);
                obstacleMesh->setPosition(raisim::Vec<3>{p.x, p.y, getTerrainHeightAt(p.x, p.y)});
                obstacleMesh->setBodyType(raisim::BodyType::STATIC);
                Eigen::AngleAxisd rotation(M_PI / 2, Eigen::Vector3d::UnitX()); // 绕X轴 90°
                Eigen::Quaterniond quat(rotation);
                obstacleMesh->setOrientation(quat);
                obstacleMesh->setAppearance("green");
            }
            else if (idx % 3 == 1)
            {

                obstacleMesh = world_->addMesh(binaryPath_.getDirectory() + "\\rsc\\objs\\Rock.obj", 1, inertia, com, 0.5);
                obstacleMesh->setPosition(raisim::Vec<3>{p.x, p.y, getTerrainHeightAt(p.x, p.y) + 1});
                obstacleMesh->setBodyType(raisim::BodyType::STATIC);
                Eigen::AngleAxisd rotation(M_PI / 2, Eigen::Vector3d::UnitX()); // 绕X轴 90°
                Eigen::Quaterniond quat(rotation);
                obstacleMesh->setOrientation(quat);
                obstacleMesh->setAppearance("marble3");
            }
            else
            {
                obstacleMesh = world_->addMesh(binaryPath_.getDirectory() + "\\rsc\\objs\\stump_4.obj", 1, inertia, com, 0.04);
                obstacleMesh->setPosition(raisim::Vec<3>{p.x, p.y, getTerrainHeightAt(p.x, p.y)});
                obstacleMesh->setBodyType(raisim::BodyType::STATIC);
                Eigen::AngleAxisd rotation(M_PI / 2, Eigen::Vector3d::UnitX()); // 绕X轴 90°
                Eigen::Quaterniond quat(rotation);
                obstacleMesh->setOrientation(quat);
                obstacleMesh->setAppearance("wood2");
            }
            scenes_[currentScene_].push_back(obstacleMesh);
            idx++;
        }

        scenes_[currentScene_].push_back(b_x);
        scenes_[currentScene_].push_back(b_y);
        scenes_[currentScene_].push_back(b_z);
    }

    double getTerrainHeightAt(float worldX, float worldY)
    {   
        double offSet = 5.0;
        if (currentHeightMap_)
        {
            std::cout << "At world position (" << worldX << ", " << worldY << "), the height of the terrain is:" << currentHeightMap_->getHeight(worldX, worldY) << std::endl;
            return currentHeightMap_->getHeight(worldX, worldY) + offSet;
        }
        else
        {
            std::cout << "No heightmap, return default height: 1" << std::endl;
            return 0; // 返回默认高度
        }
    }
    struct Point2D
    {
        double x, y;
    };

    double distance(const Point2D &a, const Point2D &b)
    {
        return std::hypot(a.x - b.x, a.y - b.y);
    }

    // 生成满足最小距离约束的随机点
    std::vector<Point2D> generateScatteredPoints(Point2D center, double radius, int numPoints, double minDist)
    {
        std::vector<Point2D> points;
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<double> angle_dist(0, 2 * M_PI);
        std::uniform_real_distribution<double> radius_dist(0, radius);

        int attempts = 0;
        const int maxAttempts = numPoints * 100;

        while (points.size() < static_cast<size_t>(numPoints) && attempts < maxAttempts)
        {
            double angle = angle_dist(rng);
            double r = radius_dist(rng);
            Point2D p{
                center.x + r * std::cos(angle),
                center.y + r * std::sin(angle)};

            bool isValid = true;
            for (const auto &existing : points)
            {
                if (distance(p, existing) < minDist)
                {
                    isValid = false;
                    break;
                }
            }

            if (isValid)
            {
                points.push_back(p);
            }

            ++attempts;
        }

        return points;
    }

    int getCurrentScene() const { return currentScene_; }
    void focusOnRobot() { server_->focusOn(robot_); }
};

// 用于线程间通信的标志
std::atomic<bool> keyPressed(false);
std::atomic<char> keyInput('\0'); // 存储键盘输入的字符

// 键盘输入监听函数
void keyboardListener()
{
    while (true)
    {
        char key;
        key = std::cin.get(); // 阻塞直到有输入
        keyPressed = true;    // 设置输入标志
        keyInput = key;       // 保存输入的键
        std::cout << "Has received keyinput" << std::endl;
    }
}

int main(int argc, char *argv[])
{
    auto binaryPath = raisim::Path::setFromArgv(argv[0]);
    raisim::World::setActivationKey(binaryPath.getDirectory() + "\\rsc\\activation.raisim");

    /// 创建RaiSim世界
    raisim::World world;
    world.setTimeStep(0.005); // 每秒200步比较合适
    /// 启动服务器
    raisim::RaisimServer server(&world);
    /// 创建场景管理器
    SceneManager sceneManager(&world, &server, binaryPath);
    server.launchServer();

    // 初始化为第一个场景,调试用
    // sceneManager.switchToScene(0);
    std::vector<double> groundHeight(504 * 504);
    for (int x = 0; x < 504; x++)
    {
        for (int y = 0; y < 504; y++)
        {
            int idx = x * 504 + y;
            groundHeight[idx] = 0;
        }
    }
    auto ground = world.addHeightMap(504, 504, 504, 504, 0, 0, groundHeight, "sand");
    ground->setAppearance("wood1");

    sceneManager.addRobot();
    // 设置机器人的初始坐标
    raisim::Vec<2> pose;
    pose[0] = 10; // x
    pose[1] = 10; // y
    // 设置机器人的初始朝向（四元数）
    raisim::Vec<4> quaternion;
    quaternion[0] = 0.7071; // w
    quaternion[1] = 0.0;    // x
    quaternion[2] = 0.0;    // y
    quaternion[3] = 0.7071; // z
    sceneManager.setRobotInitialState(pose, quaternion);

    sceneManager.initializeRobot();

    // 先初始化机器人，再以机器人为中心添加障碍物
    sceneManager.addObject();

    // 场景切换设置
    std::cout << "* Multi-Scene Environment Loaded!" << std::endl;
    std::cout << "Scenes Available:" << std::endl;
    std::cout << "0:  Hill Scene" << std::endl;
    std::cout << "1:  Lake Scene" << std::endl;
    std::cout << "2:  Mountain Scene" << std::endl;
    std::cout << "3:  Wheat Scence" << std::endl;

    // 启动事件处理线程
    std::thread keyListenerThread(keyboardListener);
    bool isAsked = false;
    auto lastFocusTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    /// 仿真循环
    while (true)
    {
        if (!isAsked && !keyPressed)
        {
            std::cout << "* Press Enter to switch scence" << std::endl;
            isAsked = true;
        }
        if (keyPressed && keyInput == '\n')
        { // 如果是回车符
            if (ground)
            {
                world.removeObject(ground);
                ground = nullptr;
            }
            int nextScene = (sceneManager.getCurrentScene() + 1) % 4;
            nextScene = 0; // 调试，指定某个地图
            sceneManager.switchToScene(nextScene);
            // 更新并设置机器人的初始位置
            // sceneManager.updateRobotState();
            sceneManager.initializeRobot();
            sceneManager.addObject(); // 先初始化机器人再生成障碍物
            std::cout << "Successfully switched scence!" << std::endl;

            keyPressed = false;
            isAsked = false;
        }
        now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastFocusTime).count() >= 1)
        {
            // sceneManager.focusOnRobot();
            lastFocusTime = now; // 更新上次聚焦时间
        }
        RS_TIMED_LOOP(int(world.getTimeStep() * 1e6))
        server.integrateWorldThreadSafe();
    }
    server.killServer();
    // 等待事件处理线程结束
    keyListenerThread.join();
    return 0;
}