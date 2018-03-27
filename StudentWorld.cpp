#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <string>
#include <list>
#include <algorithm>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

StudentWorld::StudentWorld(string assetDir)
: GameWorld(assetDir)
{
}

StudentWorld::~StudentWorld()
{
	cleanUp();		// calls the cleanUp function to destruct and clean up all dynamically allocated objects
}

int StudentWorld::init()
{
	// initialize members and create NachenBlaster
	m_numAliensOnScreen = 0;
	m_alienShipsDestroyed = 0;
	m_alienShipsToBeDestroyed = 6 + (4 * getLevel());
	m_maxAliensOnScreen = 4 + (.5 * getLevel());
	m_nachenBlaster = new NachenBlaster(this);

	// create 30 stars 
	for (int i = 0; i < 30; i++)
	{
		double x = randInt(0, VIEW_WIDTH - 1);
		double y = randInt(0, VIEW_HEIGHT - 1);
		double size = randInt(5, 50) / 100.0;
		addActor(new Star(this, x, y, size));
	}

	// Display status text
	updateStatusText();

    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
	// have NachenBlaster (if alive) and all alive actors do something 
	m_nachenBlaster->doSomething();
	if (!m_nachenBlaster->isAlive())
	{
		// if Nachenblaster dies in this tick, decrement lives and return that the player died
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}

	for (list<Actor*>::iterator p = m_actors.begin(); p != m_actors.end(); p++)
	{
		if ((*p)->isAlive())
		{
			(*p)->doSomething();

			if (!m_nachenBlaster->isAlive())
			{
				// if Nachenblaster dies in this tick, decrement lives and return that the player died
				decLives();
				return GWSTATUS_PLAYER_DIED;
			}

			// check that current level is completed
			if (m_alienShipsDestroyed >= m_alienShipsToBeDestroyed)
			{
				playSound(SOUND_FINISHED_LEVEL);
				return GWSTATUS_FINISHED_LEVEL;
			}
		}
			
	}

	// 1/15 chance of introducing new Star
	if (randInt(1, 15) == 1)
	{
		addActor(new Star(this, VIEW_WIDTH - 1, randInt(0, VIEW_HEIGHT - 1), randInt(5, 50) / 100.0));
	}

	// if the number of aliens on screen are less than the minimum of the max # vs the remaining # of ships to destroy
	// add a new alien
	if (m_numAliensOnScreen < min(m_maxAliensOnScreen, m_alienShipsToBeDestroyed - m_alienShipsDestroyed))
	{
		int addAlien = decideShipToAdd();
		int yCoord = randInt(0, VIEW_HEIGHT - 1);
		switch (addAlien)
		{
		case ADD_SMALLGON:
			addActor(new Smallgon(this, VIEW_WIDTH - 1, yCoord));
			break;
		case ADD_SMOREGON:
			addActor(new Smoregon(this, VIEW_WIDTH - 1, yCoord));
			break;
		case ADD_SNAGGLEGON:
			addActor(new Snagglegon(this, VIEW_WIDTH - 1, yCoord));
			break;
		}
		m_numAliensOnScreen++;
	}

	// delete any dead actors
	deleteDeadActors();

	// update the status text at top
	updateStatusText();

	return GWSTATUS_CONTINUE_GAME;
 
}

void StudentWorld::cleanUp()
{
	// deletes dynamically allocated player
	delete m_nachenBlaster;
	m_nachenBlaster = nullptr;

	// deletes all dynamically allocated actors
	for (list<Actor*>::iterator p = m_actors.begin(); p != m_actors.end();)
	{
		delete *p;
		*p = nullptr;
		p = m_actors.erase(p);
	}
}

int StudentWorld::decideShipToAdd()
{
	int S1 = 60;
	int S2 = 20 + getLevel() * 5;
	int S3 = 5 + getLevel() * 10;
	int S = S1 + S2 + S3;

	int chance = randInt(1, S);

	// set boundaries for the probability
	int smallgonProb = S1;
	int smoregonProb = S1 + S2;
	int snagglegonProb = S;

	if (chance <= smallgonProb)
		return ADD_SMALLGON;
	else if (chance > smallgonProb && chance <= smoregonProb)
		return ADD_SMOREGON;
	else
		return ADD_SNAGGLEGON;
}

void StudentWorld::recordAlienDestroyed()
{
	m_alienShipsDestroyed++;
}

void StudentWorld::increasePlayerHP()
{
	m_nachenBlaster->increaseHitPoints(10);
}

void StudentWorld::increaseTorpedoes()
{
	m_nachenBlaster->increaseTorpedoes(5);
}

void StudentWorld::addActor(Actor* a)
{
	m_actors.push_back(a);
}

void StudentWorld::deleteDeadActors()
{
	for (list<Actor*>::iterator p = m_actors.begin(); p != m_actors.end();)
	{
		if ((*p)->isAlive() == false)
		{
			if ((*p)->isAlien())
				m_numAliensOnScreen--;
			delete *p;
			p = m_actors.erase(p);
		}
		else
			p++;
	}
}

bool StudentWorld::playerInLineOfFire(const Actor* a) const
{
	if (m_nachenBlaster->getX() < a->getX() && m_nachenBlaster->getY() <= a->getY() + 4 &&
		m_nachenBlaster->getY() >= a->getY() - 4)
		return true;
	return false;
}

double StudentWorld::euclidean_dist(double x1, double y1, double x2, double y2) const
{
	return sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
}

bool StudentWorld::isCollision(const Actor* a, const Actor* p) const
{
	if (euclidean_dist(a->getX(), a->getY(), p->getX(), p->getY()) < .75 * (a->getRadius() + p->getRadius()))
		return true;
	return false;
}

void StudentWorld::checkCollision(Actor* a)
{
	// if the passed in actor is a NachenBlaster, check for collisions with aliens or projectiles that Aliens fire
	if (a == m_nachenBlaster)
	{
		for (list<Actor*>::const_iterator p = m_actors.begin(); p != m_actors.end(); p++)
		{
			if ((*p)->isAlive() && isCollision(a, *p))
			{
				if ((*p)->isAlien())
				{
					Alien* al = static_cast<Alien*>(*p);
					al->sufferDamage(0, HIT_BY_SHIP);
					m_nachenBlaster->sufferDamage(al->getDamageAmt(), HIT_BY_SHIP);
				}
				if ((*p)->isProjectile())
				{
					Projectile* pr = static_cast<Projectile*>(*p);
					if (pr->firedFromAlien())
					{
						m_nachenBlaster->sufferDamage(pr->getDmgAmt(), HIT_BY_PROJECTILE);
						pr->setDead();
					}
				}
			}
		}
	}

	// if a is an Alien, check for collisions with the NachenBlaster or projectiles that the NB fires
	if (a->isAlien())
	{
		Alien* al = static_cast<Alien*>(a);
		if (isCollision(al, m_nachenBlaster))
		{
			m_nachenBlaster->sufferDamage(al->getDamageAmt(), HIT_BY_SHIP);
			al->sufferDamage(0, HIT_BY_SHIP);
			
		}

		for (list<Actor*>::const_iterator p = m_actors.begin(); p != m_actors.end(); p++)
		{
			if ((*p)->isProjectile() && isCollision(al, *p) && (*p)->isAlive())
			{
				Projectile* pr = static_cast<Projectile*>(*p);
				if (pr->firedFromNB())
				{
					al->sufferDamage(pr->getDmgAmt(), HIT_BY_PROJECTILE);
					pr->setDead();
				}
			}
		}
	}

	// if a is a projectile, check for collision with aliens and the NachenBlaster
	if (a->isProjectile())
	{
		Projectile* pr = static_cast<Projectile*>(a);
		if (isCollision(pr, m_nachenBlaster) && pr->firedFromAlien())
		{
			m_nachenBlaster->sufferDamage(pr->getDmgAmt(), HIT_BY_PROJECTILE);
			pr->setDead();
		}
		for (list<Actor*>::const_iterator p = m_actors.begin(); p != m_actors.end(); p++)
		{
			if ((*p)->isAlien() && (*p)->isAlive() && isCollision(pr, *p))
			{
				if (pr->firedFromNB())
				{
					Alien* al = static_cast<Alien*>(*p);
					al->sufferDamage(pr->getDmgAmt(), HIT_BY_PROJECTILE);
					pr->setDead();
				}
			}
		}

	}

	// if a is a goodie, only check for collisions with the NachenBlaster
	if (a->isGoodie())
	{
		if (isCollision(a, m_nachenBlaster))
		{
			Goodie* gd = static_cast<Goodie*>(a);
			increaseScore(100);
			gd->giveGoodie();
			gd->setDead();
		}
	}
}

void StudentWorld::updateStatusText()
{
	// initialize game stats text string and set it
	//m_gameStatText << "Lives: 3  Health: 100%  Score: 24530  Level: 3  Cabbages: 80%  Torpedoes: 4";
	ostringstream oss;
	oss.setf(ios::fixed);
	oss.precision(0);
	oss << "Lives: " << getLives();
	oss << setw(10) << "Health: " << m_nachenBlaster->getHealthPercent() << "%  Score: " << getScore();
	oss << setw(9) << "Level: " << getLevel();
	oss << setw(11) << "Cabbages: " << m_nachenBlaster->getCabbagePercent() << "%";
	oss << setw(13) << "Torpedoes: " << m_nachenBlaster->getNumTorpedoes();
	setGameStatText(oss.str());
}