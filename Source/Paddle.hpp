#pragma once
//This class represents the paddles that players will control in this game
//Paddles will only move along a straight-line spline, the start and end points based off the starting position & degrees given in its constructor
#include "Entity.hpp"
#include "PlayerBinding.hpp"
#include "Constants.hpp"
#include "ResourceIdentifiers.hpp"

//Forward decleration
class TextNode;

class Paddle : public Entity {
private:
  typedef std::vector<PlayerAction::Type> TriggerVector;

public:
  Paddle(const TextureHolder& textures, const sf::Int8& id);

  virtual unsigned int getCategory() const;
  virtual SceneNode::BoundingCircle	getBoundingShape() const;

  sf::Int8 getIdentifier() const;
  void setIdentifier(const sf::Int8 & id);

  sf::Uint8 getLives() const;
  void setLives(const sf::Uint8& lives);
  void loseLife();

  void givePowerup(CommandQueue& commands);

  void setSplinePos(float pos);
  float getSplinePos() const;
  void moveAlongSplinePos(const PlayerAction::Type& action, sf::Time dt);

  void addLocalIndicator(const FontHolder & fonts);

private:
  struct SplinePoint {
    SplinePoint(sf::Vector2f point, TriggerVector triggers)
      : point(point)
      , triggers(triggers)
    {}

    sf::Vector2f point;
    TriggerVector triggers;
  };

private:
  virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
  virtual void updateCurrent(sf::Time dt, CommandQueue& commands);

  void reAdjustPositionOnSpline();
  void rewriteLocalIndicator();

private:
  sf::Int8 mIdentifier; //Which player this paddle belongs to
  sf::Uint8 mLives; //How many hits the paddle can take
  TextNode* mLocalDisplay; //If this player is local, they get their lives displayed over/under them

  bool mHasPowerup;
  sf::Time mPowerupTimer;

  float mSplinePos; //The relative position along the spline. E.G. a value of '0.5' means being halfway along the spline
  SplinePoint mStartPoint; //The beginning of the whole spline
  SplinePoint mEndPoint; //The end of the whole spline
  bool mMustReAdjustSplinePosition; //Do we need to change the paddle's position along the spline?
};