#include "EventTest.hpp"

#include "io/event.hpp"

#include <sstream>
#include <string>
#include <SDL_syswm.h>


CPPUNIT_TEST_SUITE_REGISTRATION(blank::test::EventTest);

using namespace std;

namespace blank {
namespace test {

void EventTest::setUp() {

}

void EventTest::tearDown() {

}


namespace {

template<class T>
string string_cast(const T &val) {
	stringstream str;
	str << val;
	return str.str();
}

}

#if SDL_VERSION_ATLEAST(2, 0, 4)

void EventTest::testAudioDevice() {
	SDL_Event event;
	event.type = SDL_AUDIODEVICEADDED;
	event.adevice.which = 1;
	event.adevice.iscapture = false;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL audio device event",
		string("audio device added: device ID: 1, capture: no"), string_cast(event));
	event.adevice.which = 2;
	event.adevice.iscapture = true;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL audio device event",
		string("audio device added: device ID: 2, capture: yes"), string_cast(event));
		event.type = SDL_AUDIODEVICEREMOVED;
	event.adevice.which = 3;
	event.adevice.iscapture = false;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL audio device event",
		string("audio device removed: device ID: 3, capture: no"), string_cast(event));
	event.adevice.which = 4;
	event.adevice.iscapture = true;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL audio device event",
		string("audio device removed: device ID: 4, capture: yes"), string_cast(event));
}

#endif

void EventTest::testController() {
	SDL_Event event;
	event.type = SDL_CONTROLLERAXISMOTION;
	event.caxis.which = 0;
	event.caxis.axis = 1;
	event.caxis.value = 16384;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL controller axis event",
		string("controller axis motion: controller ID: 0, axis ID: 1, value: 0.5"), string_cast(event));

	event.type = SDL_CONTROLLERBUTTONDOWN;
	event.cbutton.which = 2;
	event.cbutton.button = 3;
	event.cbutton.state = SDL_PRESSED;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL controller button event",
		string("controller button down: controller ID: 2, button ID: 3, state: pressed"), string_cast(event));
	event.type = SDL_CONTROLLERBUTTONUP;
	event.cbutton.which = 4;
	event.cbutton.button = 5;
	event.cbutton.state = SDL_RELEASED;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL controller button event",
		string("controller button up: controller ID: 4, button ID: 5, state: released"), string_cast(event));

	event.type = SDL_CONTROLLERDEVICEADDED;
	event.cdevice.which = 6;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL controller device event",
		string("controller device added: controller ID: 6"), string_cast(event));
	event.type = SDL_CONTROLLERDEVICEREMOVED;
	event.cdevice.which = 7;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL controller device event",
		string("controller device removed: controller ID: 7"), string_cast(event));
	event.type = SDL_CONTROLLERDEVICEREMAPPED;
	event.cdevice.which = 8;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL controller device event",
		string("controller device remapped: controller ID: 8"), string_cast(event));
}

void EventTest::testDollar() {
	SDL_Event event;
	event.type = SDL_DOLLARGESTURE;
	event.dgesture.touchId = 0;
	event.dgesture.gestureId = 1;
	event.dgesture.numFingers = 2;
	event.dgesture.error = 3;
	event.dgesture.x = 4;
	event.dgesture.y = 5;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL dollar gesture event",
		string("dollar gesture: device ID: 0, gesture ID: 1, fingers: 2, error: 3, position: 4 5"), string_cast(event));

	event.type = SDL_DOLLARRECORD;
	event.dgesture.touchId = 6;
	event.dgesture.gestureId = 7;
	event.dgesture.numFingers = 8;
	event.dgesture.error = 9;
	event.dgesture.x = 10;
	event.dgesture.y = 11;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL dollar record event",
		string("dollar record: device ID: 6, gesture ID: 7, fingers: 8, error: 9, position: 10 11"), string_cast(event));
}

void EventTest::testDrop() {
	char filename[] = "/dev/random";
	SDL_Event event;
	event.type = SDL_DROPFILE;
	event.drop.file = filename;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL drop file event",
		string("drop file: file: ") + filename, string_cast(event));
}

void EventTest::testFinger() {
	SDL_Event event;
	event.type = SDL_FINGERMOTION;
	event.tfinger.touchId = 0;
	event.tfinger.fingerId = 1;
	event.tfinger.x = 2;
	event.tfinger.y = 3;
	event.tfinger.dx = 4;
	event.tfinger.dy = 5;
	event.tfinger.pressure = 6;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL finger motion event",
		string("finger motion: device ID: 0, finger ID: 1, position: 2 3, delta: 4 5, pressure: 6"), string_cast(event));

	event.type = SDL_FINGERDOWN;
	event.tfinger.touchId = 7;
	event.tfinger.fingerId = 8;
	event.tfinger.x = 9;
	event.tfinger.y = 10;
	event.tfinger.dx = 11;
	event.tfinger.dy = 12;
	event.tfinger.pressure = 13;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL finger down event",
		string("finger down: device ID: 7, finger ID: 8, position: 9 10, delta: 11 12, pressure: 13"), string_cast(event));

	event.type = SDL_FINGERUP;
	event.tfinger.touchId = 14;
	event.tfinger.fingerId = 15;
	event.tfinger.x = 16;
	event.tfinger.y = 17;
	event.tfinger.dx = 18;
	event.tfinger.dy = 19;
	event.tfinger.pressure = 20;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL finger up event",
		string("finger up: device ID: 14, finger ID: 15, position: 16 17, delta: 18 19, pressure: 20"), string_cast(event));
}

void EventTest::testKey() {
	SDL_Event event;
	event.type = SDL_KEYDOWN;
	event.key.windowID = 0;
	event.key.state = SDL_PRESSED;
	event.key.repeat = 0;
	event.key.keysym.scancode = SDL_SCANCODE_0;
	event.key.keysym.sym = SDLK_0;
	event.key.keysym.mod = KMOD_NONE;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL key down event",
		string("key down: window ID: 0, state: pressed, repeat: no, keysym: "
			"scancode: ") + to_string(int(SDL_SCANCODE_0)) + ", sym: "
			+ to_string(int(SDLK_0)) +" (\"0\")", string_cast(event));
	event.key.windowID = 2;
	event.key.repeat = 1;
	event.key.keysym.scancode = SDL_SCANCODE_BACKSPACE;
	event.key.keysym.sym = SDLK_BACKSPACE;
	event.key.keysym.mod = KMOD_LCTRL | KMOD_LALT;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL key down event",
		string("key down: window ID: 2, state: pressed, repeat: yes, keysym: "
			"scancode: ") + to_string(int(SDL_SCANCODE_BACKSPACE)) + ", sym: "
			+ to_string(int(SDLK_BACKSPACE)) +" (\"Backspace\"), mod: LCTRL LALT", string_cast(event));

	event.type = SDL_KEYUP;
	event.key.windowID = 1;
	event.key.state = SDL_RELEASED;
	event.key.repeat = 0;
	event.key.keysym.scancode = SDL_SCANCODE_SYSREQ;
	event.key.keysym.sym = SDLK_SYSREQ;
	event.key.keysym.mod = KMOD_LSHIFT | KMOD_RALT;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL key up event",
		string("key up: window ID: 1, state: released, repeat: no, keysym: "
			"scancode: ") + to_string(int(SDL_SCANCODE_SYSREQ)) + ", sym: "
			+ to_string(int(SDLK_SYSREQ)) +" (\"SysReq\"), mod: LSHIFT RALT", string_cast(event));
	event.key.windowID = 3;
	event.key.repeat = 1;
	event.key.keysym.scancode = SDL_SCANCODE_L;
	event.key.keysym.sym = SDLK_l;
	event.key.keysym.mod = KMOD_RSHIFT | KMOD_RCTRL | KMOD_LGUI;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL key up event",
		string("key up: window ID: 3, state: released, repeat: yes, keysym: "
			"scancode: ") + to_string(int(SDL_SCANCODE_L)) + ", sym: "
			+ to_string(int(SDLK_l)) +" (\"L\"), mod: RSHIFT RCTRL LSUPER", string_cast(event));
	event.key.windowID = 4;
	event.key.repeat = 2;
	event.key.keysym.scancode = SDL_SCANCODE_VOLUMEUP;
	event.key.keysym.sym = SDLK_VOLUMEUP;
	event.key.keysym.mod = KMOD_RGUI | KMOD_NUM | KMOD_CAPS | KMOD_MODE;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL key up event",
		string("key up: window ID: 4, state: released, repeat: yes, keysym: "
			"scancode: ") + to_string(int(SDL_SCANCODE_VOLUMEUP)) + ", sym: "
			+ to_string(int(SDLK_VOLUMEUP)) +" (\"VolumeUp\"), mod: RSUPER NUM CAPS ALTGR", string_cast(event));
}

void EventTest::testJoystick() {
	SDL_Event event;
	event.type = SDL_JOYAXISMOTION;
	event.jaxis.which = 0;
	event.jaxis.axis = 1;
	event.jaxis.value = 16384;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick axis motion event",
		string("joystick axis motion: joystick ID: 0, axis ID: 1, value: 0.5"), string_cast(event));

	event.type = SDL_JOYBALLMOTION;
	event.jball.which = 2;
	event.jball.ball = 3;
	event.jball.xrel = 4;
	event.jball.yrel = 5;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick ball motion event",
		string("joystick ball motion: joystick ID: 2, ball ID: 3, delta: 4 5"), string_cast(event));

	event.type = SDL_JOYHATMOTION;
	event.jhat.which = 6;
	event.jhat.hat = 7;
	event.jhat.value = SDL_HAT_LEFTUP;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick hat motion event",
		string("joystick hat motion: joystick ID: 6, hat ID: 7, value: left up"), string_cast(event));
	event.jhat.value = SDL_HAT_UP;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick hat motion event",
		string("joystick hat motion: joystick ID: 6, hat ID: 7, value: up"), string_cast(event));
	event.jhat.value = SDL_HAT_RIGHTUP;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick hat motion event",
		string("joystick hat motion: joystick ID: 6, hat ID: 7, value: right up"), string_cast(event));
	event.jhat.value = SDL_HAT_LEFT;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick hat motion event",
		string("joystick hat motion: joystick ID: 6, hat ID: 7, value: left"), string_cast(event));
	event.jhat.value = SDL_HAT_CENTERED;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick hat motion event",
		string("joystick hat motion: joystick ID: 6, hat ID: 7, value: center"), string_cast(event));
	event.jhat.value = SDL_HAT_RIGHT;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick hat motion event",
		string("joystick hat motion: joystick ID: 6, hat ID: 7, value: right"), string_cast(event));
	event.jhat.value = SDL_HAT_LEFTDOWN;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick hat motion event",
		string("joystick hat motion: joystick ID: 6, hat ID: 7, value: left down"), string_cast(event));
	event.jhat.value = SDL_HAT_DOWN;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick hat motion event",
		string("joystick hat motion: joystick ID: 6, hat ID: 7, value: down"), string_cast(event));
	event.jhat.value = SDL_HAT_RIGHTDOWN;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick hat motion event",
		string("joystick hat motion: joystick ID: 6, hat ID: 7, value: right down"), string_cast(event));
	event.jhat.value = -1;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick hat motion event",
		string("joystick hat motion: joystick ID: 6, hat ID: 7, value: unknown"), string_cast(event));

	event.type = SDL_JOYBUTTONDOWN;
	event.jbutton.which = 8;
	event.jbutton.button = 9;
	event.jbutton.state = SDL_PRESSED;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick button down event",
		string("joystick button down: joystick ID: 8, button ID: 9, state: pressed"), string_cast(event));
	event.type = SDL_JOYBUTTONUP;
	event.jbutton.which = 10;
	event.jbutton.button = 11;
	event.jbutton.state = SDL_RELEASED;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick button up event",
		string("joystick button up: joystick ID: 10, button ID: 11, state: released"), string_cast(event));

	event.type = SDL_JOYDEVICEADDED;
	event.jdevice.which = 12;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick device added event",
		string("joystick device added: joystick ID: 12"), string_cast(event));
	event.type = SDL_JOYDEVICEREMOVED;
	event.jdevice.which = 13;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL joystick device removed event",
		string("joystick device removed: joystick ID: 13"), string_cast(event));
}

void EventTest::testMouse() {
	SDL_Event event;
	event.type = SDL_MOUSEMOTION;
	event.motion.windowID = 0;
	event.motion.which = 1;
	event.motion.x = 2;
	event.motion.y = 3;
	event.motion.xrel = 4;
	event.motion.yrel = 5;
	event.motion.state = 0;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL mouse motion event",
		string("mouse motion: window ID: 0, mouse ID: 1, position: 2 3, delta: 4 5"), string_cast(event));
	event.motion.windowID = 6;
	event.motion.which = 7;
	event.motion.x = 8;
	event.motion.y = 9;
	event.motion.xrel = 10;
	event.motion.yrel = 11;
	event.motion.state = SDL_BUTTON_LMASK | SDL_BUTTON_MMASK | SDL_BUTTON_RMASK;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL mouse motion event",
		string("mouse motion: window ID: 6, mouse ID: 7, position: 8 9, delta: 10 11, buttons: left middle right"), string_cast(event));
	event.motion.state = SDL_BUTTON_X1MASK | SDL_BUTTON_X2MASK;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL mouse motion event",
		string("mouse motion: window ID: 6, mouse ID: 7, position: 8 9, delta: 10 11, buttons: X1 X2"), string_cast(event));

	event.type = SDL_MOUSEBUTTONDOWN;
	event.button.windowID = 0;
	event.button.which = 1;
	event.button.button = SDL_BUTTON_LEFT;
	event.button.state = SDL_PRESSED;
	event.button.clicks = 2;
	event.button.x = 3;
	event.button.y = 4;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL mouse button down event",
		string("mouse button down: window ID: 0, mouse ID: 1, button: left, state: pressed, clicks: 2, position: 3 4"), string_cast(event));
	event.type = SDL_MOUSEBUTTONUP;
	event.button.windowID = 5;
	event.button.which = 6;
	event.button.button = SDL_BUTTON_MIDDLE;
	event.button.clicks = 7;
	event.button.x = 8;
	event.button.y = 9;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL mouse button up event",
		string("mouse button up: window ID: 5, mouse ID: 6, button: middle, state: pressed, clicks: 7, position: 8 9"), string_cast(event));
	event.button.button = SDL_BUTTON_RIGHT;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL mouse button up event",
		string("mouse button up: window ID: 5, mouse ID: 6, button: right, state: pressed, clicks: 7, position: 8 9"), string_cast(event));
	event.button.button = SDL_BUTTON_X1;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL mouse button up event",
		string("mouse button up: window ID: 5, mouse ID: 6, button: X1, state: pressed, clicks: 7, position: 8 9"), string_cast(event));
	event.button.button = SDL_BUTTON_X2;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL mouse button up event",
		string("mouse button up: window ID: 5, mouse ID: 6, button: X2, state: pressed, clicks: 7, position: 8 9"), string_cast(event));
	event.button.button = SDL_BUTTON_X2 + 1;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL mouse button up event",
		string("mouse button up: window ID: 5, mouse ID: 6, button: ") + to_string(int(SDL_BUTTON_X2 + 1)) + ", state: pressed, clicks: 7, position: 8 9", string_cast(event));

	event.type = SDL_MOUSEWHEEL;
	event.wheel.windowID = 0;
	event.wheel.which = 1;
	event.wheel.x = 2;
	event.wheel.y = 3;
#if SDL_VERSION_ATLEAST(2, 0, 4)
	event.wheel.direction = SDL_MOUSEWHEEL_NORMAL;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL mouse wheel event",
		string("mouse wheel: window ID: 0, mouse ID: 1, delta: 2 3, direction: normal"), string_cast(event));
	event.wheel.windowID = 4;
	event.wheel.which = 5;
	event.wheel.x = 6;
	event.wheel.y = 7;
	event.wheel.direction = SDL_MOUSEWHEEL_FLIPPED;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL mouse wheel event",
		string("mouse wheel: window ID: 4, mouse ID: 5, delta: 6 7, direction: flipped"), string_cast(event));
#else
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL mouse wheel event",
		string("mouse wheel: window ID: 0, mouse ID: 1, delta: 2 3"), string_cast(event));
#endif
}

void EventTest::testMultiGesture() {
	SDL_Event event;
	event.type = SDL_MULTIGESTURE;
	event.mgesture.touchId = 0;
	event.mgesture.dTheta = 1;
	event.mgesture.dDist = 2;
	event.mgesture.x = 3;
	event.mgesture.y = 4;
	event.mgesture.numFingers = 5;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL multi gesture event",
		string("multi gesture: device ID: 0, theta: 1, distance: 2, position: 3 4, fingers: 5"), string_cast(event));
}

void EventTest::testQuit() {
	SDL_Event event;
	event.type = SDL_QUIT;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL quit event",
		string("quit: quit"), string_cast(event));
}

void EventTest::testSysWM() {
	SDL_Event event;
	event.type = SDL_SYSWMEVENT;
	event.syswm.msg = nullptr;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL sys wm event",
		string("sys wm: without message"), string_cast(event));
	SDL_SysWMmsg msg;
	event.syswm.msg = &msg;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL sys wm event",
		string("sys wm: with message"), string_cast(event));
}

void EventTest::testText() {
	SDL_Event event;
	event.type = SDL_TEXTEDITING;
	event.edit.windowID = 0;
	event.edit.text[0] = '\303';
	event.edit.text[1] = '\244';
	event.edit.text[2] = '\0';
	event.edit.start = 1;
	event.edit.length = 2;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL text editing event",
		string("text editing: window ID: 0, text: \"ä\", start: 1, length: 2"), string_cast(event));

	event.type = SDL_TEXTINPUT;
	event.text.windowID = 3;
	event.text.text[0] = '\0';
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL text input event",
		string("text input: window ID: 3, text: \"\""), string_cast(event));
}

void EventTest::testUser() {
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.windowID = 0;
	event.user.code = 1;
	event.user.data1 = nullptr;
	event.user.data2 = reinterpret_cast<void *>(1);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL user event",
		string("user: window ID: 0, code: 1, data 1: 0, data 2: 0x1"), string_cast(event));
}

void EventTest::testWindow() {
	SDL_Event event;
	event.type = SDL_WINDOWEVENT;
	event.window.event = SDL_WINDOWEVENT_SHOWN;
	event.window.windowID = 0;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: shown, window ID: 0"), string_cast(event));

	event.window.event = SDL_WINDOWEVENT_HIDDEN;
	event.window.windowID = 1;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: hidden, window ID: 1"), string_cast(event));

	event.window.event = SDL_WINDOWEVENT_EXPOSED;
	event.window.windowID = 2;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: exposed, window ID: 2"), string_cast(event));

	event.window.event = SDL_WINDOWEVENT_MOVED;
	event.window.windowID = 3;
	event.window.data1 = 4;
	event.window.data2 = 5;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: moved, window ID: 3, position: 4 5"), string_cast(event));

	event.window.event = SDL_WINDOWEVENT_RESIZED;
	event.window.windowID = 6;
	event.window.data1 = 7;
	event.window.data2 = 8;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: resized, window ID: 6, size: 7x8"), string_cast(event));

	event.window.event = SDL_WINDOWEVENT_SIZE_CHANGED;
	event.window.windowID = 9;
	event.window.data1 = 10;
	event.window.data2 = 11;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: size changed, window ID: 9, size: 10x11"), string_cast(event));

	event.window.event = SDL_WINDOWEVENT_MINIMIZED;
	event.window.windowID = 12;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: minimized, window ID: 12"), string_cast(event));

	event.window.event = SDL_WINDOWEVENT_MAXIMIZED;
	event.window.windowID = 13;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: maximized, window ID: 13"), string_cast(event));

	event.window.event = SDL_WINDOWEVENT_RESTORED;
	event.window.windowID = 14;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: restored, window ID: 14"), string_cast(event));

	event.window.event = SDL_WINDOWEVENT_ENTER;
	event.window.windowID = 15;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: mouse entered, window ID: 15"), string_cast(event));

	event.window.event = SDL_WINDOWEVENT_LEAVE;
	event.window.windowID = 16;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: mouse left, window ID: 16"), string_cast(event));

	event.window.event = SDL_WINDOWEVENT_FOCUS_GAINED;
	event.window.windowID = 17;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: focus gained, window ID: 17"), string_cast(event));

	event.window.event = SDL_WINDOWEVENT_FOCUS_LOST;
	event.window.windowID = 18;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: focus lost, window ID: 18"), string_cast(event));

	event.window.event = SDL_WINDOWEVENT_CLOSE;
	event.window.windowID = 19;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: closed, window ID: 19"), string_cast(event));

	event.window.event = SDL_WINDOWEVENT_NONE;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL window event",
		string("window: unknown"), string_cast(event));
}

void EventTest::testUnknown() {
	SDL_Event event;
	// SDL_LASTEVENT holds the number of entries in the enum and therefore
	// shouldn't be recognized as any valid event type
	event.type = SDL_LASTEVENT;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(
		"output format of SDL user event",
		string("unknown"), string_cast(event));
}

}
}
