
#include "gametimer.h"

//------------------------------------------------------------------------------------------
// GameTimer
//------------------------------------------------------------------------------------------
float GameTimer::DeltaTime(void) const
{
    return static_cast<float> (deltaTime_);
}

float GameTimer::TotalTime(void) const
{
    NanoSeconds totalTimeInNano;
    if (stopped_ == true)
    {
        totalTimeInNano = (stopTimePoint_ - pausedDuration_) - baseTimePoint_;
    }
    else
    {
        totalTimeInNano = (curTimePoint_ - pausedDuration_) - baseTimePoint_;
    }

    return std::chrono::duration_cast<Seconds> (totalTimeInNano).count();
}

void GameTimer::Reset(void)
{
    curTimePoint_ = SteadyClock::now();

    baseTimePoint_ = curTimePoint_;
    prevTimePoint_ = curTimePoint_;
    stopTimePoint_ = TimePoint::min();
    stopped_ = false;
}

void GameTimer::Start(void)
{
    if (stopped_ == true)
    {
        TimePoint curTime = SteadyClock::now();

        pausedDuration_ = curTime - stopTimePoint_;
        prevTimePoint_ = curTime;
        stopTimePoint_ = TimePoint::min();
        stopped_ = false;
    }
}

void GameTimer::Stop(void)
{
    if (stopped_ == false)
    {
        stopTimePoint_ = SteadyClock::now();
        stopped_ = true;
    }
}

void GameTimer::Tick(void)
{
    if (stopped_ == true)
    {
        deltaTime_ = 0.0;
        return;
    }

    curTimePoint_ = std::chrono::steady_clock::now();
    deltaTime_ = std::chrono::duration_cast<Seconds> (curTimePoint_ - prevTimePoint_).count();
    prevTimePoint_ = curTimePoint_;

    if (deltaTime_ < 0.0l)
    {
        deltaTime_ = 0.0;
    }
}
