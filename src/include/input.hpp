#include <cstdlib>
typedef struct {
  bool ARROW_UP;
  bool ARROW_DOWN;
  bool ARROW_LEFT;
  bool ARROW_RIGHT;
  bool W;
  bool A;
  bool S;
  bool D;
} Keyboard;

inline Keyboard *initKeyboard() {
  Keyboard *keyboard = (Keyboard *)malloc(sizeof(Keyboard));

  keyboard->ARROW_UP = false;
  keyboard->ARROW_DOWN = false;
  keyboard->ARROW_LEFT = false;
  keyboard->ARROW_RIGHT = false;
  keyboard->W = false;
  keyboard->A = false;
  keyboard->S = false;
  keyboard->D = false;

  return keyboard;
}