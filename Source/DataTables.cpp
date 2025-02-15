#include "pch.h"
#include "DataTables.hpp"

const sf::Color& getGameColor(GameColors index) {
  switch (index)
  {
  case GameColors::COLOR_ONE:
    return sf::Color(155, 188, 15);
  case GameColors::COLOR_TWO:
    return sf::Color(139, 172, 15);
  case GameColors::COLOR_THREE:
    return sf::Color(48, 98, 48);
  case GameColors::COLOR_FOUR:
    return sf::Color(15, 56, 15);
  default:
    return sf::Color(155, 188, 15);
    break;
  }
}
