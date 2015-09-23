#ifndef BLANK_GRAPHICS_SKYBOX_HPP_
#define BLANK_GRAPHICS_SKYBOX_HPP_

#include "CubeMap.hpp"
#include "../model/SkyBoxModel.hpp"


namespace blank {

class Viewport;

class SkyBox {

public:
	explicit SkyBox(CubeMap &&);

	void Render(Viewport &) noexcept;

private:
	CubeMap texture;
	SkyBoxModel model;

};

}

#endif
