#ifndef BLANK_UI_CLIENTCONTROLLER_HPP_
#define BLANK_UI_CLIENTCONTROLLER_HPP_


namespace blank {

struct ClientController {

	/// enable or disable audio output
	virtual void SetAudio(bool) = 0;
	/// enable or disable world rendering
	virtual void SetVideo(bool) = 0;
	/// enable or disable HUD rendering
	virtual void SetHUD(bool) = 0;
	/// enable or disable debug rendering
	virtual void SetDebug(bool) = 0;

	/// change camera mode of world rendering
	virtual void NextCamera() = 0;

	/// terminate the application
	virtual void Exit() = 0;

};

}

#endif
