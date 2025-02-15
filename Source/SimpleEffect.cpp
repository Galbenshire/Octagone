#include "pch.h"
#include "SimpleEffect.hpp"

SimpleEffect::SimpleEffect(ShaderID type)
  : mShaders()
  , mShaderType(type)
{
  switch (type) {
  case ShaderID::InvertPass:
    mShaders.load(ShaderID::InvertPass, "Media/Shaders/Fullpass.vert", "Media/Shaders/Invert.frag");
    break;
  case ShaderID::GreyscalePass:
    mShaders.load(ShaderID::GreyscalePass, "Media/Shaders/Fullpass.vert", "Media/Shaders/Greyscale.frag");
    break;
  default:
    break;
  }
}

void SimpleEffect::apply(const sf::RenderTexture & input, sf::RenderTarget & output) {
  sf::Shader& shaderFragment = mShaders.get(mShaderType);

  shaderFragment.setUniform("source", input.getTexture());
  applyShader(shaderFragment, output);
}
