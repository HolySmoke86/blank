#include "Camera.hpp"
#include "Canvas.hpp"
#include "SkyBox.hpp"
#include "Viewport.hpp"

#include "../app/init.hpp"
#include "../model/geometry.hpp"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <SDL.h>


namespace blank {

Camera::Camera() noexcept
: fov(PI_0p25)
, aspect(1.0f)
, near(0.1f)
, far(256.0f)
, projection(glm::perspective(fov, aspect, near, far))
, view(1.0f) {

}


void Camera::FOV(float f) noexcept {
	fov = f;
	UpdateProjection();
}

void Camera::Aspect(float r) noexcept {
	aspect = r;
	UpdateProjection();
}

void Camera::Aspect(float w, float h) noexcept {
	Aspect(w / h);
}

void Camera::Clip(float n, float f) noexcept {
	near = n;
	far = f;
	UpdateProjection();
}

void Camera::View(const glm::mat4 &v) noexcept {
	view = v;
}

void Camera::UpdateProjection() noexcept {
	projection = glm::perspective(fov, aspect, near, far);
}


Canvas::Canvas() noexcept
: offset(0.0f, 0.0f)
, size(1.0f, 1.0f)
, near(100.0f)
, far(-100.0f)
, projection(glm::ortho(offset.x, size.x, size.y, offset.y, near, far))
, view(1.0f) {

}


void Canvas::Resize(float w, float h) noexcept {
	size.x = w;
	size.y = h;
	UpdateProjection();
}


void Canvas::UpdateProjection() noexcept {
	projection = glm::ortho(offset.x, size.x, size.y, offset.y, near, far);
}


SkyBox::SkyBox(CubeMap &&tex)
: texture(std::move(tex))
, mesh() {
	mesh.LoadUnitBox();
}

void SkyBox::Render(Viewport &viewport) noexcept {
	SkyBoxShader &prog = viewport.SkyBoxProgram();
	prog.SetTexture(texture);
	mesh.Draw();
}


Viewport::Viewport()
: cam()
, canv()
, cursor(1.0f)
, chunk_prog()
, entity_prog()
, sky_prog()
, sprite_prog()
, active_prog(NONE) {
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

void Viewport::VSync(bool b) noexcept {
	if (SDL_GL_SetSwapInterval(b) != 0) {
		throw SDLError("SDL_GL_SetSwapInterval");
	}
}

void Viewport::EnableDepthTest() noexcept {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void Viewport::EqualDepthTest() noexcept {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

void Viewport::DisableDepthTest() noexcept {
	glDisable(GL_DEPTH_TEST);
}

void Viewport::EnableBackfaceCulling() noexcept {
	glEnable(GL_CULL_FACE);
}

void Viewport::DisableBackfaceCulling() noexcept {
	glDisable(GL_CULL_FACE);
}

void Viewport::EnableAlphaBlending() noexcept {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Viewport::EnableInvertBlending() noexcept {
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
}

void Viewport::DisableBlending() noexcept {
	glDisable(GL_BLEND);
}

void Viewport::Resize(int w, int h) noexcept {
	glViewport(0, 0, w, h);
	float fw = w;
	float fh = h;
	cam.Aspect(fw, fh);
	canv.Resize(fw, fh);

	ChunkProgram().SetProjection(Perspective());
	SkyBoxProgram().SetProjection(Perspective());
	SpriteProgram().SetProjection(Ortho());
}

void Viewport::Clear() noexcept {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Viewport::ClearDepth() noexcept {
	glClear(GL_DEPTH_BUFFER_BIT);
}


glm::vec2 Viewport::GetPosition(const glm::vec2 &off, Gravity grav) const noexcept {
	return align(grav, canv.Size(), off + canv.Offset());
}

void Viewport::SetCursor(const glm::vec3 &pos) noexcept {
	cursor[3].x = pos.x;
	cursor[3].y = pos.y;
	cursor[3].z = pos.z;
}

void Viewport::SetCursor(const glm::vec3 &pos, Gravity grav) noexcept {
	glm::vec2 p(GetPosition(glm::vec2(pos), grav));
	cursor[3].x = p.x;
	cursor[3].y = p.y;
	cursor[3].z = pos.z;
}

void Viewport::MoveCursor(const glm::vec3 &d) noexcept {
	cursor[3].x += d.x;
	cursor[3].y += d.y;
	cursor[3].z += d.z;
}


BlockLighting &Viewport::ChunkProgram() noexcept {
	if (active_prog != CHUNK) {
		chunk_prog.Activate();
		EnableDepthTest();
		EnableBackfaceCulling();
		DisableBlending();
		active_prog = CHUNK;
	}
	return chunk_prog;
}

DirectionalLighting &Viewport::EntityProgram() noexcept {
	if (active_prog != ENTITY) {
		entity_prog.Activate();
		EnableDepthTest();
		EnableBackfaceCulling();
		DisableBlending();
		entity_prog.SetVP(cam.View(), cam.Projection());
		active_prog = ENTITY;
	}
	return entity_prog;
}

DirectionalLighting &Viewport::HUDProgram() noexcept {
	if (active_prog != HUD) {
		entity_prog.Activate();
		EnableDepthTest();
		EnableBackfaceCulling();
		entity_prog.SetVP(canv.View(), canv.Projection());
		active_prog = HUD;
	}
	return entity_prog;
}

PlainColor &Viewport::WorldColorProgram() noexcept {
	if (active_prog != COLOR_WORLD) {
		color_prog.Activate();
		color_prog.SetVP(cam.View(), cam.Projection());
		active_prog = COLOR_WORLD;
	}
	return color_prog;
}

PlainColor &Viewport::HUDColorProgram() noexcept {
	if (active_prog != COLOR_HUD) {
		color_prog.Activate();
		color_prog.SetVP(canv.View(), canv.Projection());
		active_prog = COLOR_HUD;
	}
	return color_prog;
}

SkyBoxShader &Viewport::SkyBoxProgram() noexcept {
	if (active_prog != SKY_BOX) {
		sky_prog.Activate();
		DisableBlending();
		DisableBackfaceCulling();
		EqualDepthTest();
		active_prog = SKY_BOX;
	}
	return sky_prog;
}

BlendedSprite &Viewport::SpriteProgram() noexcept {
	if (active_prog != SPRITE) {
		sprite_prog.Activate();
		EnableAlphaBlending();
		active_prog = SPRITE;
	}
	return sprite_prog;
}


void Viewport::WorldPosition(const glm::mat4 &t) noexcept {
	const glm::vec3 offset(0.0f, 0.0f, 0.0f);
	//const glm::vec3 offset(0.0f, 0.0f, -5.0f);
	cam.View(glm::translate(glm::inverse(t), glm::vec3(t * glm::vec4(offset, 0.0f))));
	ChunkProgram().SetView(cam.View());
	sky_prog.Activate();
	SkyBoxProgram().SetView(cam.View());
}

}
