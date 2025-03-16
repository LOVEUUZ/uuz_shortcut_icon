# 一款随时随地通过快捷键呼唤出自定义抽屉的快捷工具

# v1.10
	优化右键菜单显示，避免超出窗口范围导致窗口隐藏.
	修复当点击新增文件或文件夹后，取消选择文件造成窗口无法关闭的问题
	qt更新至6.8.2

## 工具
* Qt: 6(低版本也行，开发的时候用的是6.4.3，后续因为环境问题更换为6.8.2)
* vs: 2022
* c++ >= 17

## 功能
✔️ 可将任意文件或任意文件夹通过放置于抽屉单机打开

✔️ 文件夹和文件通过默认应用打开，exe直接打开

✔️ 通过快速双击crtl呼出抽屉

✔️ 集成Everything，可以进行快速搜索

✔️ 自由拖拖拽图标

✔️ 可设置开机自启

✔️ 简单易用的日志消息

✔️ 支持多屏单独位置记忆

❌ 支持 中/英 语言轻松切换


## 使用
### 解压点击exe即可
### 如需自己编译，注意默认的是英语。中文找到对应的qm文件，然后在生成的exe同级目录下新建 translations 文件夹，将qm文件放入即可识别

#### 注意:  本程序执行路径下不要有中文(不影响程序内设置绑定快捷项目是否有中文)(qt的好像都这样,希望有好心人告诉我怎么解决)
#### 注意:  目前everything最大支持3k的搜索量，暂不支持排序
#### 注意:  需要自己安装Everything，如未安装无法使用
#### 注意:  如果遇到双击ctrl无法呼出的情况，点击右下角托盘，然后重新鼠标点击窗口外任意位置使其关闭

## 演示
### 1.快速呼出并启动快捷方式
![1_快速呼出并启动快捷方式](https://raw.githubusercontent.com/LOVEUUZ/res/refs/heads/main/uuz_shortcut_icon/1_%E5%BF%AB%E9%80%9F%E5%91%BC%E5%87%BA%E5%B9%B6%E5%90%AF%E5%8A%A8%E5%BF%AB%E6%8D%B7%E6%96%B9%E5%BC%8F.gif)

### 2.添加icon
![1_添加icon](https://github.com/LOVEUUZ/res/blob/main/uuz_shortcut_icon/1_%E6%90%9C%E7%B4%A2%E7%95%8C%E9%9D%A2%E6%93%8D%E4%BD%9C.gif?raw=true)
 
### 3.拖动与移除icon
![1_拖动与移除icon](https://raw.githubusercontent.com/LOVEUUZ/res/refs/heads/main/uuz_shortcut_icon/1_%E6%8B%96%E5%8A%A8%E4%B8%8E%E7%A7%BB%E9%99%A4icon.gif)
 
### 4.everythin搜索
![1_everythin搜索](https://raw.githubusercontent.com/LOVEUUZ/res/refs/heads/main/uuz_shortcut_icon/1_everythin%E6%90%9C%E7%B4%A2.gif)

### 5.搜索界面操作
![1_搜索界面操作](https://github.com/LOVEUUZ/res/blob/main/uuz_shortcut_icon/1_%E6%90%9C%E7%B4%A2%E7%95%8C%E9%9D%A2%E6%93%8D%E4%BD%9C.gif?raw=true)

### 6.设置界面演示
![1_设置界面演示](https://github.com/LOVEUUZ/res/blob/main/uuz_shortcut_icon/1_%E8%AE%BE%E7%BD%AE%E7%95%8C%E9%9D%A2%E6%BC%94%E7%A4%BA.gif?raw=true)


## 下载地址
https://github.com/LOVEUUZ/uuz_shortcut_icon/releases