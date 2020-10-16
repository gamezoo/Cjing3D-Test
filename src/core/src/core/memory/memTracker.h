#pragma once

#include "mem_def.h"

#ifdef CJING_MEMORY_TRACKER
#include <unordered_map>

#define CJING_MEMORY_LEAK_FILE "memory_leaks.txt"

namespace Cjing3D
{
	class MemoryTracker
	{
	public:
		~MemoryTracker() {
			ReportMemoryLeak();
		}

		void RecordAlloc(void* ptr, size_t size, const char* filename, int line);
		void RecordRealloc(void* ptr, void* old, size_t size, const char* filename, int line);
		void RecordFree(void* ptr);
		void ReportMemoryLeak();

		uint64_t GetMemUsage() { return mMemUsage; }
		uint64_t GetMaxMemUsage() { return mMaxMemUsage; }

		static MemoryTracker& Get();

	private:
		MemoryTracker();

		struct AllocNode
		{
			size_t size_ = 0;
			const char* filename_;
			int line_ = -1;

			AllocNode(size_t size, const char* filename, int line) :
				size_(size),
				filename_(filename),
				line_(line)
			{}
		};
		std::unordered_map<void*, AllocNode> mAllocNodeMap;

		volatile uint64_t mMemUsage = 0;
		volatile uint64_t mMaxMemUsage = 0;
		char* mMemoryLeaksFileName = nullptr;
	};
}

#endif