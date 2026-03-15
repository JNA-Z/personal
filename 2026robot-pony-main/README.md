# 2026robot
2026年机器人比赛



2026机器人旅游赛道小车外设：
1.灰度
2.CCD
3.WT901陀螺仪
4.LCD
5.Maxicam摄像头
6.TOF
7.红外
8.色标
9.电机 √
10.舵机 √
11.LED √
12.按键 √



目前架构设计如下。`util`层独立于其他，可以被其他层调用。`app`依赖`model`，`model`依赖`driver`和`board`。

![arch](./arch.png)





| 名字   | 说明                                                     |
| ------ | -------------------------------------------------------- |
| app    | 应用层，实现业务逻辑汇                                   |
| model  | 模型层，构造应用层所需对象，调用设备驱动层，提高简单接口 |
| board  | 板级适配层，直接依赖stm32的HAL                           |
| driver | 设备驱动层，以注入的平台接口做设备驱动实现               |
| util   | 通用的一些算法组件，不依赖于任何平台                     |



model层

chassis：聚合 Wheel，负责运动学解耦

localization：聚合传感器数据，提供各种信息

indicator：灯光提示



app层

navigation：针对地图的运行代码



navigation从localization拿到数据，然后通过chassis做行动，然后利用indicator做出一些信号提示。以电机控制为例，是2层环，model层提供的是速度环，是电机的环，而你要做巡线，那就是得在app，根据localization数据以后做闭环控制。
