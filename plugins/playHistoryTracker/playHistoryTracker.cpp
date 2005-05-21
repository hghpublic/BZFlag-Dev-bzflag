// playHistoryTracker.cpp : Defines the entry point for the DLL application.
//


#include "bzfsAPI.h"
#include <string>
#include <map>

BZ_GET_PLUGIN_VERSION

// event handler callback

class PlayHistoryTracker : public bz_EventHandler
{
public:
	PlayHistoryTracker();
	virtual ~PlayHistoryTracker();

	virtual void process ( bz_EventData *eventData );

	virtual bool autoDelete ( void ) { return false;} // this will be used for more then one event
protected:

	typedef struct 
	{
		int playerID;
		std::string callsign;
		double		startTime;
		double		lastUpdateTime;
		int			spreeTotal;
	}trPlayerHistoryRecord;

	std::map<int, trPlayerHistoryRecord > playerList;
};

PlayHistoryTracker	historyTracker;

BZF_PLUGIN_CALL int bz_Load ( const char* commandLine )
{
	bz_debugMessage(4,"PlayHistoryTracker plugin loaded");

	bz_registerEvent(bz_ePlayerDieEvent,BZ_ALL_USERS,&historyTracker);
	bz_registerEvent(bz_ePlayerPartEvent,BZ_ALL_USERS,&historyTracker);
	bz_registerEvent(bz_ePlayerSpawnEvent,BZ_ALL_USERS,&historyTracker);
	bz_registerEvent(bz_ePlayerDieEvent,BZ_ALL_USERS,&historyTracker);

	return 0;
}

BZF_PLUGIN_CALL int bz_Unload ( void )
{
	bz_removeEvent(bz_ePlayerDieEvent,BZ_ALL_USERS,&historyTracker);
	bz_removeEvent(bz_ePlayerPartEvent,BZ_ALL_USERS,&historyTracker);
	bz_removeEvent(bz_ePlayerSpawnEvent,BZ_ALL_USERS,&historyTracker);
	bz_removeEvent(bz_ePlayerDieEvent,BZ_ALL_USERS,&historyTracker);

	bz_debugMessage(4,"PlayHistoryTracker plugin unloaded");
	return 0;
}

// ----------------- SpreeTracker-----------------

/*typedef struct 
{
int playerID;
std::string callsign;
double		startTime;
double		lastUpdateTime;
int			spreeTotal;
}trPlayerHistoryRecord;

std::map<int, trPlayerHistoryRecord > playerList; */

PlayHistoryTracker::PlayHistoryTracker()
{
}

PlayHistoryTracker::~PlayHistoryTracker()
{
}

void PlayHistoryTracker::process ( bz_EventData *eventData )
{
	switch (eventData->eventType)
	{
	default:
		// really WTF!!!!
		break;

	case bz_ePlayerDieEvent:
		{
			 bz_PlayerDieEventData	*deathRecord = ( bz_PlayerDieEventData*)eventData;

			 bz_PlayerRecord	killerData;

			 bz_getPlayerByIndex(deathRecord->killerID,&killerData);

		//	GameKeeper::Player *killerData = GameKeeper::Player::getPlayerByIndex(deathRecord->killerID);
			//GameKeeper::Player *victimData = GameKeeper::Player::getPlayerByIndex(deathRecord->playerID);

			// clear out the dude who got shot, since he won't be having any SPREEs
			if (playerList.find(deathRecord->playerID) != playerList.end())
			{
				trPlayerHistoryRecord	&record = playerList.find(deathRecord->playerID)->second;
				std::string message;
				if ( record.spreeTotal >= 5 && record.spreeTotal < 10 )
					message = record.callsign + std::string("'s rampage was stoped by ") + killerData.callsign;
				if ( record.spreeTotal >= 10 && record.spreeTotal < 20 )
					message = record.callsign + std::string("'s killing spree was halted by ") + killerData.callsign;
				if ( record.spreeTotal >= 20 )
					message = std::string("The unstopable reign of ") + record.callsign + std::string(" was ended by ") + killerData.callsign;

				if (message.size())
					 bz_sendTextMessage(BZ_SERVER, BZ_ALL_USERS, message.c_str());

				record.spreeTotal = 0;
				record.startTime = deathRecord->time;
				record.lastUpdateTime = deathRecord->time;

			}

			// chock up another win for our killer
			// if they weren't the same as the killer ( suicide ).
			if ( (deathRecord->playerID != deathRecord->killerID) && playerList.find(deathRecord->killerID) != playerList.end())
			{
				trPlayerHistoryRecord	&record = playerList.find(deathRecord->killerID)->second;
				record.spreeTotal++;
				record.lastUpdateTime = deathRecord->time;

				std::string message;

				if ( record.spreeTotal == 5 )
					message = record.callsign + std::string(" is on a Rampage!");
				if ( record.spreeTotal == 10 )
					message = record.callsign + std::string(" is on a Killing Spree!");
				if ( record.spreeTotal == 20 )
					message = record.callsign + std::string(" is Unstoppable!!");
				if ( record.spreeTotal > 20 && record.spreeTotal%5 == 0 )
					message = record.callsign + std::string("'s continues rage");

				if (message.size())
					bz_sendTextMessage(BZ_SERVER, BZ_ALL_USERS, message.c_str());
			}
		}
		break;

	case  bz_ePlayerSpawnEvent:
		// really WTF!!!!
		break;

	case  bz_ePlayerJoinEvent:
		{
			trPlayerHistoryRecord	playerRecord;

			playerRecord.playerID = (( bz_PlayerJoinPartEventData*)eventData)->playerID;
			playerRecord.callsign = (( bz_PlayerJoinPartEventData*)eventData)->callsign;
			playerRecord.spreeTotal = 0;
			playerRecord.lastUpdateTime = (( bz_PlayerJoinPartEventData*)eventData)->time;
			playerRecord.startTime = playerRecord.lastUpdateTime;

			playerList[(( bz_PlayerJoinPartEventData*)eventData)->playerID] = playerRecord;
		}
		break;
	case  bz_ePlayerPartEvent:
		{
			std::map<int, trPlayerHistoryRecord >::iterator	itr = playerList.find( (( bz_PlayerJoinPartEventData*)eventData)->playerID );
			if (itr != playerList.end())
				playerList.erase(itr);
		}
		break;
	}
}
