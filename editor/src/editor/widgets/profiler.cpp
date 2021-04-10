#include "profiler.h"
#include "core\helper\profiler.h"
#include "core\helper\stream.h"

namespace Cjing3D
{
	/// ///////////////////////////////////////////////////////////////////////
	/// definitions
	static constexpr int DEFAULT_RANGE = 100'000;

	struct ThreadLocalContextProxy
	{
		StaticString<64> mName;
		U32 mThreadID;
		U32 mBufStart;
		U32 mBufEnd;
		bool mIsShowInProfiler;
		U32 mBufSize;
		const U8* mBuffer;

		ThreadLocalContextProxy(const U8* data)
		{
			InputMemoryStream stream(data, 4096);
			mName = stream.ReadString();
			mThreadID = stream.Read<U32>();
			mBufStart = stream.Read<U32>();
			mBufEnd = stream.Read<U32>();
			mIsShowInProfiler = stream.Read<bool>();
			mBufSize = stream.Read<U32>();
			mBuffer = stream.data() + stream.Offset();
		}

		// get the next address of ThreadContext
		const U8* GetNext() {
			return mBuffer + mBufSize;
		}
	};

	struct UIProfilerBlock
	{
		U32 mOffset = 0;
		U32 mColor = 0;
	};

	/// ///////////////////////////////////////////////////////////////////////
	/// impl
	class EditorWidgetProfilerImpl
	{
	public:
		bool mIsPaused = false;
		MemoryStream mProfileData;
		U32 mRange = DEFAULT_RANGE;
		U64 mProfilerDataLastTime = 0;
		F32 mAutopause = -1.0f;

	public:
		void ShowGeneralInfo();
		void ShowCPUProfiler();
		void OnProfilePaused();
		void ShowProfileData();

	public:
		void FindProfilerDataLastTime()
		{
			// 遍历所有的ProfilerBlock，获取最迟记录的时间
			mProfilerDataLastTime = 0;
			ForEachThread([&](const ThreadLocalContextProxy& ctx) {
				if (ctx.mThreadID == 0) {
					return;
				}

				U32 pos = ctx.mBufStart;
				while (pos != ctx.mBufEnd) 
				{
					Profiler::ProfileBlockHeader header;
					ReadFromThreadCtx<Profiler::ProfileBlockHeader>(ctx, pos, header);
					mProfilerDataLastTime = std::max(mProfilerDataLastTime, header.mTime);
					pos += header.mSize;
				}
			});
		}
		
		template<typename Func>
		void ForEachThread(const Func& func)
		{
			if (mProfileData.Empty()) {
				return;
			}

			InputMemoryStream inputStream(mProfileData);
			const U32 count = inputStream.Read<U32>();
			const U8* it = inputStream.data() + inputStream.Offset();

			// global
			ThreadLocalContextProxy globalCtx(it);
			func(globalCtx);
			it = globalCtx.GetNext();

			// other threads
			for (int i = 0; i < count; i++)
			{
				ThreadLocalContextProxy ctx(it);
				func(ctx);
				it = ctx.GetNext();
			}
		}

		template<typename T>
		void ReadFromThreadCtx(const ThreadLocalContextProxy& ctx, U32 pos, T& value)
		{
			const U8* buf = ctx.mBuffer;
			const U32 bufSize = ctx.mBufSize;
			const U32 l = pos % bufSize;
			if (l + sizeof(value) <= bufSize) 
			{
				Memory::Memcpy(&value, buf + l, sizeof(value));
				return;
			}

			// 尾部读取部分数据，头部读取剩余数据
			Memory::Memcpy(&value, buf + l, bufSize - l);
			Memory::Memcpy((U8*)&value + (bufSize - l), buf, sizeof(value) - (bufSize - l));
		}
	};

	void EditorWidgetProfilerImpl::ShowGeneralInfo()
	{
	}

	void EditorWidgetProfilerImpl::ShowCPUProfiler()
	{
		ImGui::Text("Press \"Start\" to start profiling, press \"Pause\" to show infos");
		if (ImGui::Button(mIsPaused ? "Start" : "Pause"))
		{
			mIsPaused = !mIsPaused;
			Profiler::SetPause(mIsPaused);

			if (mIsPaused) {
				OnProfilePaused();
			}
		}
		ImGui::SameLine();
	
		// advanced
		if (ImGui::BeginMenu("Advanced")) 
		{
			bool autoPaused = mAutopause >= 0;
			if (ImGui::Checkbox("Autopause enabled", &autoPaused)) {
				mAutopause = -mAutopause;
			}
			if (mAutopause >= 0) {
				ImGui::InputFloat("Autopause limit (ms)", &mAutopause, 1.f, 10.f, "%.2f");
			}
			ImGui::EndMenu();
		}

		if (!mProfileData.Empty()) {
			ShowProfileData();
		}
	}

	void EditorWidgetProfilerImpl::OnProfilePaused()
	{
		mProfileData.Clear();
		Profiler::GetProfilerData(mProfileData);
		FindProfilerDataLastTime();
	}

	void EditorWidgetProfilerImpl::ShowProfileData()
	{
		const F32 startX = ImGui::GetCursorScreenPos().x;
		const F32 startY = ImGui::GetCursorScreenPos().y;
		const F32 endX = startX + ImGui::GetContentRegionAvail().x;
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const U64 startTime = mProfilerDataLastTime - mRange;

		// get global context
		ThreadLocalContextProxy globalContext(mProfileData.data() + sizeof(U32)); // skip count

		// show profiler data of each thread
		ForEachThread([&](const ThreadLocalContextProxy& ctx) {
			if (ctx.mThreadID == 0) {
				return;
			}

			if (!ImGui::TreeNode((const void*)ctx.mBuffer, "%s", ctx.mName)) {
				return;
			}

			F32 curY = ImGui::GetCursorScreenPos().y;
			F32 originalY = curY;
			auto DrawBlock = [&](U64 from, U64 to, const char* name, U32 color) {

				// calculate percentage of pos by 'from' and 'to'
				const F32 pStart = F32(I32(from - startTime) / F32(mRange));
				const F32 pEnd = F32(I32(to - startTime) / F32(mRange));

				const F32 xLeft = startX * (1 - pStart) + endX * pStart;
				const F32 xRight = startX * (1 - pEnd) + endX * pEnd;
				const ImVec2 ra(xLeft, curY);
				const ImVec2 rb(xRight, curY + 20);
				drawList->AddRectFilled(ra, rb, color);

				// add text
				if (ImGui::CalcTextSize(name).x + 2 < xRight - xLeft) {
					drawList->AddText(ImVec2(xLeft + 2, curY), 0xff000000, name);
				}
			};

			// Traversal buffer to parse ProfilerBlocks
			UIProfilerBlock blocks[64];
			UIProfilerBlock fiberWaitblocks[16];
			I32 fiberWaitLevel = 0;
			I32 scopeLevel = 0;
			I32 maxScopeLevel = 0;
			U32 pos = ctx.mBufStart;
			while (pos != ctx.mBufEnd)
			{
				Profiler::ProfileBlockHeader header;
				ReadFromThreadCtx<Profiler::ProfileBlockHeader>(ctx, pos, header);
				switch (header.mType)
				{
				case Profiler::ProfileType::BEGIN_CPU:
				{
					scopeLevel++;

					// 用于在END_CPU时从buffer读取block数据
					blocks[scopeLevel].mOffset = pos;
					blocks[scopeLevel].mColor = 0xffDDddDD;
					curY += 20.0f;

					maxScopeLevel = std::max(maxScopeLevel, scopeLevel);
				}
				break;
				case Profiler::ProfileType::END_CPU:
				{
					curY = std::max(curY - 20.0f, originalY);
					if (scopeLevel > 0)
					{
						auto& block = blocks[scopeLevel];
						Profiler::ProfileBlockHeader blockHeader;
						ReadFromThreadCtx<Profiler::ProfileBlockHeader>(ctx, block.mOffset, blockHeader);

						const char* name;
						ReadFromThreadCtx(ctx, block.mOffset + sizeof(Profiler::ProfileBlockHeader), name);
						DrawBlock(blockHeader.mTime, header.mTime, name, block.mColor);
					
						scopeLevel--;
					}
				}
				break;
				case Profiler::ProfileType::BEGIN_FIBER_WAIT: 
				{
					Profiler::FiberWaitRecord record;
					ReadFromThreadCtx(ctx, pos + sizeof(Profiler::ProfileBlockHeader), record);
					if (header.mTime >= startTime && header.mTime <= mProfilerDataLastTime)
					{
						const F32 t = F32((header.mTime - startTime) / F64(mRange));
						const float x = startX * (1 - t) + endX * t;
						drawList->AddRect(ImVec2(x - 2, curY - 2), ImVec2(x + 2, curY + 2), 0xff00ff00);
					}

					fiberWaitblocks[fiberWaitLevel].mOffset = pos;
					fiberWaitblocks[fiberWaitLevel].mColor = 0xff0000ff;
					fiberWaitLevel++;
				}
				break;
				case Profiler::ProfileType::END_FIBER_WAIT:
				{
					Profiler::FiberWaitRecord record;
					ReadFromThreadCtx(ctx, pos + sizeof(Profiler::ProfileBlockHeader), record);
					if (fiberWaitLevel > 0)
					{
						// prev block
						fiberWaitLevel--;

						auto& block = fiberWaitblocks[fiberWaitLevel];
						Profiler::ProfileBlockHeader blockHeader;
						ReadFromThreadCtx<Profiler::ProfileBlockHeader>(ctx, block.mOffset, blockHeader);
						DrawBlock(blockHeader.mTime, header.mTime, "Wait", block.mColor);
					}

					if (header.mTime >= startTime && header.mTime <= mProfilerDataLastTime)
					{
						// draw link point
						const F32 t = F32((header.mTime - startTime) / F64(mRange));
						const float x = startX * (1 - t) + endX * t;
						drawList->AddRect(ImVec2(x - 2, curY - 2), ImVec2(x + 2, curY + 2), 0xffff0000);
					}
				}
				break;
				case Profiler::ProfileType::COLOR:
					if (scopeLevel > 0) {
						ReadFromThreadCtx(ctx, pos + sizeof(Profiler::ProfileBlockHeader), blocks[scopeLevel].mColor);
					}
					break;
				default:
					break;
				}

				pos += header.mSize;
			}

			ImGui::Dummy(ImVec2(endX - startX, maxScopeLevel * 20.f));
			ImGui::TreePop();
		});

		if (ImGui::IsMouseHoveringRect(
			ImVec2(startX, startY),
			ImVec2(endX, ImGui::GetCursorScreenPos().y)))
		{
			// drag view
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
				mProfilerDataLastTime -= I64((ImGui::GetIO().MouseDelta.x / (endX - startX)) * mRange);
			}

			U64 cursorTime = U64((ImGui::GetMousePos().x - startX) / (endX - startX)) * mRange + startTime;
			U64 cursorToEnd = mProfilerDataLastTime - cursorTime;
			// ctrl + wheel to control viewRange
			if (ImGui::GetIO().KeyCtrl) 
			{
				if (ImGui::GetIO().MouseWheel > 0 && mRange > 1) 
				{
					mRange >>= 1;
					cursorToEnd >>= 1;
				}
				else if (ImGui::GetIO().MouseWheel < 0) 
				{
					mRange <<= 1;
					cursorToEnd <<= 1;
				}
				mProfilerDataLastTime = cursorToEnd + cursorTime;
			}
		}

		// process global context
		{
			float threadsEndY = ImGui::GetCursorScreenPos().y;
			ThreadLocalContextProxy& ctx = globalContext;
			U32 pos = ctx.mBufStart;
			while (pos != ctx.mBufEnd)
			{
				Profiler::ProfileBlockHeader header;
				ReadFromThreadCtx<Profiler::ProfileBlockHeader>(ctx, pos, header);
				switch (header.mType)
				{
				case Profiler::ProfileType::FRAME:
				{
					if (header.mTime >= startTime && header.mTime <= mProfilerDataLastTime)
					{
						const float t = float((header.mTime - startTime) / double(mRange));
						const float x = startX * (1 - t) + endX * t;
						drawList->AddLine(ImVec2(x, startY), ImVec2(x, threadsEndY), 0xffff0000);
					}
				}
				break;
				default:
					break;
				}

				pos += header.mSize;
			}
		}
	}

	EditorWidgetProfiler::EditorWidgetProfiler(GameEditor& editor) :
		EditorWidget(editor)
	{
		mTitleName = "Profiler";
		mIsWindow = true;
		mWidgetFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavInputs;
		mImpl = CJING_NEW(EditorWidgetProfilerImpl);
	}

	EditorWidgetProfiler::~EditorWidgetProfiler()
	{
		CJING_SAFE_DELETE(mImpl);
	}

	void EditorWidgetProfiler::Initialize()
	{
		mImpl->mIsPaused = Profiler::IsPaused();
	}

	void EditorWidgetProfiler::Update(F32 deltaTime)
	{
		if (ImGui::BeginTabBar("profiler_tabs"))
		{
			if (ImGui::BeginTabItem("General"))
			{
				mImpl->ShowGeneralInfo();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("CPU/GPU"))
			{
				mImpl->ShowCPUProfiler();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Memory"))
			{
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Resources"))
			{
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
}