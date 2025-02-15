#pragma once
//This type of ball will hurt players if they touch it

#include "Ball.hpp"

class HurtBall : public Ball {
public:
  HurtBall(const TextureHolder& textures, float angle, float speed, float delayinsecs);

  virtual unsigned int getCategory() const;

  void removeAllBalls();
};
