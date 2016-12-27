#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <SFML/Graphics.hpp>

class Display {
  int w, h;
  sf::Uint8*  pixels;
  sf::Texture texture;
  sf::Sprite  sprite;
public:
  Display(int w, int h);
  ~Display();
  
  void setPixel(int x, int y, bool on);

  sf::Sprite getSprite();
};

#endif /* DISPLAY_H_ */