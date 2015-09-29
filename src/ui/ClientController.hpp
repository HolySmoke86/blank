#ifndef BLANK_UI_CLIENTCONTROLLER_HPP_
#define BLANK_UI_CLIENTCONTROLLER_HPP_


namespace blank {

struct ClientController {

	virtual void SetAudio(bool) = 0;
	virtual void SetVideo(bool) = 0;
	virtual void SetHUD(bool) = 0;
	virtual void SetDebug(bool) = 0;

	virtual void Exit() = 0;

};

}

#endif
