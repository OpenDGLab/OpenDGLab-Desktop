# OpenDGLab Desktop
OpenDGLab Desktop 是一个使用 OpenDGLab Core 和 Qt5 编写的桌面客户端。  
OpenDGLab Desktop 的目标是在 Windows、Linux、MacOS 上提供对多 DG-Lab 设备的统一化支持。  
使用 OpenDGLab Desktop 您可以在支持蓝牙 BLE 的电脑上同时管理多个 DG-Lab 设备。给您更加优质的使用体验。  
支持 OpenDGLab OpenProtocol 协议，允许第三方程序接入。提供更多可能。

## 编译
### 获取 OpenDGLab 必要文件
下载并编译 OpenDGLab Core 获取您对应架构的 Core 文件。 

从 OpenDGLab OpenProtocol 获取 app.proto 文件放入 proto 文件夹中。  

依赖 libprotobuf 请事先安装。

### 编译 OpenDGLab Desktop (Windows)
Windows 编译请使用 `lib /def:libopendglab.def /out:libopendglab.lib /machine:x64` 或 `lib /def:libopendglab.def /out:libopendglab.lib /machine:x86` 获取对应的 .lib 文件。  
将 lib dll 文件放入 opendglab-core/bin 文件夹，将 h 文件放入 opendglab-core/header 文件夹。  

```shell
mkdir build
cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=MinSizeRel ..
cmake --build . --target all
```

在 Build 文件夹下可以找到编译好的对应版本的 zip 压缩文件。

### 编译 OpenDGLab Desktop (Linux)
将 OpenDGLab-Core 编译出的 a 文件放入 opendglab-core/bin 文件夹，将 h 文件放入 opendglab-core/header 文件夹。  
如果需要打包未 AppImage 请安装 linuxdeployqt
```shell
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
cmake --build . --target all
```
编译后会生成 OpenDGLab-Desktop 可执行文件，您需要使用 `sudo setcap CAP_NET_ADMIN+ep OpenDGLab-Desktop` 和 `chmod +x OpenDGLab-Desktop` 来对其添加权限。

如果需要生成 AppImage 则运行
```shell
mkdir appImage
cp ../external/OpenDGLab.desktop appImage/
cp ../res/OpenDGLab-Desktop.png appImage/OpenDGLab.png
cp OpenDGLab-Desktop appImage/
cd appImage
../linuxdeployqt OpenDGLab-Desktop -appimage -unsupported-bundle-everything
```
即可完成打包操作，请注意 AppImage 不能设置 Cap，如需运行请使用 `gksu` 或 `kdesu` 来运行。

### 编译 OpenDGLab Desktop (MacOS)
将 OpenDGLab-Core 编译出的 a 文件放入 opendglab-core/bin 文件夹，将 h 文件放入 opendglab-core/header 文件夹。   
```shell
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
cmake --build . --target all
macdeployqt OpenDGLab-Desktop.app -dmg
```
即可生成 OpenDGLab-Desktop.dmg 文件。

## 注意
经过一些测试开发组发现如下问题。  
 * CSR 高通芯片方案的蓝牙模块似乎不支持 BLE，您可能遇到无法搜索到 DG-Lab 设备的问题。  
 * ASUS PCE-AC58BT 因为未知原因工作不良，目前无法确定是硬件问题还是网卡固件问题。您可能会遇到长时间无法搜索到 DG-Lab 设备的问题，或者搜索到后需要很长时间才能连接和完成配置。甚至遭遇 OpenDGLab Desktop 应用程序崩溃（这是由于您的网卡蓝牙没有正常返回探索服务指令导致的）。  

## 声明
使用本程序请遵循 DG-Lab 官方安全声明使用。使用 OpenDGLab 项目，视同于您自己承担相关非官方实现风险。OpenDGLab 开源项目组不为您使用 DG-Lab 设备出现的任何问题负责。

## 协议
使用 AGPLv3 协议授权。