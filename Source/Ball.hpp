#pragma once
//The base class for the balls that will appear in game

#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"

//Forward declerations
class Wall;

class Ball : public Entity {
public:
  enum Type : sf::Uint8 {
    Hurt,
    SpeedUp,
    TypeCount
  };

public:
  Ball(const TextureHolder& textures, Type type, float angle, float speed, float delayinsecs);

  virtual unsigned int getCategory() const;
  virtual sf::FloatRect getBoundingRect() const;
  virtual SceneNode::BoundingCircle	getBoundingShape() const;

  bool canCollideWithPlayer() const;
  void setCollidableWithPlayer(const bool& flag);

  void bounceOffWall(const Wall& wall, CommandQueue& commands);

private:
  virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
  virtual void updateCurrent(sf::Time dt, CommandQueue& commands);

private:
  const Type mType;
  const float mAngle;
  const float mSpeed;
  sf::Time mMoveDelay;
  bool mCollideWithPlayer; //A flag to make sure the ball only collides with a player once
};