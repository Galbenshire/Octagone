#include "pch.h"
#include "Paddle.hpp"
#include "TextNode.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"

//Data specific to this class
namespace {
  //The neccessary data for the initialization of the paddles
  //They are computed in advance to ease computing
  struct PaddleSpawnData {
    PaddleSpawnData(sf::Vector2f startpos, std::vector<PlayerAction::Type> starttriggers, sf::Vector2f endpos, std::vector<PlayerAction::Type> endtriggers)
      : startPos(startpos)
      , startTriggers(starttriggers)
      , endPos(endpos)
      , endTriggers(endtriggers)
    {}

    sf::Vector2f startPos;
    std::vector<PlayerAction::Type> startTriggers;

    sf::Vector2f endPos;
    std::vector<PlayerAction::Type> endTriggers;
  };

  std::vector<PaddleSpawnData> initializePaddleSpawnData() {
    std::vector<PaddleSpawnData> data;

    data.push_back(PaddleSpawnData(sf::Vector2f(40.f, 166.f), { PlayerAction::MoveUp }, sf::Vector2f(40.f, 266.f), { PlayerAction::MoveDown }));
    data.push_back(PaddleSpawnData(sf::Vector2f(440.f, 166.f), { PlayerAction::MoveUp }, sf::Vector2f(440.f, 266.f), { PlayerAction::MoveDown }));
    data.push_back(PaddleSpawnData(sf::Vector2f(190.f, 16.f), { PlayerAction::MoveLeft }, sf::Vector2f(290.f, 16.f), { PlayerAction::MoveRight }));
    data.push_back(PaddleSpawnData(sf::Vector2f(190.f, 416.f), { PlayerAction::MoveLeft }, sf::Vector2f(290.f, 416.f), { PlayerAction::MoveRight }));
    data.push_back(PaddleSpawnData(sf::Vector2f(63.2233047f, 109.9339828f), { PlayerAction::MoveLeft }, sf::Vector2f(133.9339828f, 39.2233047f), { PlayerAction::MoveRight }));
    data.push_back(PaddleSpawnData(sf::Vector2f(346.0660172f, 392.7766953f), { PlayerAction::MoveLeft }, sf::Vector2f(416.7766953f, 322.0660172f), { PlayerAction::MoveRight }));
    data.push_back(PaddleSpawnData(sf::Vector2f(63.2233047f, 322.0660172f), { PlayerAction::MoveLeft }, sf::Vector2f(133.9339828f, 392.7766953f), { PlayerAction::MoveRight }));
    data.push_back(PaddleSpawnData(sf::Vector2f(346.0660172f, 39.2233047f), { PlayerAction::MoveLeft }, sf::Vector2f(416.7766953f, 109.9339828f), { PlayerAction::MoveRight }));

    return data;
  }

  //Regular data
  const TextureID Texture = TextureID::Entities;
  const sf::IntRect TextureRect = sf::IntRect(30, 0, 24, 24);
  const float EntityRadius = static_cast<float>(TextureRect.width / 2); //Radius of the paddle sprite
  const std::vector<PaddleSpawnData> PaddleSpawnDataTable = initializePaddleSpawnData();
}

Paddle::Paddle(const TextureHolder& textures, const sf::Int8& id)
  : Entity(textures.get(Texture), TextureRect)
  , mIdentifier(id)
  , mLives(3u)
  , mLocalDisplay(nullptr)
  , mHasPowerup(false)
  , mPowerupTimer(sf::Time::Zero)
  , mSplinePos(0.5f)
  , mStartPoint(PaddleSpawnDataTable[id - 1].startPos, PaddleSpawnDataTable[id - 1].startTriggers)
  , mEndPoint(PaddleSpawnDataTable[id - 1].endPos, PaddleSpawnDataTable[id - 1].endTriggers)
  , mMustReAdjustSplinePosition(false)
{
  centerOrigin(mSprite);
  reAdjustPositionOnSpline();
}

unsigned int Paddle::getCategory() const {
  return Category::PlayerPaddle;
}

SceneNode::BoundingCircle Paddle::getBoundingShape() const {
  return SceneNode::BoundingCircle(getWorldPosition(), EntityRadius);
}

sf::Int8 Paddle::getIdentifier() const {
  return mIdentifier;
}

void Paddle::setIdentifier(const sf::Int8 & id) {
  mIdentifier = id;
}

sf::Uint8 Paddle::getLives() const {
  return mLives;
}

void Paddle::setLives(const sf::Uint8 & lives) {
  mLives = lives;
  rewriteLocalIndicator();
}

void Paddle::loseLife() {
  mLives--;
  rewriteLocalIndicator();
}

void Paddle::givePowerup(CommandQueue& commands) {
  mHasPowerup = true;
  mPowerupTimer = sf::seconds(10.f);

  playLocalSound(commands, SFXID::ResetBindings);
}

void Paddle::setSplinePos(float pos) {
  //Clamp within 0 - 1
  mSplinePos = std::min(pos, 1.f);
  mSplinePos = std::max(0.f, mSplinePos);
  mMustReAdjustSplinePosition = true;
}

float Paddle::getSplinePos() const {
  return mSplinePos;
}

void Paddle::moveAlongSplinePos(const PlayerAction::Type & action, sf::Time dt) {
  //How fast will they move
  float paddleSpeed = mHasPowerup ? 1.f : 0.6f;
  
  //Check if they'll be moving towards the start point
  for (PlayerAction::Type& trigger : mStartPoint.triggers) {
    if (action == trigger) {
      setSplinePos(mSplinePos - (paddleSpeed * dt.asSeconds()));
      break;
    }
  }

  //Check if they'll be moving towards the end point
  for (PlayerAction::Type& trigger : mEndPoint.triggers) {
    if (action == trigger) {
      setSplinePos(mSplinePos + (paddleSpeed * dt.asSeconds()));
      break;
    }
  }
}

//This method will let the player know which paddle they're controlling, and how many lives they have
void Paddle::addLocalIndicator(const FontHolder & fonts) {
  std::unique_ptr<TextNode> indicatorDisplay(new TextNode(fonts, ""));

  //Player 4 will have their indicator above them rather than below,
  //otherwise it'll be outside the screen
  float offset = (mIdentifier == 4) ? -1.f : 1.f;
  indicatorDisplay->move(0.f, EntityRadius * 2.f * offset);
  mLocalDisplay = indicatorDisplay.get();
  attachChild(std::move(indicatorDisplay));

  rewriteLocalIndicator();
}

void Paddle::drawCurrent(sf::RenderTarget & target, sf::RenderStates states) const {
  target.draw(mSprite, states);
}

void Paddle::updateCurrent(sf::Time dt, CommandQueue & commands) {
  if (mMustReAdjustSplinePosition) {
    reAdjustPositionOnSpline();
    mMustReAdjustSplinePosition = false;
  }

  if (mHasPowerup) {
    mPowerupTimer -= dt;
    if (mPowerupTimer <= sf::Time::Zero) {
      mHasPowerup = false;
      playLocalSound(commands, SFXID::PowerupGone);
    }
  }

  Entity::updateCurrent(dt, commands);
}

void Paddle::reAdjustPositionOnSpline() {
  //Credits to this page for presenting a formula for linear interpolation
  //https://keithmaggio.wordpress.com/2011/02/15/math-magician-lerp-slerp-and-nlerp/
  //Accessed 19/02/19 10:56PM
  setPosition(mStartPoint.point + (mEndPoint.point - mStartPoint.point) * mSplinePos);
}

void Paddle::rewriteLocalIndicator() {
  if (!mLocalDisplay) { return; }

  //Lives are displayed as '*'
  std::string health;
  if (mLives == 0) {
    health.append("");
  } else {
    for (sf::Uint8 i = 0u; i < mLives; ++i) {
      health.append("* ");
    }
    health.erase(health.size() - 1);
  }
  
  mLocalDisplay->setString(health);
}
