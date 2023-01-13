#define WINVER 0x0500
#include <iostream>
#include <Windows.h>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;



void pressKey(INPUT input, int keyCode)
{
	// Press the key
	input.ki.wVk = keyCode; // takes in the specified key
	input.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &input, sizeof(INPUT));
}

void releaseKey(INPUT input, int keyCode)
{
	// Release the key
	input.ki.wVk = keyCode; // takes in the specified key
	input.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &input, sizeof(INPUT));
}

string readKey(bool previousKeys[])
{
	// Concatenation string to be returned
	string returnString = " ";

	// Loops through all key codes to read what key is pressed
	for (int i = 1; i < 255; i++)
	{
		// Released keys
		if (!GetAsyncKeyState(i) && previousKeys[i-1]) {
			returnString += "U" + to_string(i) + " ";
			previousKeys[i-1] = false;
		}
		// Pressed keys
		else if (GetAsyncKeyState(i) && !previousKeys[i-1]) {
			returnString += "D" + to_string(i) + " ";
			previousKeys[i-1] = true;
		}
	}
	// Returns string of all keys or buttons pressed
	return returnString;
}

void writeKey(INPUT ip, string element)
{
	// Checking if left or right mouse buttons to be pressed
	if (element == "D1") mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	else if (element == "U1") mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

	if (element == "D2") mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
	else if (element == "U2") mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
	
	// Checking for all other keys
	char sign = element[0];
	element[0] = ' ';
	if (sign == 'D') pressKey(ip, stoi(element));
	else if (sign == 'U') releaseKey(ip, stoi(element));
}

string resetKeys(INPUT ip)
{
	// Reads in all keys that are currently pressed
	bool previousKeys[254];
	for (int i = 0; i < 254; i++) previousKeys[i] = false;
	string pressedKeys = readKey(previousKeys);
	stringstream ss(pressedKeys);
	string element;

	// Loops through all pressed keys and releases them
	while (getline(ss, element, ' '))
	{
		if (element.length() > 1)
		{
			element[0] = 'U';
			writeKey(ip, element);
		}
	}

	// Returns previously pressed keys for continuing loop
	return pressedKeys;
}

string readCursorPos()
{
	// Locates cursor position
	POINT mouse;
	GetCursorPos(&mouse);
	
	// Returns cursor position as a string
	return to_string(mouse.x) + " " + to_string(mouse.y);
}



void buildLoop(string fileName)
{
	// Creates file where loop is stored
	ofstream file(fileName);

	// Array to store previous keys inputted
	bool previousKeys[254];
	for (int i = 0; i < 254; i++) previousKeys[i] = false;

	// Storing loop
	cout << "Building loop, press middle cursor button to stop.\n";
	while (!GetAsyncKeyState(VK_MBUTTON))
	{
		// Reads in user commands at regular build speed
		file << readCursorPos() + readKey(previousKeys) << endl;
		Sleep(20);
	}

	// Close file when finished making loop
	cout << "Loop stored in " + fileName + ".\n";
	file.close();
	Sleep(1000);
}


void playLoop(string fileName, int playbackSpeed)
{
	// This will be used to setup the keyboard input event
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// Variable that determines if loop should continue 
	bool runLoop = true;

	// Execute loop
	cout << "Executing loop, press middle cursor button to pause.\n";
	while (runLoop)
	{
		// Open file for reading and strings for parsing
		ifstream file(fileName);
		string element;

		// File loop
		while (getline(file, element, '\n') && runLoop)
		{
			// For string manipulation
			stringstream ss(element);

			// Setting cursor position first off
			string xPos;
			string yPos;
			getline(ss, xPos, ' ');
			getline(ss, yPos, ' ');
			SetCursorPos(stoi(xPos), stoi(yPos));

			// Checks if user wants to pause the loop
			if (GetAsyncKeyState(VK_MBUTTON))
			{
				// Pause menu
				releaseKey(ip, 4);
				string pressedKeys = resetKeys(ip);
				cout << "\nLoop has been paused. What would you like to do?\n1. Continue the loop\n2. Exit the loop\nPlease enter either 1 or 2: ";
				string option;
				cin >> option;

				// Option validity checking
				while (option != "1" && option != "2") {
					cout << "Please try again: ";
					cin >> option;
				}

				// Continues or ends the loop based on user input
				if (option == "1") {
					cout << "Loop execution will continue in 5 seconds.\n";
					Sleep(5000);
					cout << "Executing loop, press middle cursor button to pause.\n";

					// Loops through and re-presses previous keys
					stringstream ss2(pressedKeys);
					SetCursorPos(stoi(xPos), stoi(yPos));
					while (getline(ss2, element, ' ')) writeKey(ip, element);
				}
				else runLoop = false;
			}

			// Loops through and runs remaining string command
			while (getline(ss, element, ' ')) writeKey(ip, element);

			// Controls speed of the loop
			Sleep(playbackSpeed);
		}

		// Close up file for reset of loop
		file.close();
	}

	cout << "Exiting loop...\n";
	resetKeys(ip);
	Sleep(1000);
}



int main()
{
	cout << "-- Welcome to LoopBot --\n";
	
	while (true)
	{
		cout << "\nWhat would you like to do? \n1. Build a loop\n2. Run a loop\nPlease enter either 1 or 2: ";
		string option;
		cin >> option;

		while (option != "1" && option != "2") {
			cout << "Please try again: ";
			cin >> option;
		}

		cout << "\nPlease enter the name of the loop file you would like to build / run (exclude file types): ";

		string loopFileName;
		cin >> loopFileName;
		ifstream f(loopFileName + ".txt");

		if (option == "1") {
			cout << "Loop building will commence in 5 seconds.\n";
			Sleep(5000);
			buildLoop(loopFileName + ".txt");
		}
		else
		{
			cout << "\nPlease choose a loop playback speed:\n1. 0.5x speed\n2. 1x speed\n3. 2x speed\nPlease enter a number from 1 to 3: ";
			cin >> option;

			while (option != "1" && option != "2" && option != "3") {
				cout << "Please try again: ";
				cin >> option;
			}

			if (option == "3") option = "4";
			cout << "Loop execution will commence in 5 seconds.\n";
			Sleep(5000);
			playLoop(loopFileName + ".txt", 40/stoi(option));
		}

		cout << "\nWould you like to run the program again? (Y/N): ";
		cin >> option;

		while (option != "y" && option != "Y" && option != "n" && option != "N") {
			cout << "Please try again: ";
			cin >> option;
		}

		if (option == "n" || option == "N") break;
	}
	
	return 0;
}