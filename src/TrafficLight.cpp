#include <iostream>
#include <future>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> uLock(mtx);
    _condition.wait(uLock, [this] () -> bool { return !_queue.empty(); });
    T msg = std::move(_queue.front());
    _queue.pop();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lck(mtx);
    _queue.push(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while (true)
    {
        if (_trafficLightQueue.receive() == TrafficLightPhase::green)
            return;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(&TrafficLight::cycleThroughPhases, this);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    int duration = rand() % 2 + 4;
    auto lastPhaseChange = std::chrono::system_clock::now();
    while (true)
    {
        long timeElapsedSinceUpdate = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastPhaseChange).count();
        if (timeElapsedSinceUpdate >= duration)
        {
            _currentPhase = (TrafficLightPhase)!_currentPhase;
            std::future<void> msgSent = std::async(&MessageQueue<TrafficLightPhase>::send, &_trafficLightQueue, std::move(_currentPhase));
            msgSent.wait();
            lastPhaseChange = std::chrono::system_clock::now();
            duration = rand() % 2 + 4;
        }
        TrafficLightPhase currentPhase = getCurrentPhase();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

