#pragma once

//A special asthetic class. Generates a given pattern across the screen; scrolling is possible.
//This class contains two layers: The pattern, and a blank background behind it.
class ScrollingPattern : public sf::Drawable {
public:
  enum LayerIndex {
    Pattern = 1 << 0,
    Background = 1 << 1
  };

private:
  struct LayerData {
    LayerData(sf::Color color)
      : color(color)
      , visible(true)
    {
    }

    sf::Color color;
    bool visible;
  };

public:
  ScrollingPattern(sf::Texture& texture, sf::Vector2f scrollVelocity = sf::Vector2f());

  void update();

  void setColor(sf::Color color, LayerIndex index); //Allows for bit masks to set both layers
  void setVisible(bool visible, LayerIndex index); //Allows for bit masks to set both layers
  void setScrollVelocity(sf::Vector2f velocity);
  void setScrollVelocity(float velX, float velY);

private:
  virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const;

  sf::IntRect setupTextureRect();
  void centreSprite();

private:
  std::pair<LayerData, LayerData> mLayers;
  sf::Sprite mPatternSprite;
  sf::Vector2i mPatternBaseSize;
  sf::Vector2f mScrollVelocity;
};