# RaiSim 材质系统详解

## 你的问题分析

在 `hill1.cpp` 中：
```cpp
auto heightmap = world.addHeightMap(...);
heightmap->setAppearance("hidden");  // 只设置了外观为隐藏
server.setMap("hill1");              // 视觉地图来自预设
```

**问题：** 代码确实只加载了高度图的几何形状，材质和视觉效果来自 `server.setMap("hill1")` 的预设。

## 材质设置的完整方法

### 🎯 方法1: 在创建物体时指定材质

```cpp
// 创建高度图时指定材质
auto heightmap = world.addHeightMap("terrain.png", 0, 0, 100, 100, 0.1, 0.0, "grass");

// 创建其他物体时指定材质
auto box = world.addBox(1.0, 1.0, 1.0, 10.0, "steel");        // 钢铁材质
auto sphere = world.addSphere(0.5, 5.0, "rubber");             // 橡胶材质
auto ground = world.addGround(0.0, "concrete");                // 混凝土地面
```

### 🎯 方法2: 设置材质对的物理属性

```cpp
// setMaterialPairProp(材质1, 材质2, 摩擦系数, 弹性系数, 阈值)
world.setMaterialPairProp("grass", "steel", 0.8, 0.1, 0.001);
world.setMaterialPairProp("rubber", "concrete", 1.2, 0.6, 0.001);
world.setMaterialPairProp("steel", "steel", 0.6, 0.2, 0.001);
```

### 🎯 方法3: 设置默认材质属性

```cpp
// 设置所有未指定材质对的默认属性
world.setDefaultMaterial(0.8, 0.0, 0.001);  // 摩擦, 弹性, 阈值
```

### 🎯 方法4: XML配置文件

```xml
<?xml version="1.0" ?>
<raisim version="1.0">
    <objects>
        <heightmap name="terrain" material="grass">
            <file path="hill1.png"/>
            <center x="0" y="0"/>
            <size x="504" y="504"/>
            <scale height="0.007837" offset="-256.0"/>
        </heightmap>
    </objects>
    
    <material>
        <default friction="0.8" restitution="0" restitution_threshold="0"/>
        <pair_prop name1="grass" name2="steel" friction="0.8" restitution="0.1" restitution_threshold="0.001"/>
        <pair_prop name1="grass" name2="rubber" friction="1.2" restitution="0.3" restitution_threshold="0.001"/>
    </material>
</raisim>
```

## 材质参数详解

| 参数 | 含义 | 范围 | 效果 |
|------|------|------|------|
| **friction** | 摩擦系数 | 0.0-2.0+ | 控制滑动阻力 |
| **restitution** | 弹性系数 | 0.0-1.0 | 0=完全非弹性, 1=完全弹性 |
| **threshold** | 弹性阈值 | 0.001+ | 低于此速度不产生弹性 |

## 常见材质预设值

```cpp
// 🌱 草地/土壤
world.setMaterialPairProp("grass", "steel", 0.8, 0.1, 0.001);

// 🧊 冰面 (低摩擦)
world.setMaterialPairProp("ice", "steel", 0.1, 0.2, 0.001);

// 🪨 岩石 (高摩擦, 低弹性)
world.setMaterialPairProp("rock", "steel", 1.0, 0.05, 0.001);

// 🏀 橡胶 (高弹性)
world.setMaterialPairProp("rubber", "concrete", 1.2, 0.8, 0.001);

// 🛢️ 油面 (极低摩擦)
world.setMaterialPairProp("oil", "steel", 0.05, 0.1, 0.001);
```

## 视觉外观 vs 物理材质

### 🎨 外观设置 (只影响视觉)
```cpp
heightmap->setAppearance("grass");     // 绿色草地纹理
box->setAppearance("red");             // 红色
sphere->setAppearance("metal");        // 金属纹理
```

### ⚙️ 物理材质 (影响仿真)
```cpp
auto box = world.addBox(1,1,1, 10, "steel");  // 物理行为像钢铁
world.setMaterialPairProp("steel", "ground", 0.6, 0.2, 0.001);
```

## 完整示例：hill1增强版

我已经创建了 `hill1_with_materials.cpp`，展示了：
1. ✅ 高度图材质设置
2. ✅ 多种材质物体
3. ✅ 材质对属性配置
4. ✅ 视觉外观设置
5. ✅ 测试环境

## 原始hill1.cpp的问题

```cpp
// 原代码问题
heightmap->setAppearance("hidden");    // ❌ 隐藏了高度图
server.setMap("hill1");                // ✅ 视觉来自预设地图

// 改进建议
auto heightmap = world.addHeightMap(..., "grass");  // ✅ 指定材质
world.setMaterialPairProp("grass", "steel", 0.8, 0.1, 0.001);  // ✅ 配置物理属性
heightmap->setAppearance("grass");     // ✅ 设置草地外观
```

**总结：** 原代码确实只是加载了几何高度图，材质和纹理完全依赖预设。要完全控制材质，需要明确指定材质名称并配置物理属性！
