# RaiSim 环境制作完全指南

## 概述
RaiSim 提供多种环境制作方法，从简单的高度图到复杂的程序化场景。本指南将详细介绍各种环境制作技术。

## 1. 高度图 (HeightMap) 环境

### 1.1 静态高度图
```cpp
// 从PNG文件加载高度图
auto heightMap = world.addHeightMap("../rsc/heightmap.png", 
                                   0, 0,           // center x, y
                                   200, 200,       // x-size, y-size
                                   0.005,          // height scale
                                   -10.0);         // height offset
```

### 1.2 程序化生成高度图
```cpp
std::vector<double> terrainHeight(10000);  // 100x100 grid
std::vector<raisim::ColorRGB> terrainColors(10000);

for (int x = 0; x < 100; x++) {
    for (int y = 0; y < 100; y++) {
        int idx = x * 100 + y;
        
        // 使用多层噪声生成地形
        double height = 2.0 * std::sin(x * 0.1) * std::cos(y * 0.1) +     // 大型山丘
                       1.0 * std::sin(x * 0.3) * std::sin(y * 0.2) +      // 中型起伏
                       0.5 * std::cos(x * 0.5) * std::cos(y * 0.4);       // 小型细节
        
        terrainHeight[idx] = height;
        
        // 基于高度的颜色映射
        if (height < -1.0) {
            terrainColors[idx] = {0, 100, 255};    // 蓝色 (水)
        } else if (height < 1.0) {
            terrainColors[idx] = {0, 255, 100};    // 绿色 (草地)
        } else {
            terrainColors[idx] = {139, 69, 19};    // 棕色 (山脉)
        }
    }
}

auto heightMap = world.addHeightMap(100, 100, 20.0, 20.0, 0.0, 0.0, terrainHeight);
heightMap->setColor(terrainColors);
```

### 1.3 动态高度图
```cpp
// 在仿真循环中更新地形
if (i % 100 == 0) {  // 每100步更新一次
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
```

## 2. 几何体构建环境

### 2.1 基础几何体
```cpp
// 添加各种几何体
auto box = world.addBox(1.0, 2.0, 0.5, 1.0);           // 长方体
auto sphere = world.addSphere(0.5, 1.0);               // 球体
auto cylinder = world.addCylinder(0.3, 2.0, 1.0);      // 圆柱体
auto capsule = world.addCapsule(0.2, 1.5, 1.0);        // 胶囊体

// 设置位置和外观
box->setPosition(raisim::Vec<3>{0, 0, 1});
box->setAppearance("red");
```

### 2.2 复杂结构
```cpp
// 创建桥梁结构
auto bridge_support1 = world.addCylinder(0.5, 8.0, 1.0);
bridge_support1->setPosition(raisim::Vec<3>{-5.0, 0.0, 4.0});
bridge_support1->setAppearance("brown");

auto bridge_deck = world.addBox(12.0, 2.0, 0.2, 1.0);
bridge_deck->setPosition(raisim::Vec<3>{0.0, 0.0, 8.2});
bridge_deck->setAppearance("wood");

// 创建楼梯
for (int i = 0; i < 8; i++) {
    auto step = world.addBox(2.0, 2.0, 0.2, 1.0);
    step->setPosition(raisim::Vec<3>{-8.0 + i * 0.5, 3.0, i * 1.0 + 0.5});
    step->setAppearance("concrete");
}
```

## 3. 材质系统

### 3.1 材质定义
```cpp
// 创建材质
auto material = world.getMaterial("ice");
material.setFrictionCoefficient(0.1);      // 摩擦系数
material.setRestitutionCoefficient(0.9);   // 弹性系数
material.setDensity(920.0);                // 密度

// 应用材质到物体
auto icebox = world.addBox(1.0, 1.0, 1.0, material);
```

### 3.2 不同材质类型
```cpp
// 金属材质
auto metal = world.getMaterial("metal");
metal.setFrictionCoefficient(0.7);
metal.setRestitutionCoefficient(0.3);

// 橡胶材质  
auto rubber = world.getMaterial("rubber");
rubber.setFrictionCoefficient(1.2);
rubber.setRestitutionCoefficient(0.8);
```

## 4. XML场景定义

### 4.1 创建XML场景文件
```xml
<?xml version="1.0" ?>
<raisim version="1.0">
  <objects>
    <!-- 地面 -->
    <ground height="0.0" appearance="grass"/>
    
    <!-- 静态物体 -->
    <box mass="0" dim="10 1 2" pos="5 0 1" appearance="wall"/>
    <cylinder mass="0" radius="0.5" height="3" pos="0 5 1.5" appearance="pillar"/>
    <sphere mass="0" radius="1" pos="-5 -5 1" appearance="rock"/>
    
    <!-- 动态物体 -->
    <box mass="1" dim="1 1 1" pos="0 0 3" appearance="crate"/>
    
    <!-- 机器人 -->
    <articulated_system urdf_path="../rsc/anymal/anymal.urdf" 
                        initial_joint_config="0 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0"/>
  </objects>
</raisim>
```

### 4.2 加载XML场景
```cpp
world.addObjectFromXML("../rsc/custom_scene.xml");
```

## 5. RaiSimUnity 可视化

### 5.1 基础设置
```cpp
raisim::RaisimServer server(&world);
server.setMap("simple");           // 使用简单背景
// server.setMap("mountain1");     // 使用山地背景
// server.setMap("office1");       // 使用办公室背景
server.focusOn(robot);
server.launchServer();
```

### 5.2 自定义场景映射
```cpp
// 设置自定义地图文件
server.setMap("custom_map");  // 需要对应的地图文件

// 或使用默认背景
server.setMap("simple");
```

## 6. 动态环境

### 6.1 移动平台
```cpp
std::vector<raisim::SingleBodyObject*> movingPlatforms;

// 在仿真循环中更新位置
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
```

### 6.2 粒子系统
```cpp
// 创建下落的碎片
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> pos_dist(-25.0, 25.0);
std::uniform_real_distribution<> size_dist(0.1, 0.8);

for (int i = 0; i < 50; i++) {
    double size = size_dist(gen);
    auto rock = world.addSphere(size, 0.5);
    rock->setPosition(raisim::Vec<3>{pos_dist(gen), pos_dist(gen), 10.0});
    rock->setLinearVelocity(raisim::Vec<3>{
        (gen() % 20 - 10) * 0.1, 
        (gen() % 20 - 10) * 0.1, 
        -5.0
    });
}
```

## 7. 高级技巧

### 7.1 碰撞检测配置
```cpp
// 设置碰撞组
world.setContactSolverType(raisim::ContactSolverType::BISECTION);
world.setMaterialPairProp("robot", "ground", 0.8, 0.0, 0.001);
```

### 7.2 性能优化
```cpp
// 设置仿真参数
world.setTimeStep(0.001);
world.setERP(0.2, 0.2);           // Error Reduction Parameter
world.setGravity({0, 0, -9.81});

// 启用多线程
world.setNumberOfThreads(4);
```

## 8. 关于Unreal Engine场景

### 当前限制
RaiSim目前主要通过以下方式支持可视化：
1. **RaiSimUnity**: 内置的Unity可视化器，支持实时渲染
2. **高度图**: 可以动态更新的地形系统
3. **几何体**: 基础和复合几何体构建

### Unreal Engine集成
目前RaiSim没有直接的Unreal Engine场景编辑器，但可以通过以下方式实现复杂场景：

1. **程序化生成**: 使用代码创建复杂环境
2. **高度图**: 从外部工具(如World Machine、Blender)导出高度图
3. **XML定义**: 通过XML文件定义复杂场景布局
4. **网格导入**: 通过URDF/SDF文件导入复杂几何体

### 推荐工作流程
```
外部建模工具 (Blender/Maya) 
    ↓ 导出高度图/几何体
RaiSim 程序化组装
    ↓ 实时仿真
RaiSimUnity 可视化
```

## 示例项目

完整的高级环境示例可以在 `examples/src/maps/advanced_custom_environment.cpp` 中找到，包含：
- 程序化地形生成
- 复杂结构构建
- 动态环境元素
- 材质系统应用
- 性能优化技巧

这个指南应该能帮你创建从简单到复杂的各种环境！
