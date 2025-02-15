#pragma once
//This states represents what what the player will see upon going to the credits
//It shows who worked on the game, and what resources came from where, if they're not native to this game
//-- Made by Niall Mulligan

#include "State.hpp"
#include "ScrollingPattern.hpp"

class CreditsState : public State {
public:
	CreditsState(StateStack& stack, Context context);

	virtual void draw();
	virtual bool update(sf::Time dt);
	virtual bool handleEvent(const sf::Event& event);

private:
	void addCredit(const std::string& credit, const float& relY);
	bool isOldestCreditAboveScreen();

private:
	ScrollingPattern mPattern;

	float mCreditsScrollSpeed;
	std::deque<sf::Text> mCredits;
};