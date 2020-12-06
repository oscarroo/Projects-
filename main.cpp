//comment: Oscar Hong Tze, Lee ; 10478297 ; 20/11/2020
//pre-processor directives:
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <ctime>
#include <condition_variable>
#include <map>
#include <iterator>
#include <string>



//global constants:
int const MAX_NUM_OF_THREADS = 6;
int const NUM_OF_SAMPLES = 50;
int const NUM_OF_LINKS = 2;
//global variables:
std::mutex global_mu;
std::map<std::thread::id, int> threadIDs; //creating a map;

//function prototypes: (as required)
int get_thread_id();
void cout_replacement(std::string x);
int main();

//class prototypes
class Sensor;
class TempSensor;
class PressureSensor;
class PressureSensor;
class BC;

class SensorData;
class Receiver;
class LinkAccessController;
class Link;

class Sensor { //abstract base class that models a sensor
protected:
	std::string sensorType;
	int count = 0;
public:
	Sensor(std::string& type) : sensorType(type) {} //constructor
	//Declare a virtual method to be overridden by derived classes:
	virtual double getValue() = 0;

	std::string getType() {
		return sensorType;
	}
	void incCount() {
		count++;
	}

	int getCount() {
		return count;
	}


};
class TempSensor : public Sensor {
private :
	double value = 0;
public:
	TempSensor(std::string& s) : Sensor(s) {}
	virtual double getValue() {
		//return a random value of ambient temperature between 10 and 30
		value = rand() % 30 + 10.0;
		return value;
	}
};
class PressureSensor : public Sensor {
private :
	double value = 0;
public :
	PressureSensor(std::string& s) : Sensor(s) {} //inherited constructor
	virtual double getValue() {
		//return a random value  between 95-105
		value = rand() % 105 + 95.0;
		return value;
	}
};
class CapacitiveSensor : public Sensor {
private :
	double value = 0;
public :
	CapacitiveSensor(std::string& s) : Sensor(s) {} //inherited constructor
	virtual double getValue() {
		//return a random value between 1-5
		value = rand() % 5 + 1.0;
		return value;
	}
};
class BC {
private:
	bool lock = false; //'false' means that the BC is not locked
	std::vector<Sensor*>& theSensors; //reference to vector of Sensor pointers
	std::mutex BC_mu; //mutex
	std::condition_variable cond;

public:
	//constructor: initialises a vector of Sensor pointers that are
	//passed in by reference:
	BC(std::vector<Sensor*>& sensors) : theSensors(sensors) {}
	void requestBC() {
		std::unique_lock<std::mutex> locker(BC_mu);
		if (lock == false) {
			lock = true;
			cout_replacement("BusController locked by thread: " + std::to_string(get_thread_id()));
		}
		else {
			cout_replacement("BusController is locked, thread: " + std::to_string(get_thread_id()) + " is about to suspend.");
			while (lock) {
				cond.wait(locker);
			}
		}
	}
	void releaseBC() {

		if (lock == true) {
			lock = false;
			cout_replacement("BusController unlocked by thread: " + std::to_string(get_thread_id()));
			//std::cout << "BusController unlocked by thread: " << get_thread_id() << std::endl;
			cond.notify_all();

		}
	}
	double getSensorValue(int selector) {

		return (*theSensors[selector]).getValue();
	}
	std::string getSensorType(int selector) {
		return (*theSensors[selector]).getType();
	}
	void incSensorCount(int selector) {
		(*theSensors[selector]).incCount();
	}
}; //end class BC

class SensorData { //utility class to store sensor data
private:
	std::vector<double> sensor_data;
	std::string sensor_type;
public:
	SensorData(std::string type) : sensor_type(type) {}// constructor


	std::string getSensorType() {
		return sensor_type;
	}
	std::vector<double> getSensorData() {
		return sensor_data;
	}
	void addData(double newData) {
		sensor_data.push_back(newData);
	}
};

class Receiver {
private:
	//mutex:
	std::mutex rec_mu;
	//vectors to store sensor numeric data received from threads:
	std::vector<double> temp_data;
	std::vector<double> press_data;
	std::vector<double> cap_data;
public:
	Receiver() { } //constructor
	//Receives a SensorData object:
	void receiveData(SensorData sd) { //check what is the type of the SensorData object
		std::unique_lock<std::mutex> locker(rec_mu); //locks the receive?
		if (sd.getSensorType() == "Temperature Sensor") {
			temp_data = sd.getSensorData();
		}
		else if (sd.getSensorType() == "Pressure Sensor") {
			press_data = sd.getSensorData();
		}
		else if (sd.getSensorType() == "Capacitive Sensor") {
			cap_data = sd.getSensorData();
		}
	}
	// print out all data for each sensor:
	void printSensorData() {
		//print out temperature sensor data gathered
		std::cout << "Temperature Sensor data : ";
		for (auto i = temp_data.begin(); i != temp_data.end(); i++) {
			std::cout << *i << " ";
		}
		std::cout<<std::endl;

		//print out pressure sensor data gathered
		std::cout << "Pressure Sensor data : ";
		for (auto i = press_data.begin(); i != press_data.end(); i++) {
			std::cout << *i << " ";
		}
		std::cout << std::endl;

		//print out capacitive sensor data gathered
		std::cout << "Capacitive Sensor data : ";
		for (auto i = cap_data.begin(); i != cap_data.end(); i++) {
			std::cout << *i << " ";
		}
		std::cout << std::endl;
	}


}; //end class Receiver

class Link {
private:
	bool inUse; // true if in use, false if free
	Receiver& myReceiver; //Receiver reference
	int linkId;
public:
	Link(Receiver& r, int linkNum) : inUse(false), myReceiver(r), linkId(linkNum){}
	//check if the link is currently in use
	bool isInUse() {
		return inUse;
	}
	//set the link status to busy
	void setInUse() {
		inUse = true;
	}
	//set the link status to idle
	void setIdle() {
		inUse = false;
	}
	//write data to the receiver
	void writeToDataLink(SensorData sd) {
		myReceiver.receiveData(sd);
	}
	//returns the link Id
	int getLinkId() {
		return linkId;
	}

};
class LinkAccessController {
private:
	Receiver& myReceiver; //Receiver reference
	int numOfAvailableLinks; //number of available links at the start = 2;
	int x = 0;
	std::vector<Link> commsLinks; //create a vector of Link objects, not sure why
	std::mutex LAC_mu; //mutex
	std::condition_variable cond;

public:
	LinkAccessController(Receiver& r) : myReceiver(r), numOfAvailableLinks(NUM_OF_LINKS) //constructor
	{
		//constructor creates a vector of link objects, with id 0 and 1
		for (int i = 0; i < NUM_OF_LINKS; i++) {
			commsLinks.push_back(Link(myReceiver, i));
		}
	}
	//Request a comm's link: returns a reference to an available Link.
	//If none are available, the calling thread is suspended.
	Link& requestLink() {
		std::unique_lock<std::mutex> locker(LAC_mu);
		if (numOfAvailableLinks == 0) {
			cout_replacement("No links available, thread " + std::to_string(get_thread_id()) + " is about to suspend..");
			cond.wait(locker);
		}
		else {
			for (x = 0; x < NUM_OF_LINKS; x++) {
				 if (!commsLinks[x].isInUse()) {
					commsLinks[x].setInUse();
					numOfAvailableLinks--;
					cout_replacement("Thread " + std::to_string(get_thread_id())+ " is requesting Link " + std::to_string(x));
					return std::ref(commsLinks[x]);
				}
			}
		}

	}

	//Release a comms link:
	void releaseLink(Link& releasedLink) {
		if (releasedLink.isInUse()) {
			releasedLink.setIdle();  //set the link to idle
			numOfAvailableLinks++;
			cout_replacement("Link " + std::to_string(releasedLink.getLinkId())+ " is released.");
			   // increment the number of available links
			cond.notify_all();       // notify all other threads;
		}
	}
};


void cout_replacement(std::string x) {

	std::cout << x << std::endl;

}
// maybe can create a request for the cout class or something

int get_thread_id() {
	std::map <std::thread::id, int>::iterator it = threadIDs.find(std::this_thread::get_id());
	if (it == threadIDs.end()) return -1; //thread 'id' NOT found
	else return it->second; //thread 'id' found, return the
	//associated integer –note the syntax.
}

void run(BC& theBC, int idx, LinkAccessController& lac) { //run function – executed by each thread:
	//Sensordata obj creation
	SensorData tempSensor("Temperature Sensor");
	SensorData pressSensor("Pressure Sensor");
	SensorData capSensor("Capacitive Sensor");

	//seed rand with time
	srand(time(NULL));
	int x = 0;
	std::string sentence;

	//create the map for the thread
	std::mutex mu; //declare a mutex
	std::unique_lock<std::mutex> map_locker(mu); //lock the map via the mutex.
	threadIDs.insert(std::make_pair(std::this_thread::get_id(), idx));	//insert the threadID and id into the map:
	map_locker.unlock(); //unlock the map.

	for (int i = 0; i < NUM_OF_SAMPLES; i++) { // NUM_OF_SAMPLES = 50 (initially)

		theBC.requestBC();// request use of the BC:
		// generate a random value between 0 and 2, and use it to select a sensor and obtain a value and the sensor's type:
		x = (rand() % 3 + 1) - 1;

		//message to output to the print function
        cout_replacement("Sample value from Thread " + std::to_string(get_thread_id()) + " from " + theBC.getSensorType(x) + " = " + std::to_string(theBC.getSensorValue(x)));

		//storing sensor data in appropriate object
		if (x == 0) {tempSensor.addData(theBC.getSensorValue(x));}
		else if (x == 1) {pressSensor.addData(theBC.getSensorValue(x));}
		else{capSensor.addData(theBC.getSensorValue(x));}

		// increment counter for sensor chosen (to keep count of how many times each was used)
		theBC.incSensorCount(x);

		// release the BC:
		theBC.releaseBC();

		//delay for random period between 0.001s - 0.01s:
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10 + 1));
		} // end of for
        for (int i = 0; i < NUM_OF_SAMPLES; i++){
        //start to request a link from lac
		Link* link = &lac.requestLink();

        //choose the appropriate object to write to the receiver through a link
		link->writeToDataLink(tempSensor);
		link->writeToDataLink(pressSensor);
		link->writeToDataLink(capSensor);

		//release the link
		lac.releaseLink(*link);

		// delay for random period between 0.001s – 0.01s:
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10 + 1));
        }
} // end of run

int main() {

	//seed the random number generator with time
	srand(time(NULL));

	//declare a vector of Sensor pointers:
	std::vector<Sensor*> sensors;

	//initialise each sensor and insert into the vector from behind , 0 = temp sens, 1 = press sens, 2 = cap sens
	std::string s0 = "Temperature sensor";
	sensors.push_back(new TempSensor(s0));
	std::string s1 = "Pressure sensor";
	sensors.push_back(new PressureSensor(s1));
	std::string s2 = "Capacitive sensor";
	sensors.push_back(new CapacitiveSensor(s2));

	// Instantiate the BC:
	BC theBC(std::ref(sensors)); //wrapper class

	//Instantiate the Receiver:
	Receiver theReceiver;

	//Instantiate the LAC
	LinkAccessController lac(theReceiver);

	//instantiate and start the threads:
	std::thread the_threads[MAX_NUM_OF_THREADS]; //array of threads
	for (int i = 0; i < MAX_NUM_OF_THREADS; i++) {
		//launch the threads:
		the_threads[i] = std::thread(run,std::ref(theBC),i, std::ref(lac));
	}

	//wait for the threads to finish:
	for (int j = 0; j < MAX_NUM_OF_THREADS; j++) {
		the_threads[j].join();
	}
	std::cout << "All threads terminated" << std::endl;

	//print out the number of times each sensor was accessed:
		for (int k = 0; k < 3; k++) {
			std::cout << sensors[k]->getType() << " was accessed " << sensors[k]->getCount() << " times." << std::endl;
		}
	//print out all the sensor data
		theReceiver.printSensorData();
		return 0;
} // end of main
