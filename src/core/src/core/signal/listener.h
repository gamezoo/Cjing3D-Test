#pragma once

#include "core\common\definitions.h"
#include "core\container\staticArray.h"
#include "core\memory\memory.h"

namespace Cjing3D
{
	class BaseObserver
	{
	public:
		virtual ~BaseObserver() {}
	};

	template<typename... Args>
	class EventObserver : public BaseObserver
	{
	public:
		using FuncType = Function<void(Args...)>;
		
		explicit EventObserver(const FuncType& func) : mFunc(func) {}
		explicit EventObserver(FuncType&& func) : mFunc(std::move(func)) {}
		explicit EventObserver(const EventObserver<Args...>& rhs) : mFunc(rhs.mFunc) {}
		explicit EventObserver(EventObserver<Args...>&& rhs) : mFunc(std::move(rhs.mFunc)) {}

		template<typename F, typename... T>
		explicit EventObserver(F&& func, T&&... obj)
		{
			static_assert(sizeof...(T) == 0 || sizeof...(T) == 1);
			Bind(std::forward<F>(func), std::forward<T>(obj)...);
		}

		void operator()(Args&&... args)
		{
			if (mFunc) {
				mFunc(std::forward<Args>(args)...);
			}
		}

		template<typename F>
		void Bind(F&& func) {
			mFunc = func;
		}

		template<typename F, typename T>
		void Bind(F&& func, T&& obj)
		{
			if constexpr (std::is_pointer_v<obj>)
			{
				mFunc = [&](Args&&... args) {
					(obj->*func)(std::forward<Args>(args)...);
				};
			}
			else if constexpr (std::is_reference_v<obj>)
			{
				mFunc = [&](Args&&... args) {
					(obj.*func)(std::forward<Args>(args)...);
				};
			}
			else
			{
				Debug::CheckAssertion(false, 
					"[EventObserver::Bind] The obj must be pointer or refrence");
			}
		}

	private:
		FuncType mFunc;
	};

	template<size_t N>
	class EventListener
	{
	public:
		EventListener() = default;
		~EventListener() = default;

		template<typename E, typename T>
		void AddObserver(E e, T&& observer)
		{
			Debug::CheckAssertion((int)e < N);
			mEventObservers[(int)e] = UniquePtr<BaseObserver>(CJING_NEW(T)(std::forward<T>(observer)));
		}

		template<typename E, typename... Args>
		void FireEvent(E e, Args&&... args)
		{
			Debug::CheckAssertion((int)e < N);
			auto ptr = static_cast<EventObserver<Args...>*>(mEventObservers[(int)e].Get());
			if (ptr != nullptr) {
				(*ptr)(std::forward<Args>(args)...);
			}
		}

	private:
		StaticArray<UniquePtr<BaseObserver>, N> mEventObservers;
	};
}