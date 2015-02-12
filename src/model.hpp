#ifndef BLANK_MODEL_HPP_
#define BLANK_MODEL_HPP_

#include <glm/glm.hpp>


namespace blank {

class Model {

public:
	Model();
	~Model();

	glm::mat4 Transform() const;

	void Position(glm::vec3 pos) { position = pos; }
	void Move(glm::vec3 delta) { position += delta; }

	// all angles in radians (full circle = 2π)
	void Pitch(float p) { pitch = p; }
	void RotatePitch(float delta) { pitch += delta; }
	void Yaw(float y) { yaw = y; }
	void RotateYaw(float delta) { yaw += delta; }

private:
	glm::vec3 position;
	float pitch;
	float yaw;

};

}

#endif
