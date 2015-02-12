#ifndef BLANK_CAMERA_HPP_
#define BLANK_CAMERA_HPP_

#include <glm/glm.hpp>


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

	void Position(glm::vec3 pos) { position = pos; UpdateView(); }
	void Move(glm::vec3 delta) { position += delta; UpdateView(); }

	void LookAt(glm::vec3 tgt) { target = tgt; UpdateView(); }

private:
	void UpdateProjection();
	void UpdateView();

private:
	float fov;
	float aspect;
	float near_clip;
	float far_clip;

	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 up;

	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 vp;

};

}

#endif
