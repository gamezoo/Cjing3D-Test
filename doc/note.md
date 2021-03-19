
***********************************************************************************
Cjin3D Dev Log
Current Version 0.2.0
Next Version 0.2.1
***********************************************************************************
CJING 0.1.0 see <a href="https://github.com/maoxiezhao/Cjing3D">[here]</a>
***********************************************************************************

## Current Plans
__0.2.1 :__ 
####Renderer 
1. main pipeline 
   基础renderGraph构建      (ok)
   基础RenderScene         
      deferred pass       (TODO)
      v0.1.0渲染重新实现   (TODO)
   Texture converter       (ok)
   Material converter     (TODO)
   Model converter        (TODO)

#### editor
  EntityList 
  EntityInspector
  GameView
  SceneView

#### network
  基于asio实现的简易架构

## Approximate Changelog
__0.2.0 :__ 
 * 基于子模块的架构重构: client\core\gpu\imguiRhi\luaBinder\math\rendere\resource

__0.1.0 :__ 
 * UI初步框架，事件分发，渲染框架，实现基本类型的widget从xml中的创建
 * LuaBinder支持shared_ptr
 * UI框架脚本系统，支持从xml中添加Events
 * 初步Archive支持, 常用数据支持序列化
 * 场景序列化实现初版，各个component支持序列化
 * 将所有文件加载方式统一
 * 初步支持2D渲染(支持精灵）
 * 统一绝对路径和相关路径，特别时序列化相关内容 ,并将其细节封装在FileData中
 * 支持当前带纹理版本的场景序列化
 * 支持specular贴图，法线贴图，BUMP贴图
 * 基于IMGUI添加部分新的编辑界面
 * Gamma矫正
 * Compute Shader 支持，后处理简易架构
 * 支持simple tonemapping
 * 支持下顶点颜色
 * 支持对于非dds加载的纹理，延迟使用compute shader生成mipmap
 * 基于ImGUI实现部分编辑调试界面
 * optick profiler工具支持
 * postprocess FXAA
 * clear some systems, clear rhi reference
 * directional light CSM shadow and soft shadow
 * 基于高度图、QuadTreeLod和tessellation的简易无裂缝大地形
 * 增加支持gltf模型
 * GPU skinning
 * rhi代码优化v0.0：rhiResource代码优化, pipelineStateObject, renderBehavior
 * Cubemap支持，天空盒支持
 * Memory System (memory manager; custom memory allocator; custom container)
 * forward+ 
 * simple audio based on xaudio2
 * sprite2D (DrawInstances)
 * font based on stb_truetype
 * native gui support
 * linearDepth 
 * SSAO 
 * simple gpu based particle system
 * engine clean up
 * multi thread render based on D3D11

__0.0.0 :__ 
 * base framework
 * 整理GPUResource
 * 处理Reource析构释放在Render释放之后导致报错
 * 支持base script system，
 * 优化完善了luaBinder, 升级并使用luaBinder
 * render path refactor
 * 部分类的注册到Lua
 * luaBinder 优化第一版
 * base texture
 * buffer manager and base light support
 * 暂时使用lua_gc(state, GC_STEP, 200)保证每帧进行增量GC
 * base light 框架
 * phong光照支持（点光源支持）
 * 接入imGUI
 * Blinn-Phong Lighting
