#include <iostream>
#include <stdlib.h>
#include <queue>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <ncurses.h>
#include <string>
#include <iomanip>
#include <condition_variable>
#include <vector>

using namespace std;

class Client {
public:
	int id;
	string Name;
	int status;
	string order_c;
	int table_id;
	Client(int i, string m, int s) {
		id = i;
		Name = m;
		status = s;
	}
};

class Waiter {
public:
	int id;
	string Name;
	int status;
	int order_id;
	string Order_s;
	Waiter(int i, string m, int s) {
		id = i;
		Name = m;
		status = s;
	}
};

class Cook {
public:
	int id;
	string Name;
	int status;
	int order_id;
	string Order_s;
	Cook(int i, string m, int s) {
		id = i;
		Name = m;
		status = s;
	}
};

class Order {
public:
	int id;
	int status;
	string order_n;
	Order(int i, string m, int s) {
		id = i;
		order_n = m;
		status = s;
	}
};

//MUTEX
mutex mxClientsTables;
mutex mxWaiters;
mutex mxCooks;
mutex mxOrders;



//Resources:
vector <Waiter> Waiters;
vector <Cook> Cooks;
vector <Client> WaitingQueue;
vector <Client> Clients;
vector <Order> Orders;

//Static variables:
int static tableSize = 4;
int static clientsAmount = 12;
int static waitersAmount = 4;
int static cooksAmount = 3;

//Other variables
int iter1, iter2, iter3, iter4 = 0; //for displaing "nothing"
int var = 0; //needed this for cooking iterating
int TimeRng = 5;
int TimeRng2 = 10;
vector <string> Copy;


int ClientsAtTables = 0;
int WaitersReady = 0;
int CooksReady = 0;
int FoodsReady = 0;
int CookedReady = 0;
int TablesReady = 0;
int full = 0;
int endX = 0;
condition_variable endVar;
condition_variable ready;
condition_variable CookedR;
condition_variable FoodsR;
condition_variable ClientR;
condition_variable WaiterR;
condition_variable CookR;
condition_variable TablesR;

string generateName();
string generateFood();
void ClientsFabric(int number);
void WaitersFabric(int number);
void CooksFabric(int number);
void ClientsToTables(int number);
void Food(int number);
void Cooking(int number2);
void deleteQueue(int number);
void displayQueue();
void displayTables();
void displayWaiters();
void displayCooks();
void displayOrders();
void Ncurses();
