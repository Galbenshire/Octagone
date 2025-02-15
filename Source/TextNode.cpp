#include "pch.h"
#include "TextNode.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"

TextNode::TextNode(const FontHolder & fonts, const std::string & text)
  : mText("", fonts.get(FontID::Main), 16)
{
  setString(text);
}

void TextNode::setString(const std::string & text) {
  mText.setString(text);
  centerOrigin(mText);
}

void TextNode::drawCurrent(sf::RenderTarget & target, sf::RenderStates states) const {
  target.draw(mText, states);
}
