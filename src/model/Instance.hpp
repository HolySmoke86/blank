#ifndef BLANK_MODEL_INSTANCE_HPP_
#define BLANK_MODEL_INSTANCE_HPP_

#include "Part.hpp"

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class DirectionalLighting;
class EntityMesh;
class Model;
class Part;

class Instance {

	friend class Model;
	friend class Part;

public:
	Instance();
	~Instance();

	Instance(const Instance &);
	Instance &operator =(const Instance &);

	operator bool() const noexcept { return model; }
	const Model &GetModel() const noexcept { return *model; }

	void Render(const glm::mat4 &, DirectionalLighting &);

	void SetTextures(const std::vector<float> &t);
	void SetHSLModifier(const glm::vec3 &m);
	void SetRGBModifier(const glm::vec3 &m);

private:
	const Model *model;
	std::vector<Part::State> state;
	std::vector<std::unique_ptr<EntityMesh>> mesh;

	std::vector<float> tex_map;
	glm::vec3 hsl_mod;
	glm::vec3 rgb_mod;

};

}

#endif
