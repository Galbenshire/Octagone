#pragma once

#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"


class Wall : public Entity {
public:
  Wall(const TextureHolder& textures);

  virtual unsigned int getCategory() const;
  virtual SceneNode::BoundingCircle	getBoundingShape() const;

private:
  virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
  virtual void updateCurrent(sf::Time dt, CommandQueue& commands);
};