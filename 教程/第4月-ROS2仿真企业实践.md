# 第4个月：ROS 2、仿真以及企业如何构建机器人

## 本月目标
使用 ROS 2 构建一个机器人，对其进行仿真，并使其能够绘制房间地图并自主导航。

> 💡 本月不需要任何硬件，只需要一台普通笔记本电脑，甚至不需要NVIDIA显卡。

---

## 1. ROS 2 版本选择

- **Jazzy Jalisco**（发布于2024年5月，支持到2029年5月，Ubuntu 24.04）← **从这里开始**
- **Lyrical Luth**（发布于2026年5月，支持到2031年5月，Ubuntu 26.04）← 等教程丰富后再过渡
- ❌ ROS 1（Noetic已于2025年5月EOL）：看到 catkin_make/roscore/rospy 立即关闭

---

## 2. ROS 2 核心概念

### 学习资源
1. **ROS 2 官方教程**（免费）
   - https://docs.ros.org/en/jazzy/Tutorials.html
2. **The Construct**（免费版，付费版€39.97/月起）
   - https://www.theconstruct.ai/
   - 浏览器内ROS环境，无需Ubuntu双系统
3. **Edouard Renard，《ROS 2入门教程》**（Udemy，促销$10–20）
   - https://www.udemy.com/course/ros2-for-beginners/
4. **关节机器人学，Josh Newans**（免费）
   - https://articulatedrobotics.xyz/tutorials/
5. **MOGI-ROS 大学课程**（免费，Apache 2.0）
   - https://github.com/orgs/MOGI-ROS/repositories
6. **自动 Addison**（免费）
   - https://automaticaddison.com/tutorials/

### 重点内容
- 节点、主题、服务、动作：选哪种
- 自定义消息和服务定义
- 参数和 YAML 配置
- Python 启动文件，传递参数和重新映射主题
- colcon 工作区和包布局
- ros2 bag 录制和回放
- ⚠️ DDS/QoS 设置：教程普遍讲得差，遇到"主题能发布但无法接收"先查QoS

### 练习任务
构建多节点系统：传感器发布节点 + 处理节点 + 服务配置节点 + 参数化聚合器，含自定义.msg和.srv文件，Python启动文件集成，ros2 bag录制回放。

---

## 3. URDF、TF 和机器人描述

### 学习资源
1. **关节机器人学，机器人坐标变换**（免费）
   - https://articulatedrobotics.xyz/category/coordinate-transforms-for-robotics
2. **Edouard Renard Level 2: TF, URDF, RViz, Gazebo**（Udemy $10–20）
   - https://www.udemy.com/course/ros2-tf-urdf-rviz-gazebo/
3. **官方 URDF 教程**（免费）
   - https://docs.ros.org/en/jazzy/Tutorials/Intermediate/URDF/Using-URDF-with-Robot-State-Publisher-cpp.html

### 常见坑
- 混淆 joint_state_publisher 和 robot_state_publisher
- 缺少惯性标签 → 机器人爆炸/沉入地板
- 缺少激光雷达坐标系静态变换 → SLAM失败
- 手动写400行XML → 用xacro宏

### 练习任务
用 Xacro 设计机器人：差动驱动底座 + 传感器桅杆 + 两自由度云台，正确惯性矩，独立碰撞/视觉几何。joint_state_publisher_gui 驱动，RViz 查看完整 TF 树。

---

## 4. 模拟

### Gazebo 版本
- **Gazebo Harmonic** ← Jazzy 配套
- Gazebo Classic（v1–11）已于2025年1月停止维护
- Humble ↔ Fortress / Kilted ↔ Ionic / Lyrical ↔ Jetty

### 学习资源
1. **Gazebo 文档和教程**（免费）
   - https://gazebosim.org/docs/latest/getstarted/
2. **MuJoCo**（免费开源，无需GPU）
   - https://mujoco.readthedocs.io/en/stable/overview.html
3. **NVIDIA Isaac Sim**（需RTX 4080+）
   - https://docs.isaacsim.omniverse.nvidia.com/6.0.0/installation/requirements.html

> ❌ 跳过 PyBullet（2022年后未维护）；Genesis v1.0刚发布，暂不用于作品集

### 练习任务
将 Xacro 机器人导入 Gazebo，添加激光雷达和相机插件，构建含障碍物的自定义 SDF 世界，验证 ROS 2 主题传感器数据，RViz 渲染。

---

## 5. ros2_control

### 学习资源
1. **ros2_control 文档**（免费）
   - https://control.ros.org/rolling/index.html
2. **关节机器人，ros2_control 在真实硬件上**（免费）
   - https://articulatedrobotics.xyz/tutorials/mobile-robot/applications/ros2_control-real/

### 练习任务
为机器人添加 `<ros2_control>` 标签，YAML 配置 diff_drive_controller 和 joint_state_broadcaster，Gazebo 键盘遥控。编写动作服务器按指令行驶距离并报告进度。

---

## 6. SLAM 和导航

### 学习资源
1. **Nav2 入门指南**（免费）
   - https://docs.nav2.org/rolling/getting_started/index.html
2. **Nav2 教程**（免费）
   - https://docs.nav2.org/rolling/tutorials/
3. **SLAM Toolbox**（免费开源）
   - https://github.com/SteveMacenski/slam_toolbox
4. **RTAB-Map for ROS 2**（免费开源）
   - https://github.com/introlab/rtabmap_ros

### 硬件层级
- $0：Gazebo 模拟激光雷达 ← 先在这里完成全部SLAM和Nav2课程
- $250–450：DIY（RPLIDAR C1 $69 + 树莓派 + 差速驱动底盘）
- $300–535：现成平台（Hiwonder MentorPi M1 $299.99、Waveshare UGV Rover $534.99）

### 练习任务
Gazebo 运行 SLAM Toolbox，远程操控保存地图，切换定位模式，RViz 发送 Nav2 目标。调整代价地图使机器人不裁剪拐角。录制自主导航视频。

---

## 第4个月里程碑

到本月底，你应该能够：
- [ ] 用 Python 和 C++ 编写 ROS 2 节点（主题、服务、动作）
- [ ] 在 Xacro 中描述机器人（正确坐标系、惯性、碰撞几何）
- [ ] 在 Gazebo 中模拟机器人，激光雷达和摄像头正常工作
- [ ] 配置 ros2_control 并通过控制器驱动机器人
- [ ] 使用 SLAM Toolbox 建图，Nav2 自主导航
- [ ] 诊断 TF 树损坏
