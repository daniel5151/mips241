#include "display.h"

#include <SFML/Graphics.hpp>

Display::Display(int w, int h) : w(w), h(h) {
  pixels = new sf::Uint8[w * h * 4];

  texture.create(w, h);

  sprite.setTexture(texture);
}
Display::~Display() {
  delete [] pixels;
}

void Display::setPixel(int x, int y, bool on) {
  pixels[(x + y * w) * 4    ] = (on ? 255 : 0); // R?
  pixels[(x + y * w) * 4 + 1] = (on ? 255 : 0); // G?
  pixels[(x + y * w) * 4 + 2] = (on ? 255 : 0); // B?
  pixels[(x + y * w) * 4 + 3] = (on ? 255 : 0); // A?
}

sf::Sprite Display::getSprite() {
  texture.update(pixels);
  return sprite;
}
