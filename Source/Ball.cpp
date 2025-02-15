#include "pch.h"
#include "Ball.hpp"
#include "EmitterNode.hpp"
#include "Wall.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"

//Data specific to this class
namespace {
  sf::IntRect getTextureRect(Ball::Type type) {
    switch (type) {
    case Ball::Hurt:
      return sf::IntRect(0, 30, 24, 24);
    case Ball::SpeedUp:
      return sf::IntRect(24, 30, 24, 24);
    }
    return sf::IntRect(0, 30, 24, 24);
  }

  TextureID Texture = TextureID::Entities;
  float Radius = static_cast<float>(getTextureRect(Ball::Hurt).width / 2);
}

Ball::Ball(const TextureHolder & textures, Type type, float angle, float speed, float delayinsecs)
  : Entity(textures.get(Texture), getTextureRect(type))
  , mType(type)
  , mAngle(angle)
  , mSpeed(speed)
  , mMoveDelay(sf::seconds(delayinsecs))
  , mCollideWithPlayer(true)
{
  centerOrigin(mSprite);
}

unsigned int Ball::getCategory() const {
  return (mType == Hurt) ? Category::HurtBall : Category::Powerup;
}

sf::FloatRect Ball::getBoundingRect() const {
  return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}

SceneNode::BoundingCircle Ball::getBoundingShape() const {
  return SceneNode::BoundingCircle(getWorldPosition(), Radius);
}

bool Ball::canCollideWithPlayer() const {
  return mCollideWithPlayer;
}

void Ball::setCollidableWithPlayer(const bool & flag) {
  mCollideWithPlayer = flag;
}

void Ball::bounceOffWall(const Wall & wall, CommandQueue& commands) {
  //Code from this GitHub project was referenced in order to set up proper collision
  //GitHub: https://github.com/OneLoneCoder/videos/blob/master/OneLoneCoder_Balls1.cpp (Lines 210 - 222)
  //YouTube: https://youtu.be/LPzyNOHY3A4?t=769
  //Accessed: 10:55PM 14/03/2019

  SceneNode::BoundingCircle lhCircle = getBoundingShape();
  SceneNode::BoundingCircle rhCircle = wall.getBoundingShape();

  float xDiff = lhCircle.position.x - rhCircle.position.x;
  float yDiff = lhCircle.position.y - rhCircle.position.y;

  float distance = std::sqrtf(std::pow(xDiff, 2) + std::pow(yDiff, 2));
  float overlap = distance - lhCircle.radius - rhCircle.radius;

  move(-overlap * xDiff / distance, -overlap * yDiff / distance);
  setVelocity((xDiff / distance) * mSpeed, (yDiff / distance) * mSpeed);

  playLocalSound(commands, SFXID::BallImpact);
}

void Ball::drawCurrent(sf::RenderTarget & target, sf::RenderStates states) const {
  target.draw(mSprite, states);
}

void Ball::updateCurrent(sf::Time dt, CommandQueue & commands) {
  if (mMoveDelay > sf::Time::Zero) {
    mMoveDelay -= dt;
    if (mMoveDelay <= sf::Time::Zero) {
      setVelocity(std::cosf(toRadian(mAngle)) * mSpeed, std::sinf(toRadian(mAngle)) * mSpeed);

      //Give it a trail
      std::unique_ptr<EmitterNode> trail(new EmitterNode(Particle::BallTrail));
      //trail->setPosition(getBoundingRect().width / 2.f, getBoundingRect().height / 2.f);
      attachChild(std::move(trail));
    }
  }

  Entity::updateCurrent(dt, commands);
}
