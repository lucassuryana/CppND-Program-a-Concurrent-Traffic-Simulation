#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <chrono>

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    
    std::unique_lock<std::mutex> lock(_mutex); // lock the mutex
    _cond.wait(lock, [this] {return !_queue.empty();}); // wait until the queue is not empty

    T msg = std::move(_queue.front()); //move the message from the front queue
    _queue.pop_front(); //remove message from the queue

    return msg; //return the received message
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _queue = std::make_shared<MessageQueue<TrafficLightPhase>>();
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) {
        if (_queue->receive() == TrafficLightPhase::green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method âcycleThroughPhasesâ should be started in a thread when the public method 
    // âsimulateâ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    std::random_device rd;
    std::mt19937 gen(rd()); // initialize random number generator
    std::uniform_int_distribution<> distr(4000, 6000); // define the range of time between 4 and 6 seconds 
    
    while (true) 
    {
        int cycle_duration = distr(gen); // randomly choose cycle duration between 4000 and 6000 milliseconds (4 and 6 seconds)

        auto startTime = std::chrono::system_clock::now(); // measure start time

        std::this_thread::sleep_for(std::chrono::milliseconds(cycle_duration));

        auto endTime = std::chrono::system_clock::now(); // measure end time of cycle duration
        auto elapsedTime = std::chrono::system_clock::now() - startTime;

        if (elapsedTime.count() >= cycle_duration) 
        {
            std::cout << "Elapsed time is greater than cycle duration" << std::endl;
            std::lock_guard<std::mutex> lock(_mutex); // ensure thread-safe access
            _currentPhase = (_currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red); // Toggle phase to change the traffic light from red to green or vice versa

            _queue->send(std::move(_currentPhase));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    }
}
