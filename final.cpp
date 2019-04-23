#include "header.h"
#include "names.h"

int main() {
	srand(time(NULL));

	//threads
	thread TNcurses(Ncurses);
	thread TClients(ClientsFabric, clientsAmount);
	thread TClientsToTables(ClientsToTables, clientsAmount);
	thread TWaiters(WaitersFabric, waitersAmount);
	thread TCooks(CooksFabric, cooksAmount);
	thread TOrders(Food, clientsAmount);
	thread TEat(Cooking, clientsAmount);
	thread TDelete(deleteQueue, clientsAmount);

	//joins
	TClients.join();
	TClientsToTables.join();
	TWaiters.join();
	TCooks.join();
	TOrders.join();
	TEat.join();
	TDelete.join();
	TNcurses.join();

	return 0;
}

//Generates names
string generateName(){
  string name = table_with_names[rand() % (sizeof(table_with_names)/sizeof(*table_with_names))];
  return name;
}

//Generates food names
string generateFood(){
  string name = table_with_foods[rand() % (sizeof(table_with_foods)/sizeof(*table_with_foods))];
  return name;
}

//Generates clients
void ClientsFabric(int number) {
	srand(time(NULL));
	int iter = 1;
	do {
		Client k1(iter, generateName(), 1);
		unique_lock<mutex> locker(mxClientsTables);
		WaitingQueue.push_back(k1);
		locker.unlock();
		iter++;
		number--;
	} while (number);
	full = 1;
	ready.notify_all();
}

//Generates waiters. Need to wait for ClientsFabric() end.
void WaitersFabric(int number) {
	srand(time(NULL));
	int iter = 1;
	do {
		Waiter k1(iter, generateName(), 1);
		unique_lock<mutex> locker(mxWaiters);
		ready.wait(locker, [] {return full == 1; });
		Waiters.push_back(k1);
		locker.unlock();
		this_thread::sleep_for(std::chrono::seconds(1));
		iter++;
		number--;
	} while (number);
	WaitersReady = 1;
	WaiterR.notify_all();
}

//Generates cooks.
void CooksFabric(int number) {
	srand(time(NULL));
	int iter = 1;
	do {
		Cook k1(iter, generateName(), 1);
		unique_lock<mutex> locker(mxCooks);
		ready.wait(locker, [] {return full == 1; });
		//WaiterR.wait(locker, [] {return WaitersReady == 1; });
		Cooks.push_back(k1);
		locker.unlock();
		this_thread::sleep_for(std::chrono::seconds(1));
		iter++;
		number--;
	} while (number);
	CooksReady = 1;
	CookR.notify_all();
}

//Taking people to tables when waiters and cooks ready.
void ClientsToTables(int number) {
	int table1 = 0;
	int table2 = 0;
	int table3 = 0;
	int table4 = 0;
	unique_lock<mutex> locker(mxClientsTables);
	CookR.wait(locker, [] {return CooksReady == 1; });
	WaiterR.wait(locker, [] {return WaitersReady == 1; });
	locker.unlock();

	if (WaitingQueue.size() != 0) {
		do {
			if (!WaitingQueue.empty()) {
				if (table1 < tableSize) {
					mxClientsTables.lock();
					WaitingQueue.front().table_id = 1;
					Clients.push_back(WaitingQueue.front());
					WaitingQueue.erase(WaitingQueue.begin());
					mxClientsTables.unlock();
					table1++;
				}
				else if (table2 < tableSize) {
					mxClientsTables.lock();
					WaitingQueue.front().table_id = 2;
					Clients.push_back(WaitingQueue.front());
					WaitingQueue.erase(WaitingQueue.begin());
					mxClientsTables.unlock();
					table2++;
				}
				else if (table3 < tableSize) {
					mxClientsTables.lock();
					WaitingQueue.front().table_id = 3;
					Clients.push_back(WaitingQueue.front());
					WaitingQueue.erase(WaitingQueue.begin());
					mxClientsTables.unlock();
					table3++;
				}
				else if (table4 < tableSize) {
					mxClientsTables.lock();
					WaitingQueue.front().table_id = 4;
					Clients.push_back(WaitingQueue.front());
					WaitingQueue.erase(WaitingQueue.begin());
					mxClientsTables.unlock();
					table4++;
				}
				else {
					continue;
				}
			}
			this_thread::sleep_for(std::chrono::seconds(1));
			number--;
		} while (number != 0);
	}
	ClientsAtTables = 1;
	ClientR.notify_all();
}

void Food(int number) {
	srand(time(NULL));
	unique_lock<mutex> locker(mxClientsTables);
	ClientR.wait(locker, [] {return ClientsAtTables == 1; });
	locker.unlock();
	for (int number = 0; number < (int)Clients.size(); number++) {
		if (Clients[number].status == 1) {
			Clients[number].status = 2;
			string s = generateFood();
			Clients[number].order_c = s;
			Copy.push_back(s);


			unique_lock<mutex> locker4(mxOrders);
			Order z1(Clients[number].id, Clients[number].order_c, 1);
			Orders.push_back(z1);


			int iterWaiter = number % waitersAmount;
			unique_lock<mutex> locker2(mxWaiters);
			if (Waiters[iterWaiter].status == 1) {
				Waiters[iterWaiter].status = 2;

				Waiters[iterWaiter].order_id = Orders[number].id;
				Waiters[iterWaiter].Order_s = Orders[number].order_n;
			}
			locker2.unlock();

			int iterCook = number % cooksAmount;
			unique_lock<mutex> locker3(mxCooks);
			if (Cooks[iterCook].status == 1) {
				Cooks[iterCook].status = 2;

				Cooks[iterCook].order_id = Orders[number].id;
				Cooks[iterCook].Order_s = Orders[number].order_n;
			}
			locker3.unlock();

			locker4.unlock();

		}
		if (Orders.size() >= 4) {
			FoodsReady = 1;
			FoodsR.notify_all();
		}
		int timeVar =  rand() % TimeRng;
		this_thread::sleep_for(std::chrono::seconds(timeVar));
	}
}

//Cooking process.
void Cooking(int number2) {
	srand(time(NULL));
	do {
		if (Orders.empty()) {
			continue;
		}
		else {
			unique_lock<mutex> locker(mxOrders);
			FoodsR.wait(locker, [] {return FoodsReady == 1; });
			Orders[var].status = 2;
			Clients[var].status = 3;

			unique_lock<mutex> locker4(mxWaiters);
			for (int i = 0; i < (int)Waiters.size(); i++)
			{
				if (Waiters[i].order_id == Orders[var].id) {
					if (Waiters[i].order_id + waitersAmount <= clientsAmount) {
						Waiters[i].status = 2;

						Waiters[i].order_id += waitersAmount;
						Waiters[i].Order_s = Copy.back();
					}
					else {
						Waiters[i].status = 3;
					}
				}
			}
			locker4.unlock();

			unique_lock<mutex> locker5(mxCooks);
			for (int i = 0; i < (int)Cooks.size(); i++)
			{
				if (Cooks[i].order_id == Orders[var].id) {
					if (Cooks[i].order_id + cooksAmount <= clientsAmount) {
						Cooks[i].status = 2;

						Cooks[i].order_id += cooksAmount;
						Cooks[i].Order_s = Copy.back();

					}
					else {
						Cooks[i].status = 3;
					}
				}
			}
			locker5.unlock();

			locker.unlock();
			var++;
			number2--;
			if (var == clientsAmount) {
				CookedReady = 1;
				CookedR.notify_all();
			}

		}
		int timeVar =  rand() % TimeRng;
		this_thread::sleep_for(std::chrono::seconds(timeVar));
	} while (number2);
}

//Delete Clients and Orders when done.
void deleteQueue(int number) {
	srand(time(NULL));
	do {
		unique_lock<mutex> locker(mxClientsTables);
		CookedR.wait(locker, [] {return CookedReady == 1; });

		Clients.erase(Clients.begin());
		Orders.erase(Orders.begin());

		locker.unlock();
		int timeVar =  rand() % TimeRng;//1
		this_thread::sleep_for(std::chrono::seconds(timeVar));
		number--;
	} while (number);
	if(Orders.size() == 0){
		this_thread::sleep_for(std::chrono::seconds(2));
		endX = 1;
		endVar.notify_all();
	}
}

void displayQueue() {
	unique_lock<mutex> locker(mxClientsTables);

	attron(COLOR_PAIR(1));
	printw("Queue:\n");
	attroff(COLOR_PAIR(1));
	if (!WaitingQueue.empty()) {
		for (int i = 0; i < (int)WaitingQueue.size(); i++)
		{
			printw("  %2d: %s\n", WaitingQueue[i].id, WaitingQueue[i].Name.c_str());
		}
	}
	else {
		printw("There is nothing.\n");
	}
	printw("\n\n");
	locker.unlock();
}

void displayTables() {

	unique_lock<mutex> locker(mxClientsTables);
	bool n1 = false;
	bool n2 = false;
	bool n3 = false;
	bool n4 = false;
		string status;
		attron(COLOR_PAIR(1));
		printw("Table 1:\n");
		attroff(COLOR_PAIR(1));
		for (int i = 0; i < (int)Clients.size(); i++) {
			if (Clients[i].table_id == 1) {
				if (Clients[i].status == 2) {
					attron(COLOR_PAIR(6));
					status = "Orders: " + Clients[i].order_c;
					printw("  %2d: %s\t\t%s\n", Clients[i].id, Clients[i].Name.c_str(), status.c_str());
					attroff(COLOR_PAIR(6));
				}
				else if(Clients[i].status == 3){
					attron(COLOR_PAIR(2));
					status = "Eats:  " + Clients[i].order_c;
					printw("  %2d: %s\t\t%s\n", Clients[i].id, Clients[i].Name.c_str(), status.c_str());
					attroff(COLOR_PAIR(2));
				}
				else {
					attron(COLOR_PAIR(4));
					status = "Waiting for waiter.";
					printw("  %2d: %s\t\t%s\n", Clients[i].id, Clients[i].Name.c_str(), status.c_str());
					attroff(COLOR_PAIR(4));
				}
				iter1++;
			}
			else {
				if (!n1 && iter1 == 0) {
					printw("There is nothing.\n");
					n1 = true;
				}
				else {
					continue;
				}

			}
		}
		attron(COLOR_PAIR(1));
		printw("Table 2:\n");
		attroff(COLOR_PAIR(1));
		for (int i = 0; i < (int)Clients.size(); i++) {
			if (Clients[i].table_id == 2) {
				if (Clients[i].status == 2) {
					attron(COLOR_PAIR(6));
					status = "Orders: " + Clients[i].order_c;
					printw("  %2d: %s\t\t%s\n", Clients[i].id, Clients[i].Name.c_str(), status.c_str());
					attroff(COLOR_PAIR(6));
				}
				else if (Clients[i].status == 3) {
					attron(COLOR_PAIR(2));
					status = "Eats:  " + Clients[i].order_c;
					printw("  %2d: %s\t\t%s\n", Clients[i].id, Clients[i].Name.c_str(), status.c_str());
					attroff(COLOR_PAIR(2));
				}
				else {
					attron(COLOR_PAIR(4));
					status = "Waiting for waiter.";
					printw("  %2d: %s\t\t%s\n", Clients[i].id, Clients[i].Name.c_str(), status.c_str());
					attroff(COLOR_PAIR(4));
				}
				iter2++;
			}
			else {
				if (!n2 && iter2 == 0) {
					printw("There is nothing.\n");
					n2 = true;
				}
				else {
					continue;
				}

			}
		}
		attron(COLOR_PAIR(1));
		printw("Table 3:\n");
		attroff(COLOR_PAIR(1));
		for (int i = 0; i < (int)Clients.size(); i++) {
			if (Clients[i].table_id == 3) {
				if (Clients[i].status == 2) {
					attron(COLOR_PAIR(6));
					status = "Orders: " + Clients[i].order_c;
					printw("  %2d: %s\t\t%s\n", Clients[i].id, Clients[i].Name.c_str(), status.c_str());
					attroff(COLOR_PAIR(6));
				}
				else if (Clients[i].status == 3) {
					attron(COLOR_PAIR(2));
					status = "Eats:  " + Clients[i].order_c;
					printw("  %2d: %s\t\t%s\n", Clients[i].id, Clients[i].Name.c_str(), status.c_str());
					attroff(COLOR_PAIR(2));
				}
				else {
					attron(COLOR_PAIR(4));
					status = "Waiting for waiter.";
					printw("  %2d: %s\t\t%s\n", Clients[i].id, Clients[i].Name.c_str(), status.c_str());
					attroff(COLOR_PAIR(4));
				}
				iter3++;
			}
			else {
				if (!n3 && iter3 == 0) {
					printw("There is nothing.\n");
					n3 = true;
				}
				else {
					continue;
				}

			}
		}
		attron(COLOR_PAIR(1));
		printw("Table 4:\n");
		attroff(COLOR_PAIR(1));
		for (int i = 0; i < (int)Clients.size(); i++) {
			if (Clients[i].table_id == 4) {
				if (Clients[i].status == 2) {
					attron(COLOR_PAIR(6));
					status = "Orders: " + Clients[i].order_c;
					printw("  %2d: %s\t\t%s\n", Clients[i].id, Clients[i].Name.c_str(), status.c_str());
					attroff(COLOR_PAIR(6));
				}
				else if (Clients[i].status == 3) {
					attron(COLOR_PAIR(2));
					status = "Eats:  " + Clients[i].order_c;
					printw("  %2d: %s\t\t%s\n", Clients[i].id, Clients[i].Name.c_str(), status.c_str());
					attroff(COLOR_PAIR(2));
				}
				else {
					attron(COLOR_PAIR(4));
					status = "Waiting for waiter.";
					printw("  %2d: %s\t\t%s\n", Clients[i].id, Clients[i].Name.c_str(), status.c_str());
					attroff(COLOR_PAIR(4));
				}
				iter4++;
			}
			else {
				if (!n4 && iter4 == 0) {
					printw("There is nothing.\n");
					n4 = true;
				}
				else {
					continue;
				}

			}
		}
	locker.unlock();
}

void displayWaiters() {
	unique_lock<mutex> locker(mxWaiters);
	string status;
	attron(COLOR_PAIR(1));
	printw("\n\nWaiters:\n");
	attroff(COLOR_PAIR(1));
	if (!Waiters.empty()) {
		for (int i = 0; i < (int)Waiters.size(); i++)
		{
			if (Waiters[i].status == 1) {
				attron(COLOR_PAIR(4));
				status = "waiting for client ";
				printw("  %2d: %s\t\t%s\n", Waiters[i].id, Waiters[i].Name.c_str(), status.c_str());
				attroff(COLOR_PAIR(4));
			}
			else if (Waiters[i].status == 2) {
				attron(COLOR_PAIR(3));
				string s1 = "Order number " + to_string((int)Waiters[i].order_id);
				string s2 = " dish: " + (string)Waiters[i].Order_s;
				status = s1 + s2;
				printw("  %2d: %s\t\t%s\n", Waiters[i].id, Waiters[i].Name.c_str(), status.c_str());
				attroff(COLOR_PAIR(4));
			}
			else {
				attron(COLOR_PAIR(4));
				status = "waiting for client ";
				printw("  %2d: %s\t\t%s\n", Waiters[i].id, Waiters[i].Name.c_str(), status.c_str());
				attroff(COLOR_PAIR(4));
			}
		}
	}
	else {
		printw("  There is nothing.\n");
	}
	printw("\n\n");
	locker.unlock();
}

void displayCooks() {
	unique_lock<mutex> locker(mxCooks);
	string status;
	attron(COLOR_PAIR(1));
	printw("\n\nCooks:\n");
	attroff(COLOR_PAIR(1));
	if (!Cooks.empty()) {
		for (int i = 0; i < (int)Cooks.size(); i++)
		{
			if (Cooks[i].status == 1) {
				attron(COLOR_PAIR(4));
				status = "waiting for order ";
				printw("  %2d: %s\t\t%s\n", Cooks[i].id, Cooks[i].Name.c_str(), status.c_str());
				attroff(COLOR_PAIR(4));
			}
			else if (Cooks[i].status == 2) {
				attron(COLOR_PAIR(3));
				string s1 = "Order number." + to_string((int)Cooks[i].order_id);
				string s2 = " dish: " + (string)Cooks[i].Order_s;
				status = s1 + s2;
				printw("  %2d: %s\t\t%s\n", Cooks[i].id, Cooks[i].Name.c_str(), status.c_str());
				attroff(COLOR_PAIR(3));
			}
			else {
				attron(COLOR_PAIR(4));
				status = "waiting for order ";
				printw("  %2d: %s\t\t%s\n", Cooks[i].id, Cooks[i].Name.c_str(), status.c_str());
				attroff(COLOR_PAIR(4));
			}
		}
	}
	else {
		printw("  There is nothing.\n");
	}
	printw("\n\n");
	locker.unlock();
}

void displayOrders() {
	unique_lock<mutex> locker(mxOrders);
	string status;
	attron(COLOR_PAIR(1));
	printw("Orders:\n");
	attroff(COLOR_PAIR(1));
	if (!Orders.empty()) {
		for (int i = 0; i < (int)Orders.size(); i++)
		{
			if (Orders[i].status == 1) {
				attron(COLOR_PAIR(3));
				status = " received";
				printw("  %2d: %-10s %-20s\n", Orders[i].id, Orders[i].order_n.c_str(), status.c_str());
				attroff(COLOR_PAIR(3));
			}
			else if(Orders[i].status == 2) {
				attron(COLOR_PAIR(1));
				status = " cooked";
				printw("  %2d: %-10s %-20s\n", Orders[i].id, Orders[i].order_n.c_str(), status.c_str());
				attroff(COLOR_PAIR(1));
			}
		}
	}
	else {
		printw("There is nothing.\n");
	}
	printw("\n\n");
	locker.unlock();
}

void Ncurses() {
	initscr();
	start_color();
	init_pair(1,COLOR_GREEN,COLOR_BLACK);
	init_pair(2,COLOR_MAGENTA,COLOR_BLACK);
	init_pair(3,COLOR_RED,COLOR_BLACK);
	init_pair(4,COLOR_YELLOW,COLOR_BLACK);
	init_pair(5,COLOR_CYAN,COLOR_BLACK);
	init_pair(6,COLOR_BLUE,COLOR_BLACK);

	do {
		clear();
		attron(COLOR_PAIR(5));
		printw("218380 Damian Kleska. Projekt restauracji.\n\n");
		attroff(COLOR_PAIR(5));
		displayQueue();
		displayTables();
		displayWaiters();
		displayCooks();
		displayOrders();
		refresh();
		this_thread::sleep_for(std::chrono::seconds(1));
	}while(!endX);
	clear();
	attron(COLOR_PAIR(5));
	printw("218380 Damian Kleska. Projekt restauracji.\n\n");
	mvprintw(5,25, "Zamnkiete");
	attroff(COLOR_PAIR(5));
	mvprintw(21,10,"Nacisnij dowony przycisk aby kontynuowac");
	getch();
	endwin();
}
