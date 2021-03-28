#pragma once
#include "network\network.h"
#include "core\container\dynamicArray.h"
#include "core\helper\stream.h"
#include "core\helper\timer.h"

#ifdef CJING3D_NETWORK_ASIO
namespace Cjing3D
{
namespace Network
{
    thread_local static asio::error_code errorCodeIgnore;

    //////////////////////////////////////////////////////////////////////////
    // Transfer condition wrap
    //////////////////////////////////////////////////////////////////////////
    template<class T>
    class ConditionWrap
    {
    public:
        using type = T;
        ConditionWrap(T c) : mCondition(std::move(c)) {}
        inline T& operator()() { return this->mCondition; }
    protected:
        T mCondition;
    };

    template<>
    class ConditionWrap<void>
    {
    public:
        using type = void;
        ConditionWrap() = default;
    };

    template<>
    class ConditionWrap<char>
    {
    public:
        using type = char;
        ConditionWrap(char c) : mCondition(c) {}
        inline char operator()() { return this->mCondition; }
    protected:
        char mCondition = '\0';
    };

    //////////////////////////////////////////////////////////////////////////
    // IOContext
    //////////////////////////////////////////////////////////////////////////
    class IOContextASIO
    {
    public:
        IOContextASIO() :
            mStrand(mContext)
        {
        }

        ~IOContextASIO()
        {
            Stop();
        }

        IOContextASIO(const IOContextASIO& rhs) = delete;
        void operator=(const IOContextASIO& rhs) = delete;

        bool Start()
        {
            if (!mIsStoped) {
                return false;
            }

            if (mWork || mThreadContext.IsValid()) {
                return false;
            }

            mContext.restart();

            // create work guard
            // Issue a task to the asio context - This is important
            // as it will prime the context with "work", and stop it
            // from exiting immediately. Since this is a server, we 
            // want it primed ready to handle clients trying to
            // connect.
            mWork = CJING_NEW(asio::executor_work_guard<asio::io_context::executor_type>)(mContext.get_executor());

            mThreadContext = Concurrency::Thread([](void* data) {
                asio::io_context* context = reinterpret_cast<asio::io_context*>(data);
                context->run();
                return 0;
                }, &mContext, 4096, "IOContextThread");
            mIsStoped = false;
        }

        void Stop()
        {
            if (mIsStoped) {
                return;
            }

            if (!mWork && !mThreadContext.IsValid()) {
                return;
            }

            // waiting for all works to complete. 
            Wait();
            // clear work guard
            mWork->reset();

            if (mThreadContext.IsValid()) {
                mThreadContext.Join();
            }

            CJING_DELETE(mWork);
            mIsStoped = true;
        }

        void Wait()
        {
            // reset work guard
            if (mWork != nullptr) {
                mWork->reset();
            }

            // wait until the io_context is stopped
            auto time = Timer::GetAbsoluteTime();
            while (!mContext.stopped()) {
                F32 sleepTime = (F32)std::max(std::min(Timer::GetAbsoluteTime() - time, 0.01), 0.001);
                Logger::Info("Sleep:%f", sleepTime);
                Concurrency::Sleep(sleepTime);
            }
        }

        asio::io_context& GetContext() {
            return mContext;
        }
        asio::io_context::strand& GetStrand() {
            return mStrand;
        }
        bool IsStoped()const {
            return mIsStoped;
        }

    private:
        asio::io_context mContext;
        Concurrency::Thread mThreadContext;
        asio::executor_work_guard<asio::io_context::executor_type>* mWork = nullptr;
        bool mIsStoped = true;

        // io_context::strand class provides the ability to post and 
        // dispatch handlers with the guarantee that none of those handlers
        // will execute concurrently.
        asio::io_context::strand mStrand;
    };

    //////////////////////////////////////////////////////////////////////////
    // EventQueue
    //////////////////////////////////////////////////////////////////////////
    class EventQueue
    {
    public:
        EventQueue(IOContextASIO& context) :
            mContext(context) {}
        ~EventQueue() = default;

        template<typename EventFuncT>
        void PushEvent(EventFuncT&& func)
        {
            asio::post(mContext.GetStrand(), 
                [this, func = std::forward<EventFuncT>(func)]() mutable {
                    bool empty = mEvents.empty();
                    mEvents.push_back(std::move(func));
                    if (empty) {
                        (*mEvents.front())();
                    }
            });
        }

        void DoNextEvent()
        {
            asio::post(mContext.GetStrand(), [this]() {
                if (!mEvents.empty())
                {
                    mEvents.pop_front();
                    if (!mEvents.empty()) {
                        (*mEvents.front())();
                    }
                }
            });
        }

    private:
        IOContextASIO& mContext;
        ConcurrentQueue<Function<bool()>> mEvents;
    };

    //////////////////////////////////////////////////////////////////////////
    // DataPersistence
    //////////////////////////////////////////////////////////////////////////
    template<typename DerivedT>
    class DataPersistence
    {
    protected:
        DerivedT& mDerive;

    public:
        DataPersistence() :mDerive(static_cast<DerivedT&>(*this)) {}
        ~DataPersistence() = default;

        template<class T>
        inline auto Persistence(T* data, size_t count)
        {
            using ValueT = typename std::remove_cv_t<std::remove_reference_t<T>>;
            return std::basic_string<ValueT>(data, count);
        }
    };

    //////////////////////////////////////////////////////////////////////////
    // Sender
    //////////////////////////////////////////////////////////////////////////
    template<typename DerivedT>
    class Sender
    {
    protected:
        DerivedT& mDerive;

    public:
        Sender() :mDerive(static_cast<DerivedT&>(*this)) {}
        ~Sender() = default;

        template<class T>
        bool Send(T&& data)
        {
            try
            {
                mDerive.PushEvent([this, data]()->bool {
                    

                    return true;
                });
                return true;
            }
            catch (std::exception& e)
            {
                Logger::Warning("Failed to send data:%s", e.what());
                return false;
            }
        }

        // send char buffer
        template<typename T, class Traits = std::char_traits<T>>
        typename std::enable_if_t<
            std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, char> ||
            std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, wchar_t>,
            bool> Send(T* data)
        {
            return Send(data, data ? Traits::length(data) : 0);
        }

        template<typename T>
        bool Send(T&& data, size_t count)
        {
            try
            {
                mDerive.PushEvent(
                    [this, data = mDerive.Persistence(data, count)]() {
                        mDerive.HandleSend(data);
                        return true;
                });
                return true;
            }
            catch (std::exception& e)
            {
                Logger::Warning("Failed to send data:%s", e.what());
                return false;
            }
        }

    };
}
}
#endif