#ifndef BLANK_CAMERA_HPP_
#define BLANK_CAMERA_HPP_

#include <glm/glm.hpp>

#include "model.hpp"


namespace blank {

class Camera {

public:
	Camera();
	~Camera();

	Camera(const Camera &) = delete;
	Camera &operator =(const Camera &) = delete;

	glm::mat4 MakeMVP(const glm::mat4 &m) const { return vp * m; }

	void Viewport(int width, int height);
	void Viewport(int x, int y, int width, int height);

	void FOV(float f);
	void Aspect(float r);
	void Aspect(float w, float h);
	void Clip(float near, float far);

	void Position(glm::vec3 pos) { model.Position(pos); UpdateView(); }
	void Move(glm::vec3 delta) { model.Move(delta); UpdateView(); }

	// all angles in radians (full circle = 2π)
	float Pitch() const { return model.Pitch(); }
	void Pitch(float p) { model.Pitch(p); UpdateView(); }
	void RotatePitch(float delta) { model.RotatePitch(delta); UpdateView(); }
	float Yaw() const { return model.Yaw(); }
	void Yaw(float y) { model.Yaw(y); UpdateView(); }
	void RotateYaw(float delta) { model.RotateYaw(delta); UpdateView(); }

private:
	void UpdateProjection();
	void UpdateView();

private:
	float fov;
	float aspect;
	float near_clip;
	float far_clip;

	Model model;

	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 vp;

};

}

#endif
