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

	glm::vec2 GetPosition(const glm::vec2 &off, Gravity grav) const noexcept;

	void SetCursor(const glm::vec3 &) noexcept;
	void SetCursor(const glm::vec3 &, Gravity) noexcept;
	void MoveCursor(const glm::vec3 &) noexcept;
	const glm::mat4 &Cursor() const noexcept { return cursor; }

	void OffsetCamera(const glm::vec3 &o) noexcept { cam_offset = o; }
	const glm::vec3 &CameraOffset() const noexcept { return cam_offset; }

	BlockLighting &ChunkProgram() noexcept;
	DirectionalLighting &EntityProgram() noexcept;
	DirectionalLighting &HUDProgram() noexcept;
	PlainColor &WorldColorProgram() noexcept;
	PlainColor &HUDColorProgram() noexcept;
	SkyBoxShader &SkyBoxProgram() noexcept;
	BlendedSprite &SpriteProgram() noexcept;

	void WorldPosition(const glm::mat4 &) noexcept;

	const glm::mat4 &Perspective() const noexcept { return cam.Projection(); }
	const glm::mat4 &Ortho() const noexcept { return canv.Projection(); }

private:
	Camera cam;
	Canvas canv;

	glm::mat4 cursor;

	glm::vec3 cam_offset;

	BlockLighting chunk_prog;
	DirectionalLighting entity_prog;
	PlainColor color_prog;
	SkyBoxShader sky_prog;
	BlendedSprite sprite_prog;

	enum {
		NONE,
		CHUNK,
		ENTITY,
		HUD,
		COLOR_WORLD,
		COLOR_HUD,
		SKY_BOX,
		SPRITE,
	} active_prog;

};

}

#endif
