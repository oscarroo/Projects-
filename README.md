# Projects-
Concurrent Systems Project 

Simulating an embedded computer system.

This is a model of a system that is responsible for gathering environmental data from a set of three sensors, 
each of a different type. In the model a set of threads (initially six) are used to gather the data, and then
transmit it to a Receiver.Access to the sensors is via a Bus Controller (BC), such that only one thread is granted access at
a time. Thus, in order to acquire data, each thread must gain sole access to the BC, configure it
to sample from a specific sensor, and sample the data. In order to transmit the data, a thread
must gain access to one of two communications links via a Link Access Controller. Having gained
access, it sends its data, and then releases the link. Each thread may use either communications
link, but once it has gained access to a particular link, no other thread should be allowed to use
that link until it is released.

1st Attempt at applying multithreading to a C++ program.
