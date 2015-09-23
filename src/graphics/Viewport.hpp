#ifndef BLANK_GRAPHICS_VIEWPORT_HPP_
#define BLANK_GRAPHICS_VIEWPORT_HPP_

#include "align.hpp"
#include "BlendedSprite.hpp"
#include "BlockLighting.hpp"
#include "Camera.hpp"
#include "Canvas.hpp"
#include "DirectionalLighting.hpp"
#include "PlainColor.hpp"
#include "SkyBoxShader.hpp"

#include <glm/glm.hpp>


namespace blank {

class Viewport {

public:
	Viewport();

	Viewport(const Viewport &) = delete;
	Viewport &operator =(const Viewport &) = delete;

	void VSync(bool b) noexcept;

	void EnableDepthTest() noexcept;
	void EqualDepthTest() noexcept;
	void DisableDepthTest() noexcept;

	void EnableBackfaceCulling() noexcept;
	void DisableBackfaceCulling() noexcept;

	void EnableAlphaBlending() noexcept;
	void EnableInvertBlending() noexcept;
	void DisableBlending() noexcept;

	void Resize(int w, int h) noexcept;

	float Width() const noexcept { return canv.Size().x; }
	float Height() const noexcept { return canv.Size().y; }

	void Clear() noexcept;
	void ClearDepth() noexcept;

	void SetCursor(const glm::vec3 &);
	void SetCursor(const glm::vec3 &, Gravity);
	void MoveCursor(const glm::vec3 &);
	const glm::mat4 &Cursor() const noexcept { return cursor; }

	BlockLighting &ChunkProgram() noexcept;
	DirectionalLighting &EntityProgram() noexcept;
	DirectionalLighting &HUDProgram() noexcept;
	PlainColor &WorldOutlineProgram() noexcept;
	PlainColor &HUDOutlineProgram() noexcept;
	SkyBoxShader &SkyBoxProgram() noexcept;
	BlendedSprite &SpriteProgram() noexcept;

	void WorldPosition(const glm::mat4 &) noexcept;

	const glm::mat4 &Perspective() const noexcept { return cam.Projection(); }
	const glm::mat4 &Ortho() const noexcept { return canv.Projection(); }

private:
	Camera cam;
	Canvas canv;

	glm::mat4 cursor;

	BlockLighting chunk_prog;
	DirectionalLighting entity_prog;
	PlainColor outline_prog;
	SkyBoxShader sky_prog;
	BlendedSprite sprite_prog;

	enum {
		NONE,
		CHUNK,
		ENTITY,
		HUD,
		OUTLINE_WORLD,
		OUTLINE_HUD,
		SKY_BOX,
		SPRITE,
	} active_prog;

};

}

#endif
