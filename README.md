# OpenDGLab Desktop
OpenDGLab Desktop 是一个使用 OpenDGLab Core 和 Qt5 编写的桌面客户端。  
OpenDGLab Desktop 的目标是在 Windows、Linux、MacOS 上提供对多 DG-Lab 设备的统一化支持。  
使用 OpenDGLab Desktop 您可以在支持蓝牙 BLE 的电脑上同时管理多个 DG-Lab 设备。给您更加优质的使用体验。  

## 尚未实现的功能
 * 远程 API

## 编译
### 获取 OpenDGLab Core
下载并编译 OpenDGLab Core 获取您即将编译版本的 dll so 或 dylib 文件。  
并同时获取对应 .h 头文件。  
注意：Windows 编译请使用 `lib /def:libopendglab.def /out:libopendglab.lib /machine:x64` 或 `lib /def:libopendglab.def /out:libopendglab.lib /machine:x86` 获取对应的 .lib 文件。  
将 dll lib so dylib 文件放入 `opendglab-core/bin` 文件夹中，将 h 头文件放入 `opendglab-core/header` 文件夹中。  

> 注： Linux 和 MacOS 编译尚未测试。 Comming Soon...

### 编译 OpenDGLab Desktop
使用 CMake 编译即可  

```shell
mkdir build
cd build
cmake ..
make
make install
```

Windows 版本将自动生成 zip 压缩包，Linux 版本将自动生成 AppImage 单执行文件(Linux 版本尚未测试)，MacOS 因为开发组设备原因暂时无法测试与编写。

## 声明
使用本程序请遵循 DG-Lab 官方安全声明使用。使用 OpenDGLab 项目，视同于您自己承担相关非官方实现风险。OpenDGLab 开源项目组不为您使用 DG-Lab 设备出现的任何问题负责。

## 协议
使用 AGPLv3 协议授权。