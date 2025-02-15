#pragma once
//In this state, the player will enter an IP address that will be used to host/join games
#include "State.hpp"
#include "Container.hpp"
#include "Label.hpp"
#include "ScrollingPattern.hpp"

class IPConfigState : public State {
public:
	IPConfigState(StateStack& stack, Context context);

	virtual void draw();
	virtual bool update(sf::Time dt);
	virtual bool handleEvent(const sf::Event& event);

private:
	ScrollingPattern mPattern;
	GUI::Container mGUIContainer;

	std::regex mIPFormat;
	sf::String mInputString;
	bool mUpdateInputDisplay;

	GUI::Label::Ptr mInputDisplay;
	GUI::Label::Ptr mInputResult;
};