
#ifndef __GAME_TIMTER__H__
#define __GAME_TIMTER__H__

#include <chrono>

//----------------------------------------------------------------
// GameTimer
//----------------------------------------------------------------
class GameTimer
{
public:
    using SteadyClock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<SteadyClock>;
    using NanoSeconds = std::chrono::nanoseconds;
    using Seconds = std::chrono::duration<float>;
    
public:
    GameTimer(void) = default;

    float DeltaTime(void) const;
    float TotalTime(void) const;

    void Reset(void);
    void Start(void);
    void Stop(void);
    void Tick(void);

private:
    double deltaTime_ = -1.0;

    NanoSeconds pausedDuration_;
    TimePoint baseTimePoint_;    
    TimePoint stopTimePoint_;
    TimePoint prevTimePoint_;
    TimePoint curTimePoint_;

    bool stopped_ = false;
};

#endif /*__GAME_TIMTER__H__*/