#ifndef BLANK_MODEL_INSTANCE_HPP_
#define BLANK_MODEL_INSTANCE_HPP_

#include "Part.hpp"

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class DirectionalLighting;
class Model;
class Part;

class Instance {

	friend class Model;
	friend class Part;

public:
	Instance();
	~Instance();

	operator bool() const noexcept { return model; }
	const Model &GetModel() const noexcept { return *model; }

	glm::mat4 EyesTransform() const noexcept;
	Part::State &EyesState() noexcept;

	void Render(const glm::mat4 &, DirectionalLighting &);

private:
	const Model *model;
	std::vector<Part::State> state;

};

}

#endif
