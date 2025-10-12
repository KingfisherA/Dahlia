#pragma once

#include "Dahlia/Dahlia-API.h"
#include <thread>

namespace DH {
    DH_API class Thread {
    public:
        Thread() : mThread(), mThreadFinished(false), mThreadStarted(false) {
        }
        template<typename Fn, typename... Args>
        Thread(Fn &&Fx, Args &&... Ax): mThread(std::forward<Fn>(Fx), std::forward<Args>(Ax)...), mThreadFinished(false), mThreadStarted(true)
        {}
        Thread(const Thread &) = delete;
        Thread(Thread &&t) noexcept : mThread(std::move(t.mThread)), mThreadFinished(t.mThreadFinished.load()),
                                      mThreadStarted(true) {
        }

        virtual ~Thread() {
            if (mThread.joinable()) {
                mThreadRequestStop.store(true);
                mThread.join();
            }
        }

        Thread &operator=(Thread &&t) noexcept {
            this->Stop();
            mThread = std::move(t.mThread);
            mThreadFinished.store(t.mThreadFinished.load());
            mThreadStarted = t.mThreadStarted;

            return *this;
        }

        template<class Fn, class... Args>
        void Start(Fn &&Fx, Args &&... Ax) {
            Stop();
            mThreadFinished.store(false);
            mThreadRequestStop.store(false);
            mThreadStarted = true;
            mThread = std::move(std::thread(std::forward<Fn>(Fx), std::forward<Args>(Ax)...));
        }

        void Stop() {
            if (!mThreadFinished.load()) {
                mThreadFinished.store(true);
                mThreadRequestStop.store(true);
            }
            if(mThread.joinable())
                mThread.join();
        }

        void Join() {
            if (mThread.joinable())
                mThread.join();
        }

        void Detach() {
            if (!mThreadFinished.load()) {
                mThread.detach();
            }
        }

        void MarkAsFinished() {
            mThreadFinished.store(true);
            if(mThread.joinable())
                mThread.join();
        }

        [[nodiscard]] bool ShouldStop() const { return mThreadRequestStop.load(); }
        [[nodiscard]] bool IsRunning() const { return (!mThreadFinished.load()) && mThreadStarted; }
        [[nodiscard]] bool HasFinished() const { return mThreadFinished.load() && mThreadStarted; }

        [[nodiscard]] std::thread &GetThread() { return mThread; }

    protected:
        std::thread mThread;
        bool mThreadStarted = false;
        std::atomic<bool> mThreadFinished;
        std::atomic<bool> mThreadRequestStop;
    };
}
