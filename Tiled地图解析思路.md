# Tiled地图解析思路

## 整体流程

- 清理之前状态并设置当前地图路径
- 加载并解析JSON文件
- 解析基本地图信息（尺寸、瓦片大小等）
- 验证地图数据有效性
- 加载所有tileset数据
- 加载所有图层（imagelayer、tilelayer、objectgroup）

## 图块集加载

- 从主地图文件中获取tilesets数组
- 遍历每个tileset，通过source字段加载外部tileset文件
- 保存firstgid和tileset数据的映射关系供后续gid查找使用

## 图层加载

### 图像图层 (imagelayer)

- 获取图像路径、偏移量、视差因子、重复属性等字段
- 创建包含ParallaxComponent的游戏对象

### 瓦片图层 (tilelayer)

- 获取data数组中的gid数据
- 为每个gid查找对应的TileInfo（包含Sprite和TileType）
- 创建包含TileLayerComponent的游戏对象

### 对象图层 (objectgroup)

- 遍历objects数组中的每个对象
- 对于有gid的对象，创建游戏对象并设置相应组件
- 对于无gid的对象，当前版本仅记录日志（未来处理自定义形状）

## 对象创建流程

对于对象图层中的对象：

- 通过gid获取TileInfo（包含sprite和tile type）
- 解析对象变换信息（位置、缩放、旋转）
- 创建游戏对象并添加TransformComponent和SpriteComponent
- 设置碰撞组件（基于tile type或自定义碰撞体）
- 应用对象属性（标签、重力、动画、生命值等）

## 地图自定义属性约定

- 限瓦片层
  - unisolid (bool) : 单向平台
  - ladder (bool) : 梯子
  - slope (string) : 斜坡图块，格式"x_y"。x表示左边高度，y表示右边高度。范围0~1。（如果是2则表示1/2高度）

- 限对象层
  - tag (string) : 对象类型，例如player,enemy,item
  - gravity (bool) : 是否使用重力（有此字段则默认添加 PhysicsComponent）
  - health (int) : 添加生命组件，生命值为 value
  - animation (json_string) : 添加动画组件，json格式为:

    ```json
    {
      "name1" : {"duration" : int, "row" : int, "frames": array(int)},
      "name2" : ...
    } 
    ```

    含义为:

    ```json
    { "动画名" : {"帧持续时间" : ms, "行" : (从0开始数), "帧序列（列）": [0,1,2...]} }
    ```

    要求每个动画序列帧必须在同一行。

  - sound (json_string) : 添加音频组件，json格式为:

    ```json
    {
      "id1" : "file_path1",
      "id2" : ...
    } 
    ```

- 两层皆可
  - solid (bool) : 静止不可进入的图块/物体
  - hazard (bool) : 对玩家造成伤害的瓦片

## 自定义属性处理

支持的自定义属性：

- tag: 对象标签
- gravity: 是否使用重力
- health: 生命值
- animation: 动画信息（JSON格式）
- solid: 静止不可进入的图块/物体
- hazard: 对玩家造成伤害的瓦片
- unisolid: 单向平台
- ladder: 梯子
- slope: 斜坡图块

## 瓦片类型识别

- SOLID: 固体类型，不可穿越
- HAZARD: 危险类型，会对玩家造成伤害
- UNISOLID: 单向平台
- LADDER: 梯子
- SLOPE_X_Y: 各种斜坡类型
- NORMAL: 普通类型
