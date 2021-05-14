#include <App.hpp>
#include <Glm.hpp>
#include <Input.hpp>
#include <Logging.hpp>
#include <Math.hpp>
#include <Timing.hpp>

#include <imgui.h>

void App::Init(HWND t_Window)
{
	DxProfileCode(DxTimedBlock(Phase_Init, "Application initialization"));
	
	DXDEBUG("[RES] Resouces path: {}", Arguments.ResourcesPath.data());

	Graphics.InitSwapChain(t_Window, Width, Height);
	Graphics.InitBackBuffer();
	Graphics.InitZBuffer(Width, Height);
	Graphics.InitResources();
	Graphics.InitRasterizationsStates();
	Graphics.InitSamplers();
	Graphics.InitBlending();
	Graphics.InitDepthStencilStates();

	Game.Application = this;
	Game.Graphics = &Graphics;
	Game.Init();

	Timer = 0.0f;
	
	OPTICK_APP("DirectXer");
	{
		OPTICK_THREAD("MainThread");
		OPTICK_FRAME("MainThread");
	}
}

void App::Update(float dt)
{
	OPTICK_FRAME("MainThread");

	TempFormater formater;
	
	if (ImGui::CollapsingHeader("Telemetry"))
	{
		
		if (ImGui::TreeNode("Cycle Counters"))
		{

			for (const auto& [id, entry] : Telemetry::CycleCounters)
			{
				const uint32 counter = uint32(id & 0xFFFFFFFF);
				const uint32 sys = uint32((id & (uint64)(0xFFFFFFFF) << 32) >> 32);

				String text{formater.Format("[{}][{}] : {} Avrg. Cycles",
											gCycleCounterTagNames[counter], gSystemTagNames[sys], entry.AvgCycles)};

				ImGui::BulletText(text.data());
			}
			
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Memory Tacking"))
		{
			String text{formater.Format("Total Temorary Memory: {:.3f} MBs", Memory::g_Memory.TempMemoryMaxSize / (1024.0f*1024.0f))};
			ImGui::Text(text.data());

			text = formater.Format("Total Bulk Memory: {:.3f} MBs", Memory::g_Memory.BulkMemoryMaxSize / (1024.0f*1024.0f));
			ImGui::Text(text.data());
				
			ImGui::Separator();
			ImGui::Text("CPU Memory Counters");

			auto displayMemory = [&text, &formater](SystemTag tag){
				auto& entry = Telemetry::MemoryStates[tag];
				if (entry.CurrentMemory == 0) return;

				float mem = entry.CurrentMemory > 1024*1024 ? entry.CurrentMemory / (1024.0f*1024.0f) : entry.CurrentMemory / 1024.0f;
				const char* unit = entry.CurrentMemory > 1024*1024 ? "MBs" : "KBs";
				text = formater.Format("[{}] : {:.2f} {}", gSystemTagNames[tag], mem, unit);
				ImGui::BulletText(text.data());			
			};

			displayMemory(Memory_Bulk);
			displayMemory(Memory_GameState);
			displayMemory(Memory_Audio);
			displayMemory(Memory_2DRendering);
			displayMemory(Memory_3DRendering);
			displayMemory(Memory_GPUResource);

			ImGui::Separator();

			ImGui::Text("GPU Memory Counters");
			
			displayMemory(GPURes_VertexBuffer);
			displayMemory(GPURes_IndexBuffer);
			displayMemory(GPURes_Texture);
			displayMemory(GPURes_ConstantBuffer);

			ImGui::Separator();

			auto report = Graphics.ReportMemory();
			
			ImGui::Text("Reported GPU Memory");

			text = formater.Format("GPU Usage: {:.3f} MBs", report.Usage / (1024.0f * 1024.0f));
			ImGui::BulletText(text.data());

			text = formater.Format("GPU Budget: {:.3f} GBs", report.Budget / (1024.0f * 1024.0f * 1024.0f));
			ImGui::BulletText(text.data());
			
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Timing"))
		{
			String text;

			ImGui::Text("Timed Blocks:");
			auto displayTimingBlocksOfType = [&text, &formater](SystemTag requiredTag){
				for (const auto& [id, entry] : Telemetry::BlockTimers)
				{
					const uint32 tag = uint32(id & 0xFFFFFFFF);
					if ( tag != requiredTag) continue;
					text = formater.Format("[{}] [{}] : {} ms", gSystemTagNames[tag], entry.Msg, entry.Time);
					ImGui::BulletText(text.data());
				}
			};

			displayTimingBlocksOfType(Phase_Init);
			displayTimingBlocksOfType(Phase_Rendering);

			ImGui::Separator();
			
			ImGui::Text("Performance:");
			
			text = formater.Format("FPS: {:.3f} ms", Window->LastFPS);
			ImGui::BulletText(text.data());

			text = formater.Format("Frame time: {:.3f} ms", Window->LastFrameTime);
			ImGui::BulletText(text.data());
			
			text = formater.Format("Last GPU timing query: {:.3f} ms", Window->LastGpuTiming.Time);
			ImGui::BulletText(text.data());

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("GPU Pipeline"))
		{
			String text{formater.Format("Vertices count processed: {}", Window->LastGpuStats.VerticesCount)};
			ImGui::BulletText(text.data());

			text = formater.Format("Primitve count processed: {}", Window->LastGpuStats.PrimitivesCount);
			ImGui::BulletText(text.data());

			text = formater.Format("Vertex shader calls: {}", Window->LastGpuStats.VSInvocationsCount);
			ImGui::BulletText(text.data());

			text = formater.Format("Pixel shader calls: {}", Window->LastGpuStats.PSInvocationsCount);
			ImGui::BulletText(text.data());

			ImGui::Separator();

			text = formater.Format("Draw calls count: {}", Graphics.LastCallsCount); 
			ImGui::BulletText(text.data());
			
			ImGui::TreePop();
		}
	}
	
}

void App::PostInit()
{
	// @Note: Windows is weird and it sends a resize mesage during the creation
	// of the window; this method will be called after everything is initialized
	// and no more spurious resize messages are expected to arrive
	Game.PostInit();
}

void App::Resize()
{
	Graphics.ResizeBackBuffer(Width, Height);
	Graphics.DestroyZBuffer();
	Graphics.InitZBuffer(Width, Height);
	Graphics.SetViewport(0, 0, Width, Height);
	
	Game.Resize();
}
