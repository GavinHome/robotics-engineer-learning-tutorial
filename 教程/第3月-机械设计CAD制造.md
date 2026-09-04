# 第3个月：机械设计、CAD 和自行制造零件

## 本月目标
使用 CAD 设计零件，制造并使其装配到位。

---

## 1. CAD

### 工具选择决策
- **Onshape Free**：浏览器运行，公开设计，适合乐于公开的创客
  - https://learn.onshape.com/learning-paths/onshape-fundamentals-cad
- **Fusion 360 Personal**：非商业用途免费三年，年收入<$1000
  - https://productdesignonline.com/learn-autodesk-fusion-360-in-30-days-official-course/
- **FreeCAD 1.1+**：完全免费，云端CAD替代方案
  - https://www.youtube.com/@MangoJellySolutions
- **SOLIDWORKS for Makers**：$48/年，行业标准

### 学习资源
1. **Onshape 学习中心，基础知识：CAD**（免费）
2. **在线产品设计课程，30天学Fusion**（免费）
3. **MangoJelly Solutions FreeCAD 教程**（免费）
4. **Protolabs Network，面向3D打印的设计**（免费）
   - https://www.hubs.com/knowledge-base/design-for-3d-printing/

### 重点内容
- 完全约束草图
- 参数化设计由变量驱动
- 装配和配合：旋转关节、滑动关节、固定关节
- 根据实际硬件数据手册设计
- 导出STEP用于共享，STL用于打印

### 练习任务
根据伺服电机数据手册图纸（非目测），设计能固定伺服电机的支架（含螺丝孔和轴间隙），打印测试。

---

## 2. 3D打印

### 打印机价格（已核实，2026年9月）
- Creality Ender-3 V3 SE：$199
  - https://store.creality.com/products/ender-3-v3-se-3d-printer
- Bambu Lab A1 mini：$219.99
  - https://www.bestbuy.com/product/bambu-lab-a1-mini-3d-printer-silver/CZTZV9ZGGV
- Bambu Lab A1：$299.99
  - https://www.bestbuy.com/product/bambu-lab-a1-3d-printer-silver/CZW2ZH33H4

### 耗材选择
- **PLA/PLA+**：原型支架、夹具、SO-101机械臂（填充15%，层厚0.2mm）
- **PETG**：底盘板、变速箱外壳、伺服支架
- **ABS/ASA**：高温电机附近、户外探测车
- **TPU**：脚部、缓冲垫、柔性抓握手指

### 学习资源
1. **OrcaSlicer 校准维基**（免费）
   - https://github.com/OrcaSlicer/OrcaSlicer/wiki/Calibration
2. **教学技术 3D 打印机校准**（免费、交互式）
   - https://teachingtechyt.github.io/calibration.html
3. **CNC厨房**（免费）
   - https://www.youtube.com/@CNCKitchen
4. **间隙和公差 3D 打印机量规**（免费 STL）
   - https://www.printables.com/model/57067-clearance-and-tolerance-3d-printer-gauge

### 买不起打印机的替代方案
- Fab Lab（全球约2,875个）：https://fablabs.io/labs
- 公共图书馆创客空间
- Craftcloud（95个国家）：https://craftcloud3d.com/
- JLC3DP（$1/零件起，3天）：https://jlc3dp.com/

### 练习任务
打印公差规，记录实际间隙值；设计并打印无需胶水的两件式卡扣式 ESP32 外壳，反复调试直到正确卡扣。

---

## 3. 执行器、传动装置

### 重点内容
- 齿轮比及速度-扭矩权衡
- 反冲（ backlash ）及其对定位精度的影响
- 轴承选择和预紧力
- 皮带/齿轮/直接传动
- 廉价伺服塑料齿轮组是最先损坏的部件

### 练习任务
设计并打印简单的行星减速器或摆线减速器（NEMA17步进或业余电机），测量反冲并重新设计减小反冲。

参考：
- https://www.instructables.com/OpenCycloid-3D-printed-Open-Source-Robotic-Actuato/

---

## 4. 制造一个真正的机器人手臂：SO-101

### SO-101 简介
由 TheRobotStudio 和 Hugging Face 联合开发的开源五自由度机械臂及夹爪，主从式遥控。

### 采购选项及价格
- **Seeed Studio SO-ARM101 Pro 伺服套件**：$277.99（含电机和控制板，不含印刷件）
  - https://www.seeedstudio.com/SO-ARM101-Low-Cost-AI-Arm-Kit-Pro-p-6427.html
- **Seeed Studio 打印零件套装**：$30.99
  - https://www.seeedstudio.com/SO-ARM101-3D-printed-Enclosure-p-6428.html
- **Robonine 全套套件**：$349（从特拉华州发货）
  - https://robonine.com/shop/so-arm101-black-robotic-arm-kit/
- **自己打印 + 买零件**：单个从动臂约$122
- **EEZYbotARM MK2**（$50–80，免费STL）：https://www.thingiverse.com/thing:1454048
- **Hiwonder xArm 1S**：$199.99（带智能总线伺服）

### 练习任务
1. 搭建 SO-101 机械臂，校准所有舵机，用主控臂远程操控跟随臂
2. 用 TPU 设计并打印自定义夹爪手指替换原装
3. 在三个不同形状物体上测试

---

## 第3个月里程碑

到本月底，你应该能够：
- [ ] 在CAD中根据数据手册建模零件，创建完全约束草图
- [ ] 提供通过测量得出的打印机实际间隙数值
- [ ] 专门为FDM设计零件（考虑取向、悬垂、层间粘合）
- [ ] 为特定零件选择PLA/PETG/ABS/TPU并说明理由
- [ ] 解释什么是反弹并用构建物演示
- [ ] 展示组装、校准并用自定义零件改造过的可运行机器人手臂
