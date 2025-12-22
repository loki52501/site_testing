# My readings on DDIA ch1

what i learnt from DDIA ch1, also I'm using a new note taking app called logseq, i'm not yet familiar with it, so it is bound to make mistakes, as my static site generator doesn't yet transform few things yet from markdown to html.


 Most systems today are lacking architecture for data intensive , than compute intensive, as there is more power for compute intensive than the idea or how to manipulate huge amounts of data to a viable entity.
Designing Data systems require 3 important things:
	- Reliability - should work correctly even in face of adversity or failures.
	- scalability - As data volume or system grows, there should be reasonable ways of dealing with the growth.
	- Maintainability - The system should be able to be worked on productively by multiple people.

## Reliability
	- we can test reliability by using tools like chaos monkey software, which tests all the edge cases,
	- Reliability should have the ability to deal with these kinds of faults:
		- Hardware Faults
		- Software Faults
		- Human Errors
### Hardware Faults
		- RAM, Hard disk crashes, power grid has a blackout, wrongly unplugging the network cable, these are some of the hardware faults
		- solutions to this are usually redundancies like RAID, backup generators, dual power supplies and hot swappable CPUs.
		- These aren't enough when data volume and computing demand ^es, ^es no of machines, ^es hardware faults, here the quality or the solution lies on software fault tolerance techniques that can tolerate the loss of entire machines, additional to hardware redundancy.
### Software Errors
		- system failures in large data centers are primarily because of software bugs, and some sort of cascading failures.
		- Alerts can be used to detect discrepancies,
### Human Errors
		- humans are unreliable, no matter what.
		- so, we make well designed abstractions, APIs and admin interfaces, these influences people who are working on a software to implement good practices rather than get influenced by wrong choices.
		- Decouple the places where people make more mistakes, from the places where they can cause failures, use things like sandbox experiments to test out the functionality before merging in the developmentation.
		- Test software for edge cases
		- Make sure that you can easily and quickly recover from human errors.
		- Use telemetry
		- good management and training.
## Scalability
	- The more the load increases, the system degrades, only solution is to improve the system as it scales.
### Describing Load
		- Load parameters -> can be -> requests/sec, reads to writes in DBs, No.of concurrent users, the hit rate on a cache. example was twitter's architecture of how the tweets are merged in followers home timeline, where they used two architectures, one is fanout and the other is writing simultaneously, when tweet is tweeted, in the home timeline of the followers, so people with more followers use this, and less ones will be used for fanout, where the write operation is heavy than the read operation.
### Performance
		- Mostly measure in response time of the user and latency, here latency is the time taken to complete a request, as requests had to wait in a queue before getting processed.
		- Measure this using percentile , such that it will be easier to make decisions, so for eg, if by average percentile, like 50% of the request is processed within 250ms, and there are tail percentile or 99% .. which takes longer time to process a request.
		- there is a problem here known as head of line blocking , which happens if a tail end request happens , and it takes a lot of time to process, making the queued requests to delay.
		- solution is to predict which functionality of a software will be used mostly and scaling it than scaling the whole system.
		- tail percentile amplification that happens when the user sends multiple requests of large scales.
		- The head of line blocking and tail percentile amplification or performance bottlenecks.
## Maintainability
	- Involves how a software is designed.
	- it has 3 criterias
### Operability
		- making routine tasks easy such as monitoring health of the system.
### Simplicity
		- Big ball of mud , the software is mired in complexity. we have to design software such that it is easy to understand.
### Evolvability
		- The ease at which you can modify and incrementally test features in a software.
	-
-