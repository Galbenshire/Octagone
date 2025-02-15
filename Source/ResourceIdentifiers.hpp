#pragma once

// Forward declaration of SFML classes
namespace sf {
  class Texture;
  class Font;
  class Shader;
  class SoundBuffer;
}

enum class TextureID { TitleScreen, Buttons, Entities, Particle, Pixel };
enum class ShaderID { InvertPass, GreyscalePass };
enum class FontID { Main };
enum class SFXID { MenuSelect, BallImpact, ResetBindings, PowerupGone, Hit };
enum class MusicID { };

// Forward declaration and a few type definitions
template <typename Resource, typename Identifier>
class ResourceHolder;

typedef ResourceHolder<sf::Texture, TextureID> TextureHolder;
typedef ResourceHolder<sf::Font, FontID> FontHolder;
typedef ResourceHolder<sf::Shader, ShaderID> ShaderHolder;
typedef ResourceHolder<sf::SoundBuffer, SFXID> SoundBufferHolder;
