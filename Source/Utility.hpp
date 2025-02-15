#pragma once

//Forward declaration
namespace sf {
  class Sprite;
  class Text;
}

// Convert enumerators to strings
std::string toString(sf::Keyboard::Key key);

// Call setOrigin() with the center of the object
void centerOrigin(sf::Sprite& sprite);
void centerOrigin(sf::Text& text);
void centerOrigin(sf::CircleShape& circle);

// Degree/radian conversion
float toDegree(float radian);
float toRadian(float degree);

// Random number generation
int randomInt(int exclusiveMax);
int randomInt(int inclusiveMin, int exclusiveMax);

// Vector operations
float length(sf::Vector2f vector);
sf::Vector2f unitVector(sf::Vector2f vector);

//Color Palette Getter
//Since the game is emulating the asthetics of the Game Boy,
//There'll be few colors in the game
enum class GameColors {
  COLOR_ONE,
  COLOR_TWO,
  COLOR_THREE,
  COLOR_FOUR
};
sf::Color getGameColor(GameColors index);

//Linear interpolation between two colors
sf::Color lerpColor(const sf::Color& a, const sf::Color& b, const float& ratio);

//Draws a plain single-color background
void drawBlankBackground(sf::RenderWindow& window, sf::Color color);