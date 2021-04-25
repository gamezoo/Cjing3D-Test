#include "resource\resRef.h"
#include "resource\resourceManager.h"
#include "core\helper\debug.h"
#include "core\platform\platform.h"
#include "core\concurrency\jobsystem.h"
#include "renderer\renderGraph\renderGraph.h"

#define CATCH_CONFIG_RUNNER
#include "catch\catch.hpp"

using namespace Cjing3D;

TEST_CASE("RenderGraphTest", "[Render]")
{
	RenderGraph graph;

	// predepth
	auto& renderPass = graph.AddCallbackRenderPass(
		"PreDepthPass",
		RenderGraphQueueFlag::RENDER_GRAPH_QUEUE_GRAPHICS_BIT,
		[&](RenderGraphResBuilder& builder) {
			// setup


			// execute	
			return [&](RenderGraphResources& resources, GPU::CommandList& cmd) {

			};
		}
	);

	// main render pass

	// postprocess pass

	// composite + UI

	// graph.Compile();

}

int main(int argc, char* argv[])
{
	// init profiler
	Profiler::Initialize();
	Profiler::SetCurrentThreadName("MainThread");

	// init jobsystme
	JobSystem::Initialize(2, JobSystem::MAX_FIBER_COUNT, JobSystem::FIBER_STACK_SIZE);
  
	auto ret = Catch::Session().run(argc, argv);

	// uninit jobsystem
	JobSystem::Uninitialize();

	// uninit profiler
	Profiler::Uninitilize();

	return ret;
}