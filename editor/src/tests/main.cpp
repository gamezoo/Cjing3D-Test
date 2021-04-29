#include "resource\resRef.h"
#include "resource\resourceManager.h"
#include "core\helper\debug.h"
#include "core\platform\platform.h"
#include "core\concurrency\jobsystem.h"
#include "renderer\renderGraph\renderGraph.h"
#include "renderer\renderImage.h"

#define CATCH_CONFIG_RUNNER
#include "catch\catch.hpp"

using namespace Cjing3D;

class DebugLoggerSink : public LoggerSink
{
public:
	void Log(LogLevel level, const char* msg)override
	{
		if (level == LogLevel::LVL_ERROR) {
			Debug::DebugOuput("Error:");
		}
		Debug::DebugOuput(msg);
		Debug::DebugOuput("\n");
	}
};
static StdoutLoggerSink mTestStdoutLoggerSink;
static DebugLoggerSink  mTestDebugLoggerSink;

void DeisplayGraphviz(const String& str)
{
}

TEST_CASE("RenderGraphTest", "[Render]")
{
	RenderGraph graph;

	// back tex
	GPU::TextureDesc backDesc;
	backDesc.mFormat = GPU::FORMAT_R8G8B8A8_UNORM;
	backDesc.mWidth = 1024;
	backDesc.mHeight = 768;

	// predepth
	graph.AddCallbackRenderPass(
		"PreDepthPass", 
		RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
		[&](RenderGraphResBuilder& builder) {

			auto depthOutput = builder.CreateTexture("depth", &backDesc);
			builder.AddOutput(depthOutput);

			return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {
				ImageParams params = {};
				RenderImage::Draw(resources.GetTexture(depthOutput), params, cmd);
			};
		}
	);

	// main render pass
	graph.AddCallbackRenderPass(
		"MainPass",
		RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
		[&](RenderGraphResBuilder& builder) {

			builder.AddInput(graph.GetResource("depth"));

			auto output = builder.CreateTexture("mainOutput", &backDesc);
			builder.AddOutput(output);

			return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {
		
			};
		}
	);

	// postprocess pass
	graph.AddCallbackRenderPass(
		"PostprocessPass",
		RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_ASYNC_COMPUTE_BIT,
		[&](RenderGraphResBuilder& builder) {
	
			builder.AddInput(graph.GetResource("mainOutput"));
	
			auto output = builder.CreateTexture("postOutput", &backDesc);
			builder.AddOutput(output);
			
			return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {

			};
		}
	);

	// composite + UI
	graph.AddCallbackRenderPass(
		"CompositePass",
		RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
		[&](RenderGraphResBuilder& builder) {

			builder.AddInput(graph.GetResource("mainOutput"));
			builder.AddInput(graph.GetResource("postOutput"));

			auto output = builder.CreateTexture("finalOutput", &backDesc);
			builder.AddOutput(output);

			return [=](RenderGraphResources& resources, GPU::CommandList& cmd) {

			};
		}
	);

	graph.SetFinalResource(graph.GetResource("finalOutput"));

	// compile graph
	graph.Compile();
	
	// debug display by graphviz
	auto graphvizStr = graph.ExportGraphviz();
	DeisplayGraphviz(graphvizStr);

	// execute graph
	//JobSystem::JobHandle jobHandle;
	//graph.Execute(jobHandle);
	//JobSystem::Wait(&jobHandle);
}

int main(int argc, char* argv[])
{
	// init logger
	Logger::RegisterSink(mTestDebugLoggerSink);
	Logger::RegisterSink(mTestStdoutLoggerSink);
	Logger::SetIsDisplayTime(false);

	// init profiler
	Profiler::Initialize();
	Profiler::SetCurrentThreadName("MainThread");

	// init jobsystme
	JobSystem::Initialize(4, JobSystem::MAX_FIBER_COUNT, JobSystem::FIBER_STACK_SIZE);
  
	auto ret = Catch::Session().run(argc, argv);

	// uninit jobsystem
	JobSystem::Uninitialize();

	// uninit profiler
	Profiler::Uninitilize();

	return ret;
}