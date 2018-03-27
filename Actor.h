#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include <iostream>
using namespace std;
class StudentWorld;

using Direction = int;

// GLOBAL CONSTANTS
const int DOWN_LEFT = 1;
const int UP_LEFT = 2;
const int DUE_LEFT = 3;

const int HIT_BY_SHIP = 0;
const int HIT_BY_PROJECTILE = 1;

const int ADD_SMALLGON = 1;
const int ADD_SMOREGON = 2;
const int ADD_SNAGGLEGON = 3;

const int FIRED_FROM_NB = 1;
const int FIRED_FROM_SNAGGLEGON = 2;

class Actor : public GraphObject
{
public:
	// Constructor
	Actor(StudentWorld* ptrToWorld, double startX, double startY, int imageID,
		Direction dir, double size, unsigned int depth);
	// Destructor (virtual)
	virtual ~Actor() {
	}

	// pure virtual, since a general actor should not be able to "do something"
	virtual void doSomething() = 0;

	// Accessors
	bool isAlive() const;	// returns true if actor is alive
	bool isInBounds(double x, double y) const;	// checks to see if the (x,y) coordinates are within the game boundaries
	StudentWorld* getWorld() const;		// returns a pointer to the world the actor is in
	virtual bool isAlien() const;		// is actor an alien?
	virtual bool isProjectile() const;	// is actor a projectile?
	virtual bool isGoodie() const;		// is actor a goodie?

	// Mutators
	void setDead();
	virtual void moveTo(double x, double y);		// moves actor to x, y if on screen, else does not move and marks as dead
	void resetBounds(double& x, double &y);

private:
	bool m_alive;
	StudentWorld* m_world;
};

class Star : public Actor
{
public:
	// Constructor
	Star(StudentWorld* ptrToWorld, double startX, double startY, double size, Direction dir = 0, unsigned int depth = 3, int imageID = IID_STAR);
	// Destructor
	virtual ~Star() {
	}
	//Mutators
	virtual void doSomething();
};

class Explosion : public Actor
{
public:
	Explosion(StudentWorld* ptrToWorld, double startX, double startY);
	virtual void doSomething();
private:
	int m_count;			// will keep track of how many ticks the star has been alive in
};

class DamageableObject : public Actor
{
public:
	// Constructor
	DamageableObject(StudentWorld* ptrToWorld, double startX, double startY, int imageID,
		int startDir, double size, int depth, double hitPoints);

	// How many hit points does this actor have left?
	double hitPoints() const;

	// Increase this actor's hit points by amt.
	void increaseHitPoints(double amt);
	void decreaseHitPoints(double amt);

	// This actor suffers an amount of damage caused by being hit by either
	// a ship or a projectile (see constants above).
	virtual void sufferDamage(double amt, int cause) = 0;

private:
	double m_hitPoints;
};

class NachenBlaster : public DamageableObject
{
public:
	// Constructor
	NachenBlaster(StudentWorld* ptrToWorld);

	// Accessors
	double getHealthPercent() const;
	double getCabbagePercent() const;
	int getNumTorpedoes() const;

	// Mutators
	virtual void doSomething();
	void increaseTorpedoes(int torpedoes);
	virtual void sufferDamage(double amt, int cause);

private:
	void move(const int dir, double x, double y);
	int m_cabbageEnergyPoints;
	int m_numTorpedoes;
};

class Alien : public DamageableObject
{
public:
	// Constructor
	Alien(StudentWorld* ptrToWorld, double startX, double startY, int imageID,
		double hitPoints, double damageAmt, double deltaX,
		double deltaY, double speed, unsigned int scoreValue);

	virtual void doSomething();
	virtual bool isAlien() const;
	virtual void sufferDamage(double amt, int cause);

	// Accessors
	double getDamageAmt() const;
	int getScoreValue() const;

	// Mutators
	void setFlightPlan(int flightPlan);
	void setTravelDirection(int travelDirection);
	void setTravelSpeed(int travelSpeed);
	
	virtual bool possiblyShoot() = 0;		// tells alien to shoot
	void fireTurnip() const;				
	void fireTorpedo() const;
	virtual void possiblyDropGoodie() const = 0;				// if alien drops goodies, drop one with appropriate probability

private:
	// private methods
	bool isYInBounds(double y) const;		// checks if Y coord is in boundaries
	void resetYBound(double& y);
	void killAlien();			// takes care of the things done when an Alien is killed
	int getFlightPlan() const;
	void setDeltaX(double dx);
	void setDeltaY(double dy);
	void move();		// moves player by current speed in direction indicated by x and y deltas
	
	// private member variables
	int m_flightPlan;			// only Smallgons and Smoregons use this! for Snagglegons, it will be set to -1
	double m_deltaX;
	double m_deltaY;
	double m_damageAmt;
	double m_travelSpeed;
	int m_scoreValue;
};

class Smallgon : public Alien
{
public:
	// Constructor
	Smallgon(StudentWorld* ptrToWorld, double startX, double startY);
	virtual bool possiblyShoot();
	virtual void possiblyDropGoodie() const;
};

class Smoregon : public Alien
{
public:
	// Constructor
	Smoregon(StudentWorld* ptrToWorld, double startX, double startY);
	virtual void possiblyDropGoodie() const;
	virtual bool possiblyShoot();
private:
	void rammingMode();
};

class Snagglegon : public Alien
{
public:
	Snagglegon(StudentWorld* ptrToWorld, double startX, double startY);
	virtual void possiblyDropGoodie() const;
	virtual bool possiblyShoot();
};

class Projectile : public Actor
{
public:
	Projectile(StudentWorld* ptrToWorld, double startX, double startY, int imageID,
		double damageAmt, double deltaX, bool rotates, int imageDir);
	virtual void doSomething();
	virtual bool isProjectile() const;
	virtual bool firedFromNB() const;
	virtual bool firedFromAlien() const;

	// Accessor
	
	double getDmgAmt() const;

private:
	virtual bool isCabbage() const;
	virtual bool isTurnip() const;
	virtual bool isTorpedo() const;
	double getDeltaX() const;
	bool m_rotates;
	double m_damageAmt;
	double m_deltaX;
};

class Cabbage : public Projectile
{
public:
	Cabbage(StudentWorld* ptrToWorld, double startX, double startY);
	virtual bool isCabbage() const;
};

class Turnip : public Projectile
{
public: 
	Turnip(StudentWorld* ptrToWorld, double startX, double startY);
	virtual bool isTurnip() const;
};

class Torpedo : public Projectile
{
public:
	Torpedo(StudentWorld* ptrToWorld, double startX, double startY, int imageDir);
	virtual bool isTorpedo() const;
	virtual bool firedFromNB() const;
	virtual bool firedFromAlien() const;
private:
	int torpedoFiredFrom() const;
};

class Goodie : public Actor
{
public:
	Goodie(StudentWorld* ptrToWorld, double startX, double startY, int imageID);
	virtual void doSomething();
	virtual void giveGoodie() = 0;
	virtual bool isGoodie() const;
};

class ExtraLifeGoodie : public Goodie
{
public:
	ExtraLifeGoodie(StudentWorld* ptrToWorld, double startX, double startY);
	virtual void giveGoodie();
};

class RepairGoodie : public Goodie
{
public:
	RepairGoodie(StudentWorld* ptrToWorld, double startX, double startY);
	virtual void giveGoodie();
};

class TorpedoGoodie : public Goodie
{
public:
	TorpedoGoodie(StudentWorld* ptrToWorld, double startX, double startY);
	virtual void giveGoodie();
};

#endif // ACTOR_H_
