#ifndef BLANK_MODEL_INSTANCE_HPP_
#define BLANK_MODEL_INSTANCE_HPP_

#include "Part.hpp"

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class Model;
class DirectionalLighting;

class Instance {

	friend class Model;

public:
	Instance();

	operator bool() const noexcept { return model; }
	const Model &GetModel() const noexcept { return *model; }

	void Render(const glm::mat4 &, DirectionalLighting &) const;

private:
	const Model *model;
	std::vector<Part::State> state;

};

}

#endif
