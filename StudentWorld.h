#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GraphObject.h"
#include "Actor.h"
#include <string>
#include <sstream>  // defines the type std::ostringstream
#include <iomanip>  // defines the manipulator setw
#include <list>
using namespace std;

class Actor;
class NachenBlaster;
class Alien;

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir);
	~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();

	void recordAlienDestroyed();							// record that one more alien on current level has been destroyed
	void addActor(Actor* a);								// add an actor to the world
	void checkCollision(Actor* a);							// given Actor a, method will check to see if any actors in StudentWorld have collided with a
	bool playerInLineOfFire(const Actor* a) const;			// Is the player in the line of fire of a, which might cause a to attack?
	void increasePlayerHP();								// tells the Nachenblaster to increase hit points by 10
	void increaseTorpedoes();								// tells NB to increase Torpedoes by 5

private:
	// private methods used only by StudentWorld
	void updateStatusText();		// updates the status text at top of screen
	int decideShipToAdd();			// function to decide which ship to add
	void deleteDeadActors();		// delete dead actors
	double euclidean_dist(double x1, double y1, double x2, double y2) const;		// gives Euclidean distance between 2 objects
	bool isCollision(const Actor* a, const Actor* p) const;								// checks if Actor a and Actor p have collided

	// private member variables
	list<Actor*> m_actors;			// holds all live actors in current world
	NachenBlaster* m_nachenBlaster;			// pointer to the NachenBlaster object

	int m_alienShipsDestroyed;		// int to hold the number of ships that have been destroyed
	int m_alienShipsToBeDestroyed;	// total number of ships that need to be destroyed to advance
	int m_maxAliensOnScreen;			// number of max aliens that can be on screen at one time
	int m_numAliensOnScreen;			// number of aliens on screen at current time
};

#endif // STUDENTWORLD_H_
