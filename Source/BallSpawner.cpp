#include "pch.h"
#include "BallSpawner.hpp"
#include "NetworkProtocol.hpp"
#include "Ball.hpp"
#include "Utility.hpp"

namespace {
  const float BallAimAngles[] = { 180.f, 0.f, 270.f, 90.f, 225.f, 45.f, 135.f, 315.f };
}

BallSpawner::BallSpawner(sf::Clock & serverClock)
  : mActivePlayerIDs()
  , mBallsSpawned(0)
  , mRandomBallChance(3)
  , mMinBallsBeforePowerup(5)
  , mBallsSpawnedBeforePowerup(0)
  , mCanSpawnBalls(true)
  , mSpawnInterval(sf::seconds(4.f))
  , mAdditionalSpawnIntervals(sf::Time::Zero)
  , mLastSpawnTime(sf::Time::Zero)
  , mGameServerClock(&serverClock)
{
}

sf::Packet BallSpawner::createBallSpawnPacket() {
  sf::Uint8 type = determineBallType();
  float powerupSpeedIncrease = (type != static_cast<sf::Uint8>(Ball::Hurt)) ? 1.2f : 1.f; //Power-up balls go a bit faster

  sf::Packet packet;
  packet << static_cast<sf::Uint8>(ServerPacket::SpawnBall);
  packet << createBallAngle(type); //The angle
  packet << (determineBallSpeed() * powerupSpeedIncrease); //The speed
  packet << determineBallDelay(); //The delay before moving
  packet << type; // The type

  mBallsSpawned++;
  mBallsSpawnedBeforePowerup++;
  determineSpawnInterval();

  return packet;
}

void BallSpawner::reset() {
  mLastSpawnTime = now();
  mAdditionalSpawnIntervals = sf::Time::Zero;
}

void BallSpawner::appendToSpawnTime(sf::Time interval) {
  mAdditionalSpawnIntervals += interval;
}

bool BallSpawner::isTimeToSpawnBall() {
  return (now() >= (mSpawnInterval + mAdditionalSpawnIntervals) + mLastSpawnTime) && mCanSpawnBalls;
}

void BallSpawner::rollBackBallCount(const int & amount) {
  mBallsSpawned = std::max(0, mBallsSpawned - amount);
}

void BallSpawner::setSpawning(const bool & spawn) {
  mCanSpawnBalls = spawn;
}

void BallSpawner::addPlayerID(sf::Int8 id) {
  mActivePlayerIDs.push_back(id);
}

void BallSpawner::removePlayerID(sf::Int8 id) {
  for (auto it = mActivePlayerIDs.begin(); it != mActivePlayerIDs.end(); ) {
    if (*it == id)
      it = mActivePlayerIDs.erase(it);
    else
      ++it;
  }
}

sf::Time BallSpawner::now() const {
  return mGameServerClock->getElapsedTime();
}

float BallSpawner::createBallAngle(sf::Uint8 type) {
  //if ((mBallsSpawned % mRandomBallChance != 0 && mActivePlayerIDs.size() > 0) || type != static_cast<sf::Uint8>(Ball::Hurt)) {
  if (mActivePlayerIDs.size() > 0) {
    size_t chosenID = static_cast<size_t>(mActivePlayerIDs[randomInt(mActivePlayerIDs.size())]);
    return (BallAimAngles[--chosenID] - 20.f) + static_cast<float>(randomInt(40));
  } else {
    return static_cast<float>(randomInt(360));
  }
}

void BallSpawner::determineSpawnInterval() {
  switch (mBallsSpawned) {
  case 20:
    mSpawnInterval = sf::seconds(3.f);
    break;
  case 45:
    mSpawnInterval = sf::seconds(2.f);
    break;
  case 65:
    mSpawnInterval = sf::seconds(1.f);
    break;
  }
}

sf::Uint8 BallSpawner::determineBallType() {
  if (mBallsSpawnedBeforePowerup < mMinBallsBeforePowerup) {
    return static_cast<sf::Uint8>(Ball::Hurt);
  } else {
    if (randomInt(4) == 0) {
      mBallsSpawnedBeforePowerup = 0;
      return static_cast<sf::Uint8>(Ball::SpeedUp);
    } else {
      return static_cast<sf::Uint8>(Ball::Hurt);
    }
  }
}

float BallSpawner::determineBallSpeed() {
  return 200.f + (5.f * std::min(mBallsSpawned, 50));
}

float BallSpawner::determineBallDelay() {
  float delay(2.f);
  if (mBallsSpawned > 10)
    delay -= (0.025f * std::min(mBallsSpawned, 75));

  return delay;
}
