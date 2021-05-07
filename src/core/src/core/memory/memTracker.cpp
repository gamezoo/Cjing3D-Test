#include "memTracker.h"

#ifdef CJING_MEMORY_TRACKER
#include "core\helper\debug.h"
#include "core\concurrency\concurrency.h"

#include <fstream>
#include <sstream>
#include <iostream>

namespace Cjing3D
{
	Concurrency::Mutex mMutex;
	std::ofstream loggerFile;
	bool mIsExit = false;

	MemoryTracker::MemoryTracker()
	{
	}

	void MemoryTracker::RecordAlloc(void* ptr, size_t size, const char* filename, int line)
	{
		Concurrency::ScopedMutex lock(mMutex);
		if (size <= 0) {
			return;
		}

		if (mAllocNodeMap.find(ptr) != mAllocNodeMap.end())
		{
			auto kvp = mAllocNodeMap.find(ptr);
			AllocNode& allocNode = kvp->second;
			std::stringstream os;
			os << (allocNode.filename_ ? allocNode.filename_ : "(unknown)");
			os << "(" << allocNode.line_ << ")";
			os << ": alloc size:" << allocNode.size_;
			os << std::endl;
			Logger::Info(os.str().c_str());

			DBG_ASSERT_MSG(mAllocNodeMap.find(ptr) == mAllocNodeMap.end(), "The address is already allocated");
		}


		mAllocNodeMap.insert(std::pair(ptr, AllocNode(size, filename, line)));
		mMemUsage += size;
		mMaxMemUsage = std::max(mMaxMemUsage, mMemUsage);
	}

	void MemoryTracker::RecordRealloc(void* ptr, void* old, size_t size, const char* filename, int line)
	{
		Concurrency::ScopedMutex lock(mMutex);
		if (ptr != old)
		{
			auto it = mAllocNodeMap.find(old);
			if (it != mAllocNodeMap.end())
			{
				mMemUsage -= it->second.size_;
				mAllocNodeMap.erase(it);
			}
		}

		auto it = mAllocNodeMap.find(ptr);
		if (it == mAllocNodeMap.end())
		{
			mAllocNodeMap.insert(std::pair(ptr, AllocNode(size, filename, line)));
			mMemUsage += size;
			mMaxMemUsage = std::max(mMaxMemUsage, mMemUsage);
		}
		else
		{
			size_t oldSize = it->second.size_;
			it->second.size_ = size;
			it->second.filename_ = filename;
			it->second.line_ = line;

			mMemUsage += size - oldSize;
			mMaxMemUsage = std::max(mMaxMemUsage, mMemUsage);
		}
	}

	void MemoryTracker::RecordFree(void* ptr)
	{
		if (ptr == nullptr) {
			return;
		}

#ifdef DEBUG
		if (mIsExit) {
			Debug::CheckAssertion(false);
		}
#endif
		Concurrency::ScopedMutex lock(mMutex);
		auto it = mAllocNodeMap.find(ptr);
		if (it == mAllocNodeMap.end())
		{
			Logger::Error("The address is already free");
		}
		else
		{
			mMemUsage -= it->second.size_;
			mAllocNodeMap.erase(it);
		}
	}

	void MemoryTracker::ReportMemoryLeak()
	{
		if (mAllocNodeMap.empty()) {
			return;
		}

		std::stringstream os;
		os << std::endl;
		os << "[Memory] Detected memory leaks !!! " << std::endl;
		os << "[Memory] Leaked memory usage:" << mMemUsage << std::endl;
		os << "[Memory] Dumping allocations:" << std::endl;

		for (auto kvp : mAllocNodeMap)
		{
			AllocNode& allocNode = kvp.second;
			os << (allocNode.filename_ ? allocNode.filename_ : "(unknown)");
			os << "(" << allocNode.line_ << ")";
			os << ": alloc size:" << allocNode.size_;
			os << std::endl;
		}

		std::cout << os.str().c_str() << std::endl;
		Logger::Warning(os.str().c_str());

#ifdef DEBUG
		system("pause");
#endif // DEBUG

		if (mMemoryLeaksFileName != nullptr)
		{
			if (!loggerFile.is_open()) {
				loggerFile.open(mMemoryLeaksFileName);
			}
			
			loggerFile << os.str();
			loggerFile.close();
		}

		mIsExit = true;
	}

	MemoryTracker& MemoryTracker::Get()
	{
		static MemoryTracker tracker;
		return tracker;
	}
}

#endif