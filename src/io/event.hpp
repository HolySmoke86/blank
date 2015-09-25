#ifndef BLANK_IO_EVENT_HPP_
#define BLANK_IO_EVENT_HPP_

#include <iosfwd>
#include <SDL.h>


namespace blank {

std::ostream &operator <<(std::ostream &, const SDL_Event &);

std::ostream &operator <<(std::ostream &, const SDL_WindowEvent &);
std::ostream &operator <<(std::ostream &, const SDL_KeyboardEvent &);
std::ostream &operator <<(std::ostream &, const SDL_TextEditingEvent &);
std::ostream &operator <<(std::ostream &, const SDL_TextInputEvent &);
std::ostream &operator <<(std::ostream &, const SDL_MouseMotionEvent &);
std::ostream &operator <<(std::ostream &, const SDL_MouseButtonEvent &);
std::ostream &operator <<(std::ostream &, const SDL_MouseWheelEvent &);
std::ostream &operator <<(std::ostream &, const SDL_JoyAxisEvent &);
std::ostream &operator <<(std::ostream &, const SDL_JoyBallEvent &);
std::ostream &operator <<(std::ostream &, const SDL_JoyHatEvent &);
std::ostream &operator <<(std::ostream &, const SDL_JoyButtonEvent &);
std::ostream &operator <<(std::ostream &, const SDL_JoyDeviceEvent &);
std::ostream &operator <<(std::ostream &, const SDL_ControllerAxisEvent &);
std::ostream &operator <<(std::ostream &, const SDL_ControllerButtonEvent &);
std::ostream &operator <<(std::ostream &, const SDL_ControllerDeviceEvent &);
#if SDL_VERSION_ATLEAST(2, 0, 4)
std::ostream &operator <<(std::ostream &, const SDL_AudioDeviceEvent &);
#endif
std::ostream &operator <<(std::ostream &, const SDL_QuitEvent &);
std::ostream &operator <<(std::ostream &, const SDL_UserEvent &);
std::ostream &operator <<(std::ostream &, const SDL_SysWMEvent &);
std::ostream &operator <<(std::ostream &, const SDL_TouchFingerEvent &);
std::ostream &operator <<(std::ostream &, const SDL_MultiGestureEvent &);
std::ostream &operator <<(std::ostream &, const SDL_DollarGestureEvent &);
std::ostream &operator <<(std::ostream &, const SDL_DropEvent &);

std::ostream &operator <<(std::ostream &, const SDL_Keysym &);

}

#endif
