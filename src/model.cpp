#include "model.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>


namespace blank {

Model::Model()
: velocity(0, 0, 0)
, position(0, 0, 0)
, pitch(0)
, yaw(0) {

}

Model::~Model() {

}


glm::mat4 Model::Transform() const {
	return glm::translate(position) * glm::eulerAngleYX(yaw, pitch);
}


void Model::Update(int dt) {
	position += velocity * float(dt);
}

}
