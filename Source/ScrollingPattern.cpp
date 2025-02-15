#include "pch.h"
#include "ScrollingPattern.hpp"
#include "Constants.hpp"
#include "Utility.hpp"

namespace {
  const sf::Color DefaultColor = getGameColor(GameColors::COLOR_TWO);
}

ScrollingPattern::ScrollingPattern(sf::Texture & texture, sf::Vector2f scrollVelocity)
  : mLayers(LayerData(DefaultColor), LayerData(DefaultColor))
  , mPatternBaseSize(texture.getSize())
  , mScrollVelocity(scrollVelocity)
{
  //Set up the sprite
  texture.setRepeated(true);
  mPatternSprite.setTexture(texture);
  sf::IntRect textureRect = setupTextureRect();
  mPatternSprite.setTextureRect(textureRect);
  centreSprite();
}

void ScrollingPattern::update() {
  //Change the pattern color if its corresponding color in Layer Data was changed
  if (mPatternSprite.getColor() != mLayers.second.color) {
    mPatternSprite.setColor(mLayers.second.color);
  }

  //Move the pattern back in place as it moves off the screen to keep the illusion of an endless background
  if (mScrollVelocity.x != 0.f) {
    if (mScrollVelocity.x > 0.f && mPatternSprite.getPosition().x >= 0.f) { //Has gone too far right...
      mPatternSprite.move(static_cast<float>(-mPatternBaseSize.x), 0.f);
    } else if (mScrollVelocity.x < 0.f && mPatternSprite.getPosition().x <= mPatternBaseSize.x * -2) { //Has gone too far left...
      mPatternSprite.move(static_cast<float>(mPatternBaseSize.x), 0.f);
    }
  }
  if (mScrollVelocity.y != 0.f) {
    if (mScrollVelocity.y > 0.f && mPatternSprite.getPosition().y >= 0.f) { //Has gone too far down...
      mPatternSprite.move(0.f, static_cast<float>(-mPatternBaseSize.y));
    } else if (mScrollVelocity.y < 0.f && mPatternSprite.getPosition().y <= mPatternBaseSize.y * -2) { //Has gone too far up...
      mPatternSprite.move(0.f, static_cast<float>(mPatternBaseSize.y));
    }
  }

  mPatternSprite.move(mScrollVelocity);
}

void ScrollingPattern::setColor(sf::Color color, LayerIndex index) {
  //Update the background first
  if (LayerIndex::Background & index)
    mLayers.first.color = color;

  //Now update the pattern
  if (LayerIndex::Pattern & index)
    mLayers.second.color = color;
}

void ScrollingPattern::setVisible(bool visible, LayerIndex index) {
  //Update the background first
  if (LayerIndex::Background & index)
    mLayers.first.visible = visible;

  //Now update the pattern
  if (LayerIndex::Pattern & index)
    mLayers.second.visible = visible;
}

void ScrollingPattern::setScrollVelocity(sf::Vector2f velocity) {
  mScrollVelocity = velocity;
}

void ScrollingPattern::setScrollVelocity(float velX, float velY) {
  mScrollVelocity.x = velX;
  mScrollVelocity.y = velY;
}

void ScrollingPattern::draw(sf::RenderTarget & target, sf::RenderStates states) const {
  //Draw the background if possible
  if (mLayers.first.visible) {
    sf::RectangleShape plainBackground;
    sf::Vector2f plainBackgroundSize(static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y));
    plainBackground.setFillColor(mLayers.first.color);
    plainBackground.setSize(plainBackgroundSize);

    target.draw(plainBackground, states);
  }

  
  if (mLayers.second.visible)
    target.draw(mPatternSprite, states);
}

sf::IntRect ScrollingPattern::setupTextureRect() {
  //The pattern should be a bit larger than the screen
  auto length = [](const int& base, const int& inc) {
    return base + (inc * 2);
  };

  return sf::IntRect(0, 0, length(VIEW_WIDTH, mPatternBaseSize.x), length(VIEW_HEIGHT, mPatternBaseSize.y));
}

void ScrollingPattern::centreSprite() {
  int cuttoffWidth = (mPatternSprite.getTextureRect().width - VIEW_WIDTH) / 2;
  int cuttoffHeight = (mPatternSprite.getTextureRect().height - VIEW_HEIGHT) / 2;

  mPatternSprite.move(static_cast<float>(-cuttoffWidth), static_cast<float>(-cuttoffHeight));
}
