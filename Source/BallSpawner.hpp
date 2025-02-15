#pragma once
//This class will be used by GameServer to manage the spawning of a ball
//It will manage the data that goes into a ball,
//The frequency of balls,
//And if a ball will be random in its angle

class BallSpawner {
public:
  BallSpawner(sf::Clock& serverClock);

  sf::Packet createBallSpawnPacket();
  void reset();
  void appendToSpawnTime(sf::Time interval);
  bool isTimeToSpawnBall();
  void rollBackBallCount(const int& amount);
  void setSpawning(const bool& spawn);

  void addPlayerID(sf::Int8 id);
  void removePlayerID(sf::Int8 id);

private:
  sf::Time now() const;
  float createBallAngle(sf::Uint8 type);
  void determineSpawnInterval();
  sf::Uint8 determineBallType();
  float determineBallSpeed();
  float determineBallDelay();

private:
  std::vector<sf::Int8> mActivePlayerIDs;

  int mBallsSpawned;
  int mRandomBallChance;
  int mMinBallsBeforePowerup;
  int mBallsSpawnedBeforePowerup;
  bool mCanSpawnBalls;

  sf::Time mSpawnInterval;
  sf::Time mAdditionalSpawnIntervals;
  sf::Time mLastSpawnTime;
  sf::Clock* mGameServerClock;
};