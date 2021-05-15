#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
TrafficLightPhase&& MessageQueue::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
   std::unique_lock<std::mutex> lock(_mutex);
   _condition.wait(lock, [this] { return !_queue.empty(); }); 
  
   TrafficLightPhase&& msg = std::move(_queue.back());
   _queue.pop_back();
 
   return std::move(msg);
}

void MessageQueue::send(TrafficLightPhase &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  std::lock_guard<std::mutex> lck(_mutex);
  
  _queue.clear();
  _queue.push_back(std::move(msg));
  _condition.notify_one();
  
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  	
  	while (true){
      if (_msg_queue.receive() == green){
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
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
  
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
  	auto time = std::chrono::system_clock::now();	
  	long time_diff;
  	int duration = rand() % 3 + 4; //4-6 seconds
  	while (true)
    {
      //std::this_thread::sleep_for(std::chrono::seconds(30));
	  auto tmp_time = std::chrono::system_clock::now();   
      time_diff = std::chrono::duration_cast<std::chrono::seconds>(tmp_time - time).count();
      	
      if (time_diff >= duration)
      {
        TrafficLightPhase tmpPhase;
        if (_currentPhase == TrafficLightPhase::green){
          _currentPhase = TrafficLightPhase::red;
        }
        else{
           _currentPhase = TrafficLightPhase::green;
        }
        tmpPhase = _currentPhase;
        
        _msg_queue.send(std::move(tmpPhase));
        duration = rand() % 3 + 4;
        time = tmp_time;
      }
      
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

