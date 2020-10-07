/******************************************************************************/
/*!
@file   Sound.cpp
@author Natalia Pollack
@par    Course: GAM200
@par    Copyright 2020 Digipen Institute of Technology
@date   09/02/2020
@brief
	This is a stub implementation file

*/
/**************************************************************************/

#include "fmod.h"
#include "Sound.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "Trace.h"
#include "fmod_studio.hpp"
#include "fmod.hpp"

/* the fmod result to check for errors */
FMOD_RESULT result;    

// struct outline
struct BankData
{
	// the data for bank
	FMOD::Studio::Bank* bank;

	// the name of bank
	std::string nameofBank;

};

// struct for the events
struct EventData
{
	// name of the event
	std::string eventName;

	// description of the event
	FMOD::Studio::EventDescription* eventDescription;

	// holding the event instance
	FMOD::Studio::EventInstance* eventInstance;
};

// vector holding bank struct so i can add to it 
std::vector<BankData> Banks;

// vector of event names to keep track of it all
std::vector<EventData> Events;

// making the system namee
FMOD::Studio::System* AudioSystem = NULL;

// lowlevel is no longer there
FMOD::System* coreSystem = NULL;

// this function error checks all the fmod things
void ErrorCheck(FMOD_RESULT test, const char* message)
{
	// if its not okay, print error
	if (test != FMOD_OK)
		Trace::Log("Audio", "Error in: %s", message);
}

// the function to create the sound system, call once in beginning.
void Audio::Init()
{
	// creating the audio studio system
	result = FMOD::Studio::System::create(&AudioSystem);
	ErrorCheck(result, "System Creation");

	// setting things up
	result = AudioSystem->getCoreSystem(&coreSystem);
	ErrorCheck(result, "Getting Core System");
	result = coreSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_STEREO, 0);
	ErrorCheck(result, "Setting SoftwareFormat");

	// initilize the system
	result = AudioSystem->initialize(32, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL, 0);
	ErrorCheck(result, "Initialization");

	// helper function shown later to read in the file
	ReadGUID();
	// helper function shown later to load in the data from banks
	LoadSampleData();
	// helpger function shown later to create event descriptions / instances
	SettingUpEvents();

}


// function to read in the file and create the banks
void Audio::ReadGUID()
{
	//openening the file
	std::ifstream file("FMOD/Build/GUIDs.txt");

	//if file opens successfully
	if (file.is_open())
	{
		// creating a string to hold the line
		std::string Line;
		// while there's lines 2 read
		while (std::getline(file, Line))
		{
			// if the word bank is there make a bank
			if (Line.find("bank") != std::string::npos)
			{
				// creating a temp bank using the struct to store it
				BankData b;
				// finding the name of the bank
				b.nameofBank = Line.substr(Line.find("/") + 1);
				// storing name of the path to the bank by making char
				char PathName[256];
			    // puts the full path name in
				sprintf_s(PathName, 256, "FMOD/Build/Desktop/%s.bank", b.nameofBank.c_str());
				// loading in the bank 
				result = AudioSystem->loadBankFile(PathName, FMOD_STUDIO_LOAD_BANK_NORMAL, &b.bank);
				//error checking
				ErrorCheck(result, "Loading in Banks");
				// adding that to the file
				Banks.push_back(b);
			}

			// if event is listed, add it to the list of eventnames
			if (Line.find("event") != std::string::npos)
			{
				// making the event data holder
				EventData e;
				// setting name of event
				e.eventName = Line.substr(Line.find("event"));
				// putting it in array
				Events.push_back(e);
			}
		}
	}
}

FMOD::Studio::Bank* FindBank(std::string BankName)
{
	// loop through the banks vector array to find the matching name 
	for (unsigned int i = 0; i < Banks.size(); ++i)
	{
		// if we got it, return it
		if (Banks[i].nameofBank == BankName)
			return Banks[i].bank;
	}
	// retrn nothing if not found
	return NULL;
}

// load in the data from the banks
void Audio::LoadSampleData()
{
	// only need the sample data of the non-streaming sounds, so everything bUT music
	FMOD::Studio::Bank* MasterBank = FindBank("Master");
	result = MasterBank->loadSampleData();
	ErrorCheck(result, "Loading Sample Data 4 MasterBank");
	FMOD::Studio::Bank* SFXBank = FindBank("SFX");
	result = SFXBank->loadSampleData();
	ErrorCheck(result, "Loading Sample Data 4 SFXBank");
	FMOD::Studio::Bank* MasterStrings = FindBank("Master.strings");
	result = MasterStrings->loadSampleData();
	ErrorCheck(result, "Loading Sample Data 4 MasterStringsBank");
}

// set up the events from the banks
void Audio::SettingUpEvents()
{
	// for every event
	for (unsigned int i = 0; i < Events.size(); ++i)
	{
		// make the event description and store it
		result = AudioSystem->getEvent(Events[i].eventName.c_str(), &Events[i].eventDescription);
		ErrorCheck(result, "Setting up Event Descriptions");
		// make the event instance and store it
		result = Events[i].eventDescription->createInstance(&Events[i].eventInstance);
		ErrorCheck(result, "Setting up Event Instances");
	}
}

// helper function to find certain events based on their name
FMOD::Studio::EventInstance* FindEvent(const char* EventName)
{
	// loop through the events vector array to find the matching name 
	for (unsigned int i = 0; i < Events.size(); ++i)
	{
		// if we got that part of the event name, then return the event instnace
		if (Events[i].eventName.find(EventName) != std::string::npos)
			return Events[i].eventInstance;
	}
	// retrn nothing if not found
	return NULL;
}

// function to play the event based on its name
void Audio::PlayEvent(const char* EventName)
{
	FindEvent(EventName)->start();
}

// function to stop the event based on its name
void Audio::StopEvent(const char* EventName)
{
	FindEvent(EventName)->stop(FMOD_STUDIO_STOP_IMMEDIATE);
}

// the function to update all audio, call in every update gamestate
void Audio::Update()
{
	result = AudioSystem->update();
	ErrorCheck(result, "Update");
}


// unloads all the banks that were created
void UnloadBanks()
{
	for (unsigned int i = 0; i < Banks.size(); ++i)
	{
		Banks[i].bank->unload();
	}
}

// the function to close and shut down the audio system, call once in the end.
void Audio::ShutDown()
{
	AudioSystem->release();
	UnloadBanks();
}