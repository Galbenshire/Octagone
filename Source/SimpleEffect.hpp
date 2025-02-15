#pragma once
//A custom post effect that takes in a very basic fragment shader, usually a fullscreen color changer
#include "PostEffect.hpp"
#include "ResourceIdentifiers.hpp"
#include "ResourceHolder.hpp"

class SimpleEffect : public PostEffect {
public:
  SimpleEffect(ShaderID type);

  virtual void apply(const sf::RenderTexture& input, sf::RenderTarget& output);

private:
  ShaderHolder mShaders;
  ShaderID mShaderType;
};