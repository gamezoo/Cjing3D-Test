#include "resource.h"
#include "core\string\stringUtils.h"
#include "core\helper\debug.h"
#include "core\jobsystem\concurrency.h"

namespace  Cjing3D
{
	const ResourceType ResourceType::INVALID_TYPE("");

	ResourceType::ResourceType(const char* type) 
	{
		assert(type[0] == 0 || (type[0] >= 'a' && type[0] <= 'z') || (type[0] >= 'A' && type[0] <= 'Z'));
		mTypeValue = StringUtils::StringToHash(type);
	}

	Resource::Resource() :
		mPath(),
		mRefCount(0),
		mEmpty(1),
		mFailed(0),
		mConverting(0),
		mState(ResState::EMPTY)
	{
	}

	Resource::Resource(const Path& path) :
		mPath(path),
		mRefCount(0),
		mEmpty(1),
		mFailed(0),
		mConverting(0),
		mState(ResState::EMPTY)
	{
	}

	Resource::~Resource()
	{
	}

	void Resource::SetSourceFiles(const DynamicArray<String>& srcFiles)
	{
		mSourceFiles = srcFiles;
	}

	DynamicArray<String> Resource::GetSourceFiles() const
	{
		return mSourceFiles;
	}

	void Resource::AddDependency(Resource& res)
	{
		// bind onStateChanged signal
		mConnectionMap.Connect(
			StringID(res.GetPath().GetHash()),
			res.OnStateChangedSignal, 
			[this](ResState oldState, ResState newState) {
				OnStateChanged(oldState, newState);
			}
		);

		if (res.IsEmpty()) {
			Concurrency::AtomicIncrement(&mEmpty);
		}
		if (res.IsFaild()) {
			Concurrency::AtomicIncrement(&mFailed);
		}
		CheckState();
	}

	void Resource::RemoveDependency(Resource& res)
	{
		mConnectionMap.Disconnect(StringID(res.GetPath().GetHash()));

		if (res.IsEmpty()) 
		{
			Debug::CheckAssertion(mEmpty > 0);
			Concurrency::AtomicDecrement(&mEmpty);
		}
		if (res.IsFaild()) 
		{
			Debug::CheckAssertion(mFailed > 0);
			Concurrency::AtomicDecrement(&mFailed);
		}

		CheckState();
	}

	void Resource::OnLoaded(bool ret)
	{
		if (ret)
		{
			Debug::CheckAssertion(mEmpty > 0);
			Concurrency::AtomicDecrement(&mEmpty);
		}
		else
		{
			Debug::CheckAssertion(mEmpty > 0);
			Concurrency::AtomicDecrement(&mEmpty);
			Concurrency::AtomicIncrement(&mFailed);
		}
		CheckState();
	}

	bool Resource::SetConverting(bool isConverting)
	{
		if (isConverting) {
			return Concurrency::AtomicIncrement(&mConverting) == 1;
		}
		else {
			return Concurrency::AtomicDecrement(&mConverting) == 0;
		}
	}

	void Resource::CheckState()
	{
		// check state by failed dep and emtpy dep
		ResState oldState = mState;
		if (mState != ResState::FAILURE && mFailed > 0)
		{
			mState = ResState::FAILURE;
			OnStateChangedSignal(oldState, mState);
		}
		else if (mFailed <= 0)
		{
			if (mEmpty == 0 && mState != ResState::LOADED)
			{
				mState = ResState::LOADED;
				OnStateChangedSignal(oldState, mState);
			}

			if (mEmpty > 0 && mState != ResState::EMPTY)
			{
				mState = ResState::EMPTY;
				OnStateChangedSignal(oldState, mState);
			}
		}
	}

	void Resource::OnStateChanged(ResState oldState, ResState newState)
	{
		Debug::CheckAssertion(oldState != newState);
		Debug::CheckAssertion(mState != ResState::EMPTY);

		// process old states
		if (oldState == ResState::EMPTY)
		{
			Debug::CheckAssertion(mEmpty > 0);
			Concurrency::AtomicDecrement(&mEmpty);
		}
		if (oldState == ResState::FAILURE)
		{
			Debug::CheckAssertion(mFailed > 0);
			Concurrency::AtomicDecrement(&mFailed);
		}
		// process new states
		if (newState == ResState::EMPTY) {
			Concurrency::AtomicIncrement(&mEmpty);
		}
		if (newState == ResState::FAILURE) {
			Concurrency::AtomicIncrement(&mFailed);
		}

		CheckState();
	}
}