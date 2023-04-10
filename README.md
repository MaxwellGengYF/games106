# games106 

现代图形绘制流水线原理与实践，作业框架。

这个fork来自[SaschaWillems/Vulkan: Examples and demos for the new Vulkan API (github.com)](https://github.com/SaschaWillems/Vulkan) 在这个项目中有非常多的example可以学习。在学习一个API的时候，代码示例永远是最好的老师。本课程的作业需要在已有的代码示例中做修改。

## Build

详情可以查看项目原来的 [BUILD文档](./BUILD.md) ，可以在Windows/Linux/Andorid/macOS/iOS中构建

### XMake

新增了xmake编译支持（目前仅测试了windows平台LLVM与MSVC工具链），命令行:

使用LLVM工具链：

xmake f --toolchain=llvm -c

若提示"llvm toolchain not found!"。请手动指定LLVM路径：

xmake f --toolchain=llvm -sdk=#LLVM_DIR# -c

Windows平台可使用MSVC工具链：

xmake f --toolchain=msvc -c

启动编译：

xmake

同时提供了VSCode设置支持: .vscode/settings.json。

## HomeWork

作业的课程代码在./homework目录下，shader在./data/homework/shaders下。上传作业也按照一样的文件结构。上传对应的文件即可。

### 作业提交

课程学生注册方法：登录 http://cn.ces-alpha.org/course/register/GAMES106/  注册账号，填写个人信息，输入验证码ilovegraphics，即可进入课程主页，查看并提交作业

### homework0

作业0，作为一个熟悉编译环境的课程作业。最后显示一个如下图一样的三角形。有兴趣可以尝试在Android或者iOS上运行

![triangle](./screenshots/triangle.jpg)

### homework1

作业1，扩展GLTF loading。作业1提供了一个gltf显示的demo，只支持静态模型，以及颜色贴图。作业1需要在这个基础上进行升级。支持：1、支持gltf的骨骼动画。2、支持gltf的PBR的材质，包括法线贴图。
进阶作业：增加一个Tone Mapping的后处理pass。增加GLTF的滤镜功能。

下面是相关的资料

- GLTF格式文档 https://github.com/KhronosGroup/glTF
- 带动画的GLTF模型 https://github.com/GPUOpen-LibrariesAndSDKs/Cauldron-Media/tree/v1.0.4/buster_drone

