#pragma once

#include <SFML/Graphics/Color.hpp>

//Color Palette Data
//Since the game is emulating the asthetics of the Game Boy,
//There'll be few colors in the game
enum class GameColors {
  COLOR_ONE,
  COLOR_TWO,
  COLOR_THREE,
  COLOR_FOUR
};

const sf::Color& getGameColor(GameColors index);