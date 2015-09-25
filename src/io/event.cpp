#include "event.hpp"

#include <cctype>
#include <ostream>

using std::ostream;


namespace blank {

ostream &operator <<(ostream &out, const SDL_Event &evt) {
	switch (evt.type) {
#if SDL_VERSION_ATLEAST(2, 0, 4)
		case SDL_AUDIODEVICEADDED:
			out << "audio device added: " << evt.adevice;
			break;
		case SDL_AUDIODEVICEREMOVED:
			out << "audio device removed: " << evt.adevice;
			break;
#endif
		case SDL_CONTROLLERAXISMOTION:
			out << "controller axis motion: " << evt.caxis;
			break;
		case SDL_CONTROLLERBUTTONDOWN:
			out << "controller button down: " << evt.cbutton;
			break;
		case SDL_CONTROLLERBUTTONUP:
			out << "controller button up: " << evt.cbutton;
			break;
		case SDL_CONTROLLERDEVICEADDED:
			out << "controller device added: " << evt.cdevice;
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
			out << "controller device removed: " << evt.cdevice;
			break;
		case SDL_CONTROLLERDEVICEREMAPPED:
			out << "controller device remapped: " << evt.cdevice;
			break;
		case SDL_DOLLARGESTURE:
			out << "dollar gesture: " << evt.dgesture;
			break;
		case SDL_DOLLARRECORD:
			out << "dollar record: " << evt.dgesture;
			break;
		case SDL_DROPFILE:
			out << "drop file: " << evt.drop;
			break;
		case SDL_FINGERMOTION:
			out << "finger motion: " << evt.tfinger;
			break;
		case SDL_FINGERDOWN:
			out << "finger down: " << evt.tfinger;
			break;
		case SDL_FINGERUP:
			out << "finger up: " << evt.tfinger;
			break;
		case SDL_KEYDOWN:
			out << "key down: " << evt.key;
			break;
		case SDL_KEYUP:
			out << "key up: " << evt.key;
			break;
		case SDL_JOYAXISMOTION:
			out << "joystick axis motion: " << evt.jaxis;
			break;
		case SDL_JOYBALLMOTION:
			out << "joystick ball motion: " << evt.jball;
			break;
		case SDL_JOYHATMOTION:
			out << "joystick hat motion: " << evt.jhat;
			break;
		case SDL_JOYBUTTONDOWN:
			out << "joystick button down: " << evt.jbutton;
			break;
		case SDL_JOYBUTTONUP:
			out << "joystick button up: " << evt.jbutton;
			break;
		case SDL_JOYDEVICEADDED:
			out << "joystick device added: " << evt.jdevice;
			break;
		case SDL_JOYDEVICEREMOVED:
			out << "joystick device removed: " << evt.jdevice;
			break;
		case SDL_MOUSEMOTION:
			out << "mouse motion: " << evt.motion;
			break;
		case SDL_MOUSEBUTTONDOWN:
			out << "mouse button down: " << evt.button;
			break;
		case SDL_MOUSEBUTTONUP:
			out << "mouse button up: " << evt.button;
			break;
		case SDL_MOUSEWHEEL:
			out << "mouse wheel: " << evt.wheel;
			break;
		case SDL_MULTIGESTURE:
			out << "multi gesture: " << evt.mgesture;
			break;
		case SDL_QUIT:
			out << "quit: " << evt.quit;
			break;
		case SDL_SYSWMEVENT:
			out << "sys wm: " << evt.syswm;
			break;
		case SDL_TEXTEDITING:
			out << "text editing: " << evt.edit;
			break;
		case SDL_TEXTINPUT:
			out << "text input: " << evt.text;
			break;
		case SDL_USEREVENT:
			out << "user: " << evt.user;
			break;
		case SDL_WINDOWEVENT:
			out << "window: " << evt.window;
			break;
		default:
			out << "unknown";
			break;
	}
	return out;
}

ostream &operator <<(ostream &out, const SDL_WindowEvent &evt) {
	switch (evt.event) {
		case SDL_WINDOWEVENT_SHOWN:
			out << "shown, window ID: " << evt.windowID;
			break;
		case SDL_WINDOWEVENT_HIDDEN:
			out << "hidden, window ID: " << evt.windowID;
			break;
		case SDL_WINDOWEVENT_EXPOSED:
			out << "exposed, window ID: " << evt.windowID;
			break;
		case SDL_WINDOWEVENT_MOVED:
			out << "moved, window ID: " << evt.windowID
				<< ", position: " << evt.data1 << '+' << evt.data2;
			break;
		case SDL_WINDOWEVENT_RESIZED:
			out << "resized, window ID: " << evt.windowID
				<< ", size: " << evt.data1 << 'x' << evt.data2;
			break;
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			out << "size changed, window ID: " << evt.windowID
				<< ", size: " << evt.data1 << 'x' << evt.data2;
			break;
		case SDL_WINDOWEVENT_MINIMIZED:
			out << "minimized, window ID: " << evt.windowID;
			break;
		case SDL_WINDOWEVENT_MAXIMIZED:
			out << "maximized, window ID: " << evt.windowID;
			break;
		case SDL_WINDOWEVENT_RESTORED:
			out << "restored, window ID: " << evt.windowID;
			break;
		case SDL_WINDOWEVENT_ENTER:
			out << "mouse entered, window ID: " << evt.windowID;
			break;
		case SDL_WINDOWEVENT_LEAVE:
			out << "mouse left, window ID: " << evt.windowID;
			break;
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			out << "focus gained, window ID: " << evt.windowID;
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			out << "focus lost, window ID: " << evt.windowID;
			break;
		case SDL_WINDOWEVENT_CLOSE:
			out << "closed, window ID: " << evt.windowID;
			break;
		default:
			out << "unknown";
			break;
	}
	return out;
}

ostream &operator <<(ostream &out, const SDL_KeyboardEvent &evt) {
	out << "window ID: " << evt.windowID
		<< ", state: " << (evt.state == SDL_PRESSED ? "pressed" : "released")
		<< ", repeat: " << (evt.repeat ? "yes" : "no")
		<< ", keysym: " << evt.keysym;
	return out;
}

ostream &operator <<(ostream &out, const SDL_Keysym &keysym) {
	out << "scancode: " << int(keysym.scancode)
		<< ", sym: " << int(keysym.sym);
	if (std::isprint(keysym.sym)) {
		out << " (\"" << char(keysym.sym) << "\")";
	}
	if (keysym.mod) {
		out << "mod:";
		if (keysym.mod & KMOD_LSHIFT) {
			out << " LSHIFT";
		}
		if (keysym.mod & KMOD_RSHIFT) {
			out << " RSHIFT";
		}
		if (keysym.mod & KMOD_LCTRL) {
			out << " LCTRL";
		}
		if (keysym.mod & KMOD_RCTRL) {
			out << " RCTRL";
		}
		if (keysym.mod & KMOD_LALT) {
			out << " LALT";
		}
		if (keysym.mod & KMOD_RALT) {
			out << " RALT";
		}
		if (keysym.mod & KMOD_LGUI) {
			out << " LSUPER";
		}
		if (keysym.mod & KMOD_RGUI) {
			out << " RSUPER";
		}
		if (keysym.mod & KMOD_NUM) {
			out << " NUM";
		}
		if (keysym.mod & KMOD_CAPS) {
			out << " CAPS";
		}
		if (keysym.mod & KMOD_MODE) {
			out << " ALTGR";
		}
	}
	return out;
}

ostream &operator <<(ostream &out, const SDL_TextEditingEvent &evt) {
	out << "window ID: " << evt.windowID
		<< ", text: \"" << evt.text
		<< "\", start: " << evt.start
		<< ", length: " << evt.length;
	return out;
}

ostream &operator <<(ostream &out, const SDL_TextInputEvent &evt) {
	out << "window ID: " << evt.windowID
		<< ", text: \"" << evt.text << '"';
	return out;
}

ostream &operator <<(ostream &out, const SDL_MouseMotionEvent &evt) {
	out << "window ID: " << evt.windowID
		<< ", mouse ID: " << evt.which
		<< ", position: " << evt.x << '+' << evt.y
		<< ", delta: " << evt.xrel << '+' << evt.yrel;
	if (evt.state) {
		out << "buttons:";
		if (evt.state & SDL_BUTTON_LMASK) {
			out << " left";
		}
		if (evt.state & SDL_BUTTON_MMASK) {
			out << " middle";
		}
		if (evt.state & SDL_BUTTON_RMASK) {
			out << " right";
		}
		if (evt.state & SDL_BUTTON_X1MASK) {
			out << " X1";
		}
		if (evt.state & SDL_BUTTON_X2MASK) {
			out << " X2";
		}
	}
	return out;
}

ostream &operator <<(ostream &out, const SDL_MouseButtonEvent &evt) {
	out << "window ID: " << evt.windowID
		<< ", mouse ID: " << evt.which
		<< ", button: ";
	switch (evt.button) {
		case SDL_BUTTON_LEFT:
			out << "left";
			break;
		case SDL_BUTTON_MIDDLE:
			out << "middle";
			break;
		case SDL_BUTTON_RIGHT:
			out << "right";
			break;
		case SDL_BUTTON_X1:
			out << "X1";
			break;
		case SDL_BUTTON_X2:
			out << "X2";
			break;
		default:
			out << int(evt.button);
			break;
	}
	out << ", state: " << (evt.state == SDL_PRESSED ? "pressed" : "released")
		<< ", clicks: " << int(evt.clicks)
		<< ", position: " << evt.x << '+' << evt.y;
	return out;
}

ostream &operator <<(ostream &out, const SDL_MouseWheelEvent &evt) {
	out << "window ID: " << evt.windowID
		<< ", mouse ID: " << evt.which
		<< ", delta: " << evt.x << '+' << evt.y
#if SDL_VERSION_ATLEAST(2, 0, 4)
		<< ", direction: " << (evt.direction == SDL_MOUSEWHEEL_NORMAL ? "normal" : "flipped")
#endif
		;
	return out;
}

ostream &operator <<(ostream &out, const SDL_JoyAxisEvent &evt) {
	out << "joystick ID: " << evt.which
		<< ", axis ID: " << int(evt.axis)
		<< ", value: " << (float(evt.value) / 32768.0f);
	return out;
}

ostream &operator <<(ostream &out, const SDL_JoyBallEvent &evt) {
	out << "joystick ID: " << evt.which
		<< ", ball ID: " << int(evt.ball)
		<< ", delta: " << evt.xrel << '+' << evt.yrel;
	return out;
}

ostream &operator <<(ostream &out, const SDL_JoyHatEvent &evt) {
	out << "joystick ID: " << evt.which
		<< ", hat ID: " << int(evt.hat)
		<< ", value: ";
	switch (evt.value) {
		case SDL_HAT_LEFTUP:
			out << "left up";
			break;
		case SDL_HAT_UP:
			out << "up";
			break;
		case SDL_HAT_RIGHTUP:
			out << "right up";
			break;
		case SDL_HAT_LEFT:
			out << "left";
			break;
		case SDL_HAT_CENTERED:
			out << "center";
			break;
		case SDL_HAT_RIGHT:
			out << "right";
			break;
		case SDL_HAT_LEFTDOWN:
			out << "left down";
			break;
		case SDL_HAT_DOWN:
			out << "down";
			break;
		case SDL_HAT_RIGHTDOWN:
			out << "right down";
			break;
		default:
			out << "unknown";
			break;
	}
	return out;
}

ostream &operator <<(ostream &out, const SDL_JoyButtonEvent &evt) {
	out << "joystick ID: " << evt.which
		<< ", button ID: " << int(evt.button)
		<< ", state: " << (evt.state == SDL_PRESSED ? "pressed" : "released");
	return out;
}

ostream &operator <<(ostream &out, const SDL_JoyDeviceEvent &evt) {
	out << "joystick ID: \"" << evt.which;
	return out;
}

ostream &operator <<(ostream &out, const SDL_ControllerAxisEvent &evt) {
	out << "controller ID: " << evt.which
		<< ", axis ID: " << int(evt.axis)
		<< ", value: " << (float(evt.value) / 32768.0f);
	return out;
}

ostream &operator <<(ostream &out, const SDL_ControllerButtonEvent &evt) {
	out << "controller ID: " << evt.which
		<< ", button ID: " << int(evt.button)
		<< ", state: " << (evt.state == SDL_PRESSED ? "pressed" : "released");
	return out;
}

ostream &operator <<(ostream &out, const SDL_ControllerDeviceEvent &evt) {
	out << "controller ID: " << evt.which;
	return out;
}

#if SDL_VERSION_ATLEAST(2, 0, 4)
ostream &operator <<(ostream &out, const SDL_AudioDeviceEvent &evt) {
	out << "device ID: " << evt.which
		<< ", capture: " << (evt.iscapture ? "yes" : "no");
	return out;
}
#endif

ostream &operator <<(ostream &out, const SDL_QuitEvent &evt) {
	out << "quit";
	return out;
}

ostream &operator <<(ostream &out, const SDL_UserEvent &evt) {
	out << "window ID: " << evt.windowID
		<< ", code: " << evt.code
		<< ", data 1: " << evt.data1
		<< ", data 2: " << evt.data2;
	return out;
}

ostream &operator <<(ostream &out, const SDL_SysWMEvent &evt) {
	if (evt.msg) {
		out << "with message";
	} else {
		out << "without message";
	}
	return out;
}

ostream &operator <<(ostream &out, const SDL_TouchFingerEvent &evt) {
	out << "device ID: " << evt.touchId
		<< ", finger ID: " << evt.fingerId
		<< ", position: " << evt.x << ' ' << evt.y
		<< ", delta: " << evt.dx << ' ' << evt.dy
		<< ", pressure: " << evt.pressure;
	return out;
}

ostream &operator <<(ostream &out, const SDL_MultiGestureEvent &evt) {
	out << "device ID: " << evt.touchId
		<< ", theta: " << evt.dTheta
		<< ", distance: " << evt.dDist
		<< ", position: " << evt.x << ' ' << evt.y
		<< ", fingers: " << evt.numFingers;
	return out;
}

ostream &operator <<(ostream &out, const SDL_DollarGestureEvent &evt) {
	out << "device ID: " << evt.touchId
		<< ", gesture ID: " << evt.gestureId
		<< ", fingers: " << evt.numFingers
		<< ", error: " << evt.error
		<< ", position: " << evt.x << ' ' << evt.y;
	return out;
}

ostream &operator <<(ostream &out, const SDL_DropEvent &evt) {
	out << "file: " << evt.file;
	return out;
}

}
