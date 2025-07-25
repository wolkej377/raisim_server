# RaiSim 多场景实现方法完全指南

## 🎯 你的问题解答：绝对不是只能一个场景！

RaiSim 提供了多种实现多场景的方法，从简单的场景切换到复杂的多区域环境。

## 📋 多场景实现方法对比

| 方法 | 复杂度 | 优势 | 适用场景 |
|------|---------|------|----------|
| **场景切换** | ⭐⭐ | 内存效率高、性能好 | 不同环境测试 |
| **多区域环境** | ⭐⭐⭐ | 连续体验、真实感强 | 大世界探索 |
| **动态场景管理** | ⭐⭐⭐⭐ | 完全可控、灵活 | 高级仿真需求 |
| **XML配置** | ⭐⭐ | 易于配置、重用性高 | 标准化场景 |

## 🔧 实现方法详解

### 方法1: 场景切换 (Scene Switching)

**原理：** 动态删除和创建场景对象

```cpp
// 清除当前场景
void clearScene() {
    for (auto* obj : currentSceneObjects) {
        world->removeObject(obj);
    }
    currentSceneObjects.clear();
}

// 切换到新场景
void switchToScene(int sceneId) {
    clearScene();
    switch(sceneId) {
        case 0: createMountainScene(); break;
        case 1: createUrbanScene(); break;
        case 2: createDesertScene(); break;
    }
}
```

**示例文件：** `multi_scene_environment.cpp` - 自动切换4个不同场景

### 方法2: 多区域环境 (Multi-Zone Environment)

**原理：** 在同一个世界中创建多个相邻区域

```cpp
// 区域1: 山地 (-100,-100) to (0,0)
auto mountainZone = world.addHeightMap(100, 100, 100, 100, -50, -50, mountainHeight, "grass");

// 区域2: 城市 (0,-100) to (100,0)
auto urbanGround = world.addBox(100, 100, 1.0, 1000.0, "concrete");
urbanGround->setPosition(raisim::Vec<3>{50, -50, -0.5});

// 区域3: 沙漠 (-100,0) to (0,100)
auto desertZone = world.addHeightMap(100, 100, 100, 100, -50, 50, desertHeight, "sand");

// 区域4: 冰雪 (0,0) to (100,100)
auto iceZone = world.addHeightMap(100, 100, 100, 100, 50, 50, iceHeight, "ice");
```

**示例文件：** `multi_zone_environment.cpp` - 4个区域连接的大世界

### 方法3: 用户选择场景 (User Choice)

**原理：** 运行时让用户选择场景

```cpp
std::cout << "Choose a scene:" << std::endl;
std::cout << "1 - Mountain Environment" << std::endl;
std::cout << "2 - Urban Environment" << std::endl;
std::cout << "3 - Desert Environment" << std::endl;

int choice;
std::cin >> choice;

switch(choice) {
    case 1: createScene1(world); break;
    case 2: createScene2(world); break;
    case 3: createScene3(world); break;
}
```

**示例文件：** `simple_multi_scene.cpp` - 简单的用户选择界面

## 🌍 场景类型示例

### 🏔️ 山地环境
- **地形：** 高度图生成起伏地形
- **材质：** 草地 (friction=0.8, restitution=0.1)
- **物体：** 岩石、巨石
- **特点：** 高摩擦、复杂地形

### 🏙️ 城市环境
- **地形：** 平坦混凝土地面
- **材质：** 混凝土 (friction=0.7, restitution=0.2)
- **物体：** 建筑、汽车、道路
- **特点：** 结构化环境、障碍物多

### 🏜️ 沙漠环境
- **地形：** 沙丘起伏地形
- **材质：** 沙子 (friction=0.3, restitution=0.05)
- **物体：** 仙人掌、岩石
- **特点：** 低摩擦、松软地面

### ❄️ 冰雪环境
- **地形：** 光滑冰面
- **材质：** 冰 (friction=0.05, restitution=0.9)
- **物体：** 冰柱、雪堆
- **特点：** 极低摩擦、高弹性

## ⚙️ 技术要点

### 材质管理
```cpp
// 为每个区域设置不同材质属性
world.setMaterialPairProp("grass", "steel", 0.8, 0.1, 0.001);      // 山地
world.setMaterialPairProp("concrete", "steel", 0.7, 0.2, 0.001);   // 城市
world.setMaterialPairProp("sand", "steel", 0.3, 0.05, 0.001);      // 沙漠
world.setMaterialPairProp("ice", "steel", 0.05, 0.9, 0.001);       // 冰雪
```

### 性能优化
```cpp
// 对于大场景，可以使用视锥剔除
server.lockVisualizationServerMutex();
// 只更新可见区域
server.unlockVisualizationServerMutex();

// 分层加载
if (distanceToRobot < loadRadius) {
    loadDetailedObjects();
} else {
    loadSimplifiedObjects();
}
```

### 动态加载
```cpp
// 检测机器人位置，动态加载场景
auto pos = robot->getGeneralizedCoordinate();
double x = pos[0], y = pos[1];

if (x < 0 && y < 0) {
    // 在山地区域，加载山地细节
    loadMountainDetails();
} else if (x >= 0 && y < 0) {
    // 在城市区域，加载城市细节
    loadUrbanDetails();
}
```

## 🎮 运行示例

### 1. 自动场景切换
```bash
cd examples/src/maps
g++ -std=c++14 multi_scene_environment.cpp -o multi_scene -lraisim
./multi_scene
```

### 2. 用户选择场景
```bash
g++ -std=c++14 simple_multi_scene.cpp -o simple_multi -lraisim
./simple_multi
# 然后输入 1, 2, 或 3 选择场景
```

### 3. 大世界多区域
```bash
g++ -std=c++14 multi_zone_environment.cpp -o multi_zone -lraisim
./multi_zone
# 机器人可以在4个区域间自由移动
```

## 💡 高级技巧

### 场景预加载
```cpp
// 预先创建所有场景对象但设为不可见
std::vector<std::vector<raisim::Object*>> preloadedScenes(4);
for (int i = 0; i < 4; i++) {
    createScene(i, preloadedScenes[i]);
    setSceneVisibility(preloadedScenes[i], false);
}

// 快速切换只需改变可见性
void fastSwitch(int sceneId) {
    setSceneVisibility(preloadedScenes[currentScene], false);
    setSceneVisibility(preloadedScenes[sceneId], true);
    currentScene = sceneId;
}
```

### 渐进式加载
```cpp
// 根据距离渐进加载细节
float distanceToRobot = calculateDistance(robot, sceneCenter);
if (distanceToRobot < 50.0f) {
    loadHighDetailObjects();
} else if (distanceToRobot < 100.0f) {
    loadMediumDetailObjects();
} else {
    loadLowDetailObjects();
}
```

## 📁 提供的示例文件

1. **`multi_scene_environment.cpp`** - 完整的场景管理系统
2. **`simple_multi_scene.cpp`** - 简单的用户选择界面
3. **`multi_zone_environment.cpp`** - 大世界多区域环境
4. **`hill1_with_materials.cpp`** - 增强的单场景示例

**总结：RaiSim 完全支持多场景！你可以实现场景切换、多区域环境、动态加载等各种复杂的多场景需求。** 🚀
