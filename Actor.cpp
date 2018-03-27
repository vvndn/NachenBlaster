#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include <cmath>

// Students:  Add code to this file, Actor.h, StudentWorld.h, and StudentWorld.cpp
Actor::Actor(StudentWorld* ptrToWorld, double startX, double startY, int imageID, Direction dir, double size, unsigned int depth)
	: GraphObject(imageID, startX, startY, dir, size, depth), m_alive(true), m_world(ptrToWorld)
{
}

bool Actor::isAlive() const
{
	return m_alive;
}

bool Actor::isAlien() const
{
	return false;
}

bool Actor::isProjectile() const
{
	return false;
}

bool Actor::isGoodie() const
{
	return false;
}

bool Actor::isInBounds(double x, double y) const
{
	if (x < 0 || x >= VIEW_WIDTH || y < 0 || y >= VIEW_HEIGHT)
		return false;
	return true;
}

void Actor::setDead()
{
	m_alive = false;
}

void Actor::moveTo(double x, double y)
{
	if (isInBounds(x, y))
		GraphObject::moveTo(x, y);
	else
		setDead();
}

void Actor::resetBounds(double& x, double& y)
{
	if (x < 0)
		x = 0;
	else if (x >= VIEW_WIDTH)
		x = VIEW_WIDTH - 1;
	else if (y < 0)
		y = 0;
	else if (y >= VIEW_HEIGHT)
		y = VIEW_HEIGHT - 1;
}

StudentWorld* Actor::getWorld() const
{
	return m_world;
}

Star::Star(StudentWorld* ptrToWorld, double startX, double startY, double size, Direction dir, unsigned int depth, int imageID)
	: Actor(ptrToWorld, startX, startY, IID_STAR, 0, randInt(5, 50) / 100.0, 3)
{
}

void Star::doSomething()
{
	moveTo(getX() - 1, getY());
}

Explosion::Explosion(StudentWorld* ptrToWorld, double startX, double startY)
	: Actor(ptrToWorld, startX, startY, IID_EXPLOSION, 0, 1, 0), m_count(0)
{
}

void Explosion::doSomething()
{
	if (m_count < 3)
	{
		setSize(1.5*getSize());
		m_count++;
	}
	else
		setDead();
}

DamageableObject::DamageableObject(StudentWorld* ptrToWorld, double startX, double startY, int imageID,
	int startDir, double size, int depth, double hitPoints)
	: Actor(ptrToWorld, startX, startY, imageID, startDir, size, depth), m_hitPoints(hitPoints)
{
}

double DamageableObject::hitPoints() const
{
	return m_hitPoints;
}

void DamageableObject::increaseHitPoints(double amt)
{
	if (m_hitPoints < 50)
		m_hitPoints += amt;
}

void DamageableObject::decreaseHitPoints(double amt)
{
	m_hitPoints -= amt;
}

NachenBlaster::NachenBlaster(StudentWorld* ptrToWorld)
	: DamageableObject(ptrToWorld, 0, 128, IID_NACHENBLASTER, 0, 1.0, 0, 50), m_cabbageEnergyPoints(30), m_numTorpedoes(0)
{
	
}

void NachenBlaster::move(const int dir, double x, double y)
{
	double moveX = x;
	double moveY = y;
	switch (dir)
	{
	case KEY_PRESS_LEFT:
		moveX -= 6;
		break;
	case KEY_PRESS_RIGHT:
		moveX += 6;
		break;
	case KEY_PRESS_DOWN:
		moveY -= 6;
		break;
	case KEY_PRESS_UP:
		moveY += 6;
		break;
	}
	if (!isInBounds(moveX, moveY))
	{
		resetBounds(moveX, moveY);
	}
	moveTo(moveX, moveY);
	return;
}

void NachenBlaster::doSomething()
{
	if (isAlive() == false)
		return;

	int ch;
	if (getWorld()->getKey(ch))
	{
		if (ch == KEY_PRESS_SPACE && m_cabbageEnergyPoints >= 5)
		{
			getWorld()->addActor(new Cabbage(getWorld(), getX() + 12, getY()));
			m_cabbageEnergyPoints -= 5;
			getWorld()->playSound(SOUND_PLAYER_SHOOT);
		}
			
		if (ch == KEY_PRESS_TAB && m_numTorpedoes > 0)
		{
			getWorld()->addActor(new Torpedo(getWorld(), getX() + 12, getY(), 0));
			m_numTorpedoes--;
			getWorld()->playSound(SOUND_TORPEDO);
		}

		else
		{
			double x = getX();
			double y = getY();
			move(ch, x, y);
			getWorld()->checkCollision(this);
			if (!isAlive())						// check if alive, if not, return
				return;
		}
	}

	if (m_cabbageEnergyPoints < 30)
		m_cabbageEnergyPoints++;
}

double NachenBlaster::getHealthPercent() const
{
	return ((hitPoints() / 50.0) * 100.0);
}

double NachenBlaster::getCabbagePercent() const
{
	return ((m_cabbageEnergyPoints / 30.0) * 100.0);
}

int NachenBlaster::getNumTorpedoes() const
{
	return m_numTorpedoes;
}

void NachenBlaster::increaseTorpedoes(int torpedoes)
{
	m_numTorpedoes += torpedoes;
}

void NachenBlaster::sufferDamage(double amt, int cause)
{
	decreaseHitPoints(amt);
	if (hitPoints() <= 0)
		setDead();
}

Alien::Alien(StudentWorld* ptrToWorld, double startX, double startY, int imageID,
	double hitPoints, double damageAmt, double deltaX,
	double deltaY, double speed, unsigned int scoreValue)
	: DamageableObject(ptrToWorld, startX, startY, imageID, 0, 1.5, 1, hitPoints), m_deltaX(deltaX),
	m_deltaY(deltaY), m_damageAmt(damageAmt), m_travelSpeed(speed), m_scoreValue(scoreValue)
{
}

void Alien::sufferDamage(double amt, int cause)
{
	if (cause == HIT_BY_PROJECTILE)
	{
		decreaseHitPoints(amt);
		if (hitPoints() <= 0)
		{
			killAlien();
		}
		else
			getWorld()->playSound(SOUND_BLAST);
	}
	else if (cause == HIT_BY_SHIP)
	{
		killAlien();
	}
}

void Alien::killAlien()
{
	getWorld()->increaseScore(getScoreValue());
	setDead();
	getWorld()->recordAlienDestroyed();
	getWorld()->playSound(SOUND_DEATH);
	getWorld()->addActor(new Explosion(getWorld(), getX(), getY()));
	possiblyDropGoodie();
}

bool Alien::isAlien() const
{
	return true;
}

void Alien::setDeltaY(double dy)
{
	m_deltaY = dy;
}

void Alien::setDeltaX(double dx)
{
	m_deltaX = dx;
}

int Alien::getFlightPlan() const
{
	return m_flightPlan;
}

void Alien::setFlightPlan(int flightPlan)
{
	m_flightPlan = flightPlan;
}

void Alien::setTravelDirection(int travelDirection)
{
	if (travelDirection == UP_LEFT)
	{
		setDeltaX(-1);
		setDeltaY(1);
	}
	else if (travelDirection == DOWN_LEFT)
	{
		setDeltaX(-1);
		setDeltaY(-1);
	}
	else if (travelDirection == DUE_LEFT)
	{
		setDeltaX(-1);
		setDeltaY(0);
	}
}

void Alien::setTravelSpeed(int travelSpeed)
{
	m_travelSpeed = travelSpeed;
}

bool Alien::isYInBounds(double y) const
{
	if (y < 0 || y > VIEW_HEIGHT - 1)
		return false;
	return true;
}

void Alien::resetYBound(double& y)
{
	if (y < 0)
		y = 0;
	else if (y >= VIEW_HEIGHT)
		y = VIEW_HEIGHT - 1;
}

void Alien::move()
{
	double x = getX() + (m_deltaX * m_travelSpeed);
	double y = getY() + (m_deltaY * m_travelSpeed);
	if (!isYInBounds(y))
		resetYBound(y);
	moveTo(x, y);
	m_flightPlan--;
}

double Alien::getDamageAmt() const
{
	return m_damageAmt;
}

int Alien::getScoreValue() const
{
	return m_scoreValue;
}

void Alien::fireTurnip() const
{
	getWorld()->addActor(new Turnip(getWorld(), getX() - 14, getY()));
	getWorld()->playSound(SOUND_ALIEN_SHOOT);
}

void Alien::fireTorpedo() const
{
	getWorld()->addActor(new Torpedo(getWorld(), getX() - 14, getY(), 180));
	getWorld()->playSound(SOUND_TORPEDO);
}

void Alien::doSomething()
{
	if (!isAlive())						// check if alive, if not, return
		return;

	if (getX() < 0)						// if has flown off screen, set alive status to false (dead) and do nothing else
	{
		setDead();
		return;
	}

	///////////////////////////////////////////////////////////////////////////////
	// CHECK FOR COLLISION WITH NACHENBLASTER PROJECTILE OR NACHENBLASTER ITSELF //
	///////////////////////////////////////////////////////////////////////////////
	getWorld()->checkCollision(this);
	if (!isAlive())						// check if alive, if not, return
		return;


	if (getFlightPlan() == 0 || getY() >= VIEW_HEIGHT - 1 || getY() <= 0)		// checking if new flight plan is needed 
	{
		if (getY() >= VIEW_HEIGHT - 1)
			setTravelDirection(DOWN_LEFT);
		else if (getY() <= 0)
			setTravelDirection(UP_LEFT);
		else if (getFlightPlan() == 0)
			setTravelDirection(randInt(1, 3));
		if (getFlightPlan() != -1)
			setFlightPlan(randInt(1, 32));
	}

	///////////////////////////////////////////////////////////////////////////////
	//					CHECK FOR SHOOTING (#5 PG 35)							 //
	///////////////////////////////////////////////////////////////////////////////
	if (getWorld()->playerInLineOfFire(this))
		if (possiblyShoot())
			return;


	move();

	///////////////////////////////////////////////////////////////////////////////
	//////////// NEED TO CHECK AGAIN FOR COLLISIONS !!! ///////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	getWorld()->checkCollision(this);
}

Smallgon::Smallgon(StudentWorld* ptrToWorld, double startX, double startY)
	: Alien(ptrToWorld, startX, startY, IID_SMALLGON, 5 * (1 + (ptrToWorld->getLevel() - 1) * 0.1), 5, 0, 0, 2.0, 250)
{
	setFlightPlan(0);
}

bool Smallgon::possiblyShoot()
{
	if (randInt(1, (20 / getWorld()->getLevel()) + 5) == 1)
	{
		fireTurnip();
		return true;
	}
	return false;
}

void Smallgon::possiblyDropGoodie() const
{
	return;
}

Smoregon::Smoregon(StudentWorld* ptrToWorld, double startX, double startY)
	: Alien(ptrToWorld, startX, startY, IID_SMOREGON, 5 * (1 + (ptrToWorld->getLevel() - 1) * 0.1), 5, 0, 0, 2.0, 250)
{
	setFlightPlan(0);
}

void Smoregon::possiblyDropGoodie() const
{
	if (randInt(1, 3) == 1)
	{
		if (randInt(1, 2) == 1)
			getWorld()->addActor(new RepairGoodie(getWorld(), getX(), getY()));
		else
			getWorld()->addActor(new TorpedoGoodie(getWorld(), getX(), getY()));
		getWorld()->playSound(SOUND_GOODIE);
	}
}

void Smoregon::rammingMode()
{
	setTravelDirection(DUE_LEFT);
	setFlightPlan(VIEW_WIDTH);
	setTravelSpeed(5);
}

bool Smoregon::possiblyShoot() 
{
	int n = randInt(1, (20 / getWorld()->getLevel()) + 5);
	if (n == 1)
	{
		fireTurnip();
		return true;
	}
	if (n == 2)
		rammingMode();
	return false;
}


Snagglegon::Snagglegon(StudentWorld* ptrToWorld, double startX, double startY)
	: Alien(ptrToWorld, startX, startY, IID_SNAGGLEGON, 10 * (1 + (ptrToWorld->getLevel() - 1) * 0.1), 15, -1, -1, 1.75, 1000)
{
	setFlightPlan(-1);		// only Snagglegons will have a "flight plan" of -1, to indicate it does not use this
}

void Snagglegon::possiblyDropGoodie() const
{
	if (randInt(1, 6) == 1)
	{
		getWorld()->addActor(new ExtraLifeGoodie(getWorld(), getX(), getY()));
		getWorld()->playSound(SOUND_GOODIE);
	}
}

bool Snagglegon::possiblyShoot()
{
	if (randInt(1, (15 / getWorld()->getLevel()) + 10) == 1)
	{
		fireTorpedo();
		return true;
	}
	return false;
}


Projectile::Projectile(StudentWorld* ptrToWorld, double startX, double startY, int imageID,
	double damageAmt, double deltaX, bool rotates, int imageDir)
	: Actor(ptrToWorld, startX, startY, imageID, imageDir, 0.5, 1), m_rotates(rotates), m_damageAmt(damageAmt), m_deltaX(deltaX)
{
}

double Projectile::getDeltaX() const
{
	return m_deltaX;
}

void Projectile::doSomething()
{
	if (!isAlive())
		return;

	if (getX() >= VIEW_WIDTH || getX() < 0)						// if has flown off screen, set alive status to false (dead) and do nothing else
	{
		setDead();
		return;
	}

	///////////////////////////////////////////////////////////
	// CHECK FOR COLLISION PG 26 #3		
	///////////////////////////////////////////////////////////
	getWorld()->checkCollision(this);
	if (!isAlive())						// check if alive, if not, return
		return;

	// implement way to check if nachenblaster shooting or alien shooting
	// for flatulence torpedoes (does this work?)
	if (firedFromAlien())
		moveTo(getX() - getDeltaX(), getY());
	else
		moveTo(getX() + getDeltaX(), getY());

	if (m_rotates)
		setDirection(getDirection() + 20);

	///////////////////////////////////////////////////////////
	// CHECK FOR COLLISION AGAIN!
	///////////////////////////////////////////////////////////
	getWorld()->checkCollision(this);
}

bool Projectile::isProjectile() const
{
	return true;
}

bool Projectile::firedFromNB() const
{
	if (isCabbage())
		return true;
	else
		return false;
}

bool Projectile::firedFromAlien() const
{
	if (isTurnip())
		return true;
	else
		return false;
}

bool Projectile::isCabbage() const
{
	return false;
}

bool Projectile::isTurnip() const
{
	return false;
}

bool Projectile::isTorpedo() const
{
	return false;
}

double Projectile::getDmgAmt() const
{
	return m_damageAmt;
}

Cabbage::Cabbage(StudentWorld* ptrToWorld, double startX, double startY)
	: Projectile(ptrToWorld, startX, startY, IID_CABBAGE, 2, 8, true, 0)
{
}

bool Cabbage::isCabbage() const
{
	return true;
}

Turnip::Turnip(StudentWorld* ptrToWorld, double startX, double startY)
	: Projectile(ptrToWorld, startX, startY, IID_TURNIP, 2, 6, true, 0)
{
}

bool Turnip::isTurnip() const
{
	return true;
}

Torpedo::Torpedo(StudentWorld* ptrToWorld, double startX, double startY, int imageDir)
	: Projectile(ptrToWorld, startX, startY, IID_TORPEDO, 8, 8, false, imageDir)
{
}

bool Torpedo::isTorpedo() const
{
	return true;
}

int Torpedo::torpedoFiredFrom() const
{
	if (getDirection() == 0)
		return FIRED_FROM_NB;							// shot by NachenBlaster
	else
		return FIRED_FROM_SNAGGLEGON;					// shot by Snagglegon
}

bool Torpedo::firedFromNB() const
{
	if (torpedoFiredFrom() == FIRED_FROM_NB)
		return true;
	else
		return false;
}

bool Torpedo::firedFromAlien() const
{
	if (torpedoFiredFrom() == FIRED_FROM_SNAGGLEGON)
		return true;
	else
		return false;
}

Goodie::Goodie(StudentWorld* ptrToWorld, double startX, double startY, int imageID)
	: Actor(ptrToWorld, startX, startY, imageID, 0, 0.5, 1)
{
}

void Goodie::doSomething()
{
	if (!isAlive())
		return;

	if (!isInBounds(getX(), getY()))
	{
		setDead();
		return;
	}

	///////////////////////////////////////
	// CHECK FOR COLLISION!!!!			 //
	///////////////////////////////////////
	getWorld()->checkCollision(this);
	if (!isAlive())						// check if alive, if not, return
		return;
	

	moveTo(getX() - 0.75, getY() - 0.75);

	///////////////////////////////////////
	// CHECK FOR COLLISION!!!!			 //
	///////////////////////////////////////
	getWorld()->checkCollision(this);

}

bool Goodie::isGoodie() const
{
	return true;
}

ExtraLifeGoodie::ExtraLifeGoodie(StudentWorld* ptrToWorld, double startX, double startY)
	: Goodie(ptrToWorld, startX, startY, IID_LIFE_GOODIE)
{
}

void ExtraLifeGoodie::giveGoodie()
{
	getWorld()->incLives();
}

RepairGoodie::RepairGoodie(StudentWorld* ptrToWorld, double startX, double startY)
	: Goodie(ptrToWorld, startX, startY, IID_REPAIR_GOODIE)
{
}

void RepairGoodie::giveGoodie()
{
	getWorld()->increasePlayerHP();
}

TorpedoGoodie::TorpedoGoodie(StudentWorld* ptrToWorld, double startX, double startY)
	: Goodie(ptrToWorld, startX, startY, IID_TORPEDO_GOODIE)
{
}

void TorpedoGoodie::giveGoodie()
{
	getWorld()->increaseTorpedoes();
}