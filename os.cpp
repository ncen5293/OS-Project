//Nicky Cen
//OS PROJECT
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

struct PageTable {
	unsigned int page = 0;
	unsigned int PID = 0;
	unsigned int passedtime = 0;
};

struct Memory {
	unsigned int size;
	unsigned int time = 0;
	std::vector<PageTable> Frame;
};

struct Process {
	unsigned int timeLeft;
	unsigned int PID = 0;
	unsigned int level;
	std::string filename;
};

struct Disk {
	std::vector<Process> waiting;
	Process inUse;
	bool isEmpty = true;
};

struct Core {
	Process process;
	bool isEmpty = true;
};

struct Scheduler {
	std::vector<Process> levelZero;
	std::vector<Process> levelOne;
	std::vector<Process> levelTwo;
	unsigned int newestPID = 0;

	Core CPU;
};

void allocateMemory(Memory &RAM, Scheduler &Queue, Process newProcess);
void leastRecentlyUsed(Memory &RAM, unsigned int PID, unsigned int pagenum);
Process addProcess(Scheduler &Queue);
Process startingProcess(Scheduler &Queue);
Process anotherProcess(Scheduler &Queue);
Process newLevelZero(Scheduler &Queue);
Process swapNewProcess(Scheduler &Queue);
Process createProcess();
void Tick(Scheduler &Queue);
void insertFront(Scheduler &Queue, Process &pro);
void insertToCPU(Scheduler &Queue);
void Terminate(Scheduler &Queue);
void deleteProcessMemory(Memory &RAM, unsigned int pid);
void read_write(Scheduler &Queue, Disk &HDD, std::string filename);
void finishDisk(Scheduler &Queue, Disk &HDD);
void memoryOP(Memory &RAM, Scheduler Queue, unsigned int address);
void CPUandQueue(Scheduler &Queue);
void HDDs_Queues_and_Filenames(Disk &HDD);
void Memory_Pages_and_Frames(Memory &RAM);

int main()
{
	unsigned int mem, page;
	unsigned int hdd, num, addr;
	Scheduler queue;

	std::string input, file = "";

	std::cout << "Enter total amount of RAM memory: ";
	while (!(std::cin >> mem)) {
		std::cin.clear();
		std::cin.ignore(999, '\n');
		std::cout << "Bad input, Enter total amount of RAM memory: ";
	}
	Memory RAM;

	std::cout << "Enter page size: ";
	while (!(std::cin >> page)) {
		std::cin.clear();
		std::cin.ignore(999, '\n');
		std::cout << "Bad input, Enter page size: ";
	}
	RAM.size = mem / page;
	//to populate memory vector so it can go to any position
	PageTable none;
	for (unsigned int i = 0; i < RAM.size; i++)
		RAM.Frame.push_back(none);

	std::cout << "Enter total number of hard disks: ";
	while (!(std::cin >> hdd)) {
		std::cin.clear();
		std::cin.ignore(999, '\n');
		std::cout << "Bad input, Enter total number of hard disks: ";
	}
	std::vector<Disk> harddrive;
	//to populate harddrive vector so it can go to any position
	Disk empty;
	for (unsigned int i = 0; i<hdd; i++)
		harddrive.push_back(empty);

	Process newP;
	//enter any command until program termination
	std::cout << "Input commands: \n";
	while (std::cin >> input) {
		if (input == "A") {
			newP = addProcess(queue);
			allocateMemory(RAM, queue, newP);
		}
		else if (input == "Q") {
			Tick(queue);
		}
		else if (input == "t") {
			deleteProcessMemory(RAM, queue.CPU.process.PID);
			Terminate(queue);
		}
		else if (input == "d") {
			std::cin >> num;
			std::cin >> file;
			if (num >= hdd || file == "")
				std::cout << "Bad input\n";
			else
				read_write(queue, harddrive[num], file);
		}//read write to a hdd from process
		else if (input == "D") {
			std::cin >> num;
			//if there is a number that represents a hdd in this os
			if (num < hdd)
				finishDisk(queue, harddrive[num]);
			else
				std::cout << "Bad Input, there are only " << hdd << "Disks available\n";
		}
		else if (input == "m") {
			std::cin >> addr;
			addr = addr/page;
			memoryOP(RAM, queue, addr);
		}
		else if (input == "S") {
			std::cin >> input;
			if (input == "r") {
				CPUandQueue(queue);
			}
			else if (input == "i")
				for (unsigned int k = 0; k < hdd; k++) {
					std::cout << "Disk " << k << '\n';
					HDDs_Queues_and_Filenames(harddrive[k]);
				}
			else if (input == "m") {
				Memory_Pages_and_Frames(RAM);
			}
			else
				std::cout << "Bad input, enter 'r' 'i' or 'm' after 'S'\n";
		}
		else
			std::cout << "Bad input\n";
	}
	return 0;
}

//make first page in memory for a new process
void allocateMemory(Memory &RAM, Scheduler &Queue, Process newProcess) {
	int size = RAM.Frame.size();
	RAM.time++;
	bool isFull = true;
	//find free frame from start to end to put page into
	for (unsigned int i = 0; i < size; i++) {
		if (RAM.Frame[i].PID == 0 && isFull) {
			RAM.Frame[i].PID = newProcess.PID;
			RAM.Frame[i].passedtime = RAM.time;
			RAM.Frame[i].page = 0;
			isFull = false;
		}
	}
	if (isFull)
		leastRecentlyUsed(RAM, newProcess.PID, 0);
}

//replaces frame that was not used the longest
void leastRecentlyUsed(Memory &RAM, unsigned int PID, unsigned int pagenum) {
	unsigned int lastused = RAM.time;
	unsigned int loc = 0;
	//checks every frame and gets the position of the least recently used frame
	for (unsigned int i = 0; i < RAM.size; i++) {
		if (RAM.Frame[i].passedtime < lastused) {
			loc = i;
			lastused = RAM.Frame[i].passedtime;
		}
	}
	RAM.Frame[loc].PID = PID;
	RAM.Frame[loc].page = pagenum;
	RAM.Frame[loc].passedtime = RAM.time;
}

//add new process
Process addProcess(Scheduler &Queue) {
	if (Queue.CPU.isEmpty) {
		return startingProcess(Queue);
	}
	else if (!Queue.CPU.isEmpty) {
		return anotherProcess(Queue);
	}
}

//if cpu is empty, make a new process in it
Process startingProcess(Scheduler &Queue) {
	Queue.CPU.process = createProcess();
	Queue.newestPID++;
	Queue.CPU.process.PID = Queue.newestPID;
	Queue.CPU.isEmpty = false;
	return Queue.CPU.process;
}

//if cpu is full, make a new process and check cpu process
Process anotherProcess(Scheduler &Queue) {
	Process newP;
	if (Queue.CPU.process.level == 0) {
		return newLevelZero(Queue);
	}
	else if (Queue.CPU.process.level != 0) {
		return swapNewProcess(Queue);
	}
}

//make a new process in level 0 queue
Process newLevelZero(Scheduler &Queue) {
	Process pro = createProcess();
	Queue.newestPID++;
	pro.PID = Queue.newestPID;
	Queue.levelZero.push_back(pro);
	return pro;
}

//make a new process in cpu and put the old cpu process into its level queue
Process swapNewProcess(Scheduler &Queue) {
	Process currentrunning = Queue.CPU.process;
	insertFront(Queue, currentrunning);
	return startingProcess(Queue);
}

//initialize the process
Process createProcess() {
	Process pro;
	pro.timeLeft = 1;
	pro.PID = 1;
	pro.level = 0;
	return pro;
}

//go to the next time quantum
void Tick(Scheduler &Queue) {
	Process current = Queue.CPU.process;
	if (current.timeLeft > 0)
		current.timeLeft--;
	if (current.timeLeft == 0) {
		current.level++;
		if (current.level == 1) {
			current.timeLeft = 2;
			Queue.levelOne.push_back(current);
		}
		else if (current.level == 2) {
			current.timeLeft = -1;
			Queue.levelTwo.push_back(current);
		}
		insertToCPU(Queue);
	}
	else if (current.timeLeft > 0) {
		Queue.CPU.process = current;
	}
}

//insert a process to the front of its queue level to preempt every other process into that queue
void insertFront(Scheduler &Queue, Process &pro) {
	unsigned int size;
	if (pro.level == 1) {
		Queue.levelOne.push_back(pro);
		if (Queue.levelOne.size() > 1) {
			size = Queue.levelOne.size();
			for (int i = size - 2; i >= 0; i--)
				Queue.levelOne[i + 1] = Queue.levelOne[i];
			Queue.levelOne[0] = pro;
		}
	}
	else if (pro.level == 2) {
		Queue.levelTwo.push_back(pro);
		if (Queue.levelTwo.size() > 1) {
			size = Queue.levelTwo.size();
			for (int i = size - 2; i >= 0; i--)
				Queue.levelTwo[i + 1] = Queue.levelTwo[i];
			Queue.levelTwo[0] = pro;
		}
	}
}

//turn process in cpu into highest level process in queue
void insertToCPU(Scheduler &Queue) {
	if (Queue.levelZero.size() != 0) {
		Queue.CPU.process = Queue.levelZero[0];
		Queue.levelZero.erase(Queue.levelZero.begin());
	}
	else if (Queue.levelOne.size() != 0) {
		Queue.CPU.process = Queue.levelOne[0];
		Queue.levelOne.erase(Queue.levelOne.begin());
	}
	else if (Queue.levelTwo.size() != 0) {
		Queue.CPU.process = Queue.levelTwo[0];
		Queue.levelTwo.erase(Queue.levelTwo.begin());
	}
	else {
		std::cout << "No processes available\n";
		Queue.CPU.isEmpty = true;
	}
}

//removes process in cpu then inserts new process from queue
void Terminate(Scheduler &Queue) {
	Process empty;
	Queue.CPU.process = empty;
	insertToCPU(Queue);
}

//deletes all pages with a certain pid
void deleteProcessMemory(Memory &RAM, unsigned int pid) {
	unsigned int size = RAM.Frame.size();
	for (unsigned int i = 0; i < size; i++) {
		if (RAM.Frame[i].PID == pid) {
			RAM.Frame[i].PID = 0;
			RAM.Frame[i].page = 0;
			RAM.Frame[i].passedtime = 0;
		}
	}
}

//move process in cpu into a disk with a filename
void read_write(Scheduler &Queue, Disk &HDD, std::string filename) {
	Queue.CPU.process.filename = filename;
	if (!Queue.CPU.isEmpty) {
		if (HDD.inUse.PID == 0) {
			HDD.inUse = Queue.CPU.process;
		}
		else {
			HDD.waiting.push_back(Queue.CPU.process);
		}
		HDD.isEmpty = false;
		Process empty;
		Queue.CPU.process = empty;
		insertToCPU(Queue);
	}
	else
		std::cout << "No available process to read/write\n";
}

//put current process using the disk back into the queue or cpu
void finishDisk(Scheduler &Queue, Disk &HDD) {
	//if disk process is higher level, preempt the cpu process
	if (Queue.CPU.process.level > HDD.inUse.level) {
		insertFront(Queue, Queue.CPU.process);
		Queue.CPU.process = HDD.inUse;
		if (Queue.CPU.process.level == 1)
			Queue.CPU.process.timeLeft = 2;
	}
	//if not higher level, just put at back of queue
	else if (HDD.inUse.level == 0)
		Queue.levelZero.push_back(HDD.inUse);
	else if (HDD.inUse.level == 1) {
		HDD.inUse.timeLeft = 2;
		Queue.levelOne.push_back(HDD.inUse);
	}
	else if (HDD.inUse.level == 2)
		Queue.levelTwo.push_back(HDD.inUse);

	HDD.isEmpty = true;
	HDD.inUse.PID = 0;
	//gets the next process in queue for the disk if there is one
	if (HDD.waiting.size() != 0) {
		HDD.inUse = HDD.waiting[0];
		HDD.waiting.erase(HDD.waiting.begin());
		HDD.isEmpty = false;
	}

}

//go to a specific page of the cpu process and check it in memory frame
void memoryOP(Memory &RAM, Scheduler Queue, unsigned int address) {
	RAM.time++;
	bool found = false;
	//checks if the page is already in frame and make it recently used
	for (int i = 0; i < RAM.Frame.size(); i++) {
		if (RAM.Frame[i].PID == Queue.CPU.process.PID && RAM.Frame[i].page == address) {
			RAM.Frame[i].passedtime = RAM.time;
			found = true;
			break;
		}
	}
	//if page not in frame, then put it in the frame
	if (!found) {
		for (int i = 0; i < RAM.Frame.size(); i++) {
			if (RAM.Frame[i].PID == 0) {
				RAM.Frame[i].PID = Queue.CPU.process.PID;
				RAM.Frame[i].page = address;
				RAM.Frame[i].passedtime = RAM.time;
				return;
			}
		}
		//if memory is full, replace least recently used page with it
		leastRecentlyUsed(RAM, Queue.CPU.process.PID, address);
	}
}

//print out cpu process and multi-level queue processes
void CPUandQueue(Scheduler &Queue) {
	std::cout << '\n' << "------------------------------\n";
	std::cout << "Current Process in CPU: ";
	if (!Queue.CPU.isEmpty)
		std::cout << Queue.CPU.process.PID << " [Level " << Queue.CPU.process.level << "]\n";
	else
		std::cout << "[NONE]\n";
	std::cout << "Level Zero Queue:\n" << "	[CPU]<---";
	for (int i = 0; i < Queue.levelZero.size(); i++) {
		std::cout << Queue.levelZero[i].PID << "<---";
	}
	std::cout << '\n' << "Level One Queue:\n" << "	[CPU]<---";
	for (int i = 0; i < Queue.levelOne.size(); i++) {
		std::cout << Queue.levelOne[i].PID << "<---";
	}
	std::cout << '\n' << "Level Two Queue:\n" << "	[CPU]<---";
	for (int i = 0; i < Queue.levelTwo.size(); i++) {
		std::cout << Queue.levelTwo[i].PID << "<---";
	}
	std::cout << '\n' << "------------------------------\n";
}

//print out disk process and its queue processes
void HDDs_Queues_and_Filenames(Disk &HDD) {
	if (!HDD.isEmpty) {
		std::cout << "Current Process: " << HDD.inUse.PID;
		std::cout << " --- Reading/Writing " << HDD.inUse.filename << '\n';
		std::cout << "Queue: [DISK]<---";
		for (int i = 0; i < HDD.waiting.size(); i++)
			std::cout << HDD.waiting[i].PID << "<---";
	}
	else
		std::cout << "[EMPTY]";
	std::cout << '\n' << "------------------------------\n";
}

//prints out everything in memory that has pages in it
void Memory_Pages_and_Frames(Memory &RAM) {
	std::cout << "Current Memory Usage:\n";
	for (int i = 0; i < RAM.Frame.size(); i++) {
		if (RAM.Frame[i].PID != 0)
			std::cout << "[FRAME: " << i << "] --- [PAGE: " << RAM.Frame[i].page << "] --- [PID: " << RAM.Frame[i].PID << "] --- [time: " << RAM.Frame[i].passedtime << "]\n";
	}
	std::cout << "------------------------------\n";
}
