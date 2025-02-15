#include "pch.h"
#include "CreditsState.hpp"
#include "Constants.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"

//Data specific to this class
namespace {
  const float PatternScrollSpeed = 1.5f;
  const float CreditsScrollSpeed = -1.f;
}

CreditsState::CreditsState(StateStack & stack, Context context)
	: State(stack, context)
	, mPattern(context.textures->get(TextureID::TitleScreen), sf::Vector2f(0.f, PatternScrollSpeed))
	, mCreditsScrollSpeed(CreditsScrollSpeed)
{
    //Set the colors of the pattern
	mPattern.setColor(getGameColor(GameColors::COLOR_TWO), ScrollingPattern::Pattern);
	mPattern.setColor(getGameColor(GameColors::COLOR_ONE), ScrollingPattern::Background);

    //Add all our credits
	addCredit("A GAME BY", 50.f);
	addCredit("RONAN DUFFY", 40.f);
	addCredit("AND", 20.f);
	addCredit("NIALL MULLIGAN", 20.f);

	addCredit("USING CODE FROM", 80.f);
    addCredit("JOHN LOANE", 20.f);
}

void CreditsState::draw() {
	sf::RenderWindow& window = *getContext().window;
	window.setView(window.getDefaultView());

	window.draw(mPattern); //Draw the pattern first

	//Only try to draw credits if there are any left
	if (mCredits.empty()) { return; }

	//Get a rectangle that encompasses the bounds of the view window
	//Credits to this forum post for leading us in the right direction:
	//https://en.sfml-dev.org/forums/index.php?topic=21355.0
	sf::FloatRect viewBounds(sf::Vector2f(), window.getView().getSize());

    //Draw any credits within view
	for (auto& credit : mCredits) {
      if (credit.getGlobalBounds().intersects(viewBounds))
        window.draw(credit);
	}
}

bool CreditsState::update(sf::Time dt) {
	mPattern.update();

	//If the player is pressing Space, make the credits move faster
    float scrollBoost = sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ? -1.5f : 0.f;
    //Move credits upwards
	for (auto& credit : mCredits)
      credit.move(0.f, mCreditsScrollSpeed + scrollBoost);

	//Remove any credits that have gone above the screen
	while (!mCredits.empty() && isOldestCreditAboveScreen())
      mCredits.pop_front();

	//No more credits? The Credits State is done.
	if (mCredits.empty())
      requestStackPop();

	return false;
}

bool CreditsState::handleEvent(const sf::Event & event) {
	return false;
}

void CreditsState::addCredit(const std::string& credit, const float& relY) {
  //Start below the most recent credit in the deque,
  //Unless there are none, then start at the bottom the screen
  float baseY = !mCredits.empty() ? mCredits.back().getPosition().y : VIEW_HEIGHT;

  sf::Text newCredit(credit, getContext().fonts->get(FontID::Main), 25);
  newCredit.setFillColor(getGameColor(GameColors::COLOR_FOUR));
  centerOrigin(newCredit);
  newCredit.setPosition(VIEW_WIDTH / 2, baseY);
  newCredit.move(0.f, relY);

  mCredits.push_back(newCredit);
}

//Check if the font-most credit in the Credits deque has gone high enough above the screen
bool CreditsState::isOldestCreditAboveScreen() {
	sf::FloatRect creditBounds(mCredits.front().getGlobalBounds());

	return (creditBounds.top + creditBounds.height) < -60.f; 
}
