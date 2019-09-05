#include <iostream>
#include <random>
#include <functional>
#include <chrono>
#include <cstdlib>
#include "TrafficLight.h"
#include <assert.h>

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  	std::unique_lock<std::mutex> lock(mtx_);
  	// Although this queue is not really used in this application
  	// to asynchronously enequeue more than one element at a time
  // before the element is consumed, I wanted to give it that
  // capability. So if there is data in the deque already,
  // don't wait on the condition variable. Enter the lock and 
  // pop and return the frontmost element that was be previously enqueued, in FIFO fashion
   // Only block and wait if the queue is empty:
  	// (Otherwise we would block when there is already an element there)
  // condition_variable is harder to use than say, ResetEvent (in C#) becuase the condition_variable does not save state. The consumer needs to be waiting on it when the producer calls notify.
  // However, the deque *does* save state so let's account for that:
  	while(_queue.empty())
    {
      cond_.wait(lock);
    }
  	
   // Now that we are sure there is something already in the 	
   // queue, whether saved of newly enqueued while we waited, pop it off and return it:
  	TrafficLightPhase phase = std::move(_queue.front());
  	_queue.pop_front();
  
  	return std::move(phase);
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
	
  	std::lock_guard lock(mtx_);
  
  	_queue.push_back(msg);
  
  	cond_.notify_one();
}


/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight() :
	_currentPhase(TrafficLightPhase::red)
{
    
}

TrafficLight::~TrafficLight()
{
  // Per code review comments, I inadvertently omitted this before, but have included it now.
  // Set the flag that the loop checks to false so that the method used to launch the thread
  // returns and the thread can thus be joined:
  printf("%s: Setting loop flag to false...\n", __func__);
  doLoop_ = false;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  	while(messageQ_.receive() != TrafficLightPhase::green) ;
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
  doLoop_ = true;
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 


  // I didn't realize before that std::thread::cTor would work with instance methods so Iremove std::bind
  this->threads.emplace_back(&TrafficLight::cycleThroughPhases, this);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
  
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
 
  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
std::srand(now_ms.time_since_epoch().count());

std::chrono::system_clock::time_point nextChange = now + std::chrono::milliseconds((std::rand() % 2000) + 4000) ;

/* The reason this checks a class member boolean is so that the base class destructor can succeed (after this class' destructor sets that boolean false) in its attempt to join all threads in the 'threads' vector. A truly infinite loop would never allow for a call to thread::join() to return. Maybe std::terminate is going to destroy everything anyway, wtih this example, but the best practice rule of thumb is to implement classes that can be torn down properly. I have changed the flag to a standard boolean to avoid the overhead of an atomic, becuase all that matters with a use case like this is that the background thread's cached copy of the boolean, eventually syncs up. I forgot to set the boolean false in the destructor in the first submission, which would amount to the boolean check being nothing but overhead. */
  
while(doLoop_)
{
	now = std::chrono::system_clock::now();
	if(now < nextChange)	
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
	else
    {
     
     	_currentPhase = (_currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red);
		messageQ_.send(std::move(_currentPhase));

	    nextChange = now + std::chrono::milliseconds((std::rand() % 2000) + 4000) ;
    }
 
}
   printf("%s: Loop flag is false, returning...\n", __func__);
}