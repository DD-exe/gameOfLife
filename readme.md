## gameOfLife
<a href="https://github.com/DD-exe/gameOfLife/blob/master/readme_EN.md">English Version</a><br>
项目旨在实现一图形化Windows程序，用于拓展参数的康威生命游戏的可视化模拟。<br>
在康威生命游戏的故事中，往往会强调其初始参数的重要性——例如，过弱的存活条件参数会使其退化为无序扩张，过强的存活条件会让模拟一片死寂。然而，大多数可用的生命游戏模拟器不允许自由调整这些参数，限制了用户探索和理解系统行为的能力。<br>
在本项目中，我们引入了对生命游戏参数的调整功能，并让其以一可接受的代价进行。现在，模拟不再局限于经典的“最佳”参数——您可以随时修改它们，即使在模拟过程中也可以。<br>
项目同样包含其他生命游戏可视化模拟中的常见功能，例如可调地图大小、可调迭代速度。<br>
以下是项目的源代码结构：
```
gameOfLife
│  about.cpp            //用于实现“关于”对话框消息处理
│  downloads.cpp        //用于实现内置的模拟结果保存功能
│  gameOfLife.cpp       /*程序主框架
│                         用于创建主窗口和维护消息循环*/
│  life.cpp             //用于实现生命游戏规则下数据的迭代
│  tools.cpp            //用于实现其他部分用到的小工具
│  gameOfLife.h         //gameOfLife.cpp所需头文件
│  framework.h          //项目框架头文件
│  resource.h           //项目资源接口定义
│  targetver.h
│  gameOfLife.rc        //项目资源
│
└─image                //图标
        lin.bmp
        OIP-C.bmp
        OIP-C.ico
        small.ico
```