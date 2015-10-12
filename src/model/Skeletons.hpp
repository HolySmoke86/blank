#ifndef BLANK_MODEL_SKELETONS_HPP_
#define BLANK_MODEL_SKELETONS_HPP_

#include <cstdint>
#include <memory>
#include <vector>


namespace blank {

class Model;
class EntityMesh;

class Skeletons {

public:
	using size_type = std::size_t;
	using reference = Model &;
	using const_reference = const Model &;

public:
	Skeletons();
	~Skeletons();

	void LoadHeadless();
	void Load();

	size_type size() const noexcept { return skeletons.size(); }

	reference operator[](size_type i) noexcept { return *skeletons[i]; }
	const_reference operator[](size_type i) const noexcept { return *skeletons[i]; }

	Model *ByID(std::uint16_t) noexcept;
	const Model *ByID(std::uint16_t) const noexcept;

private:
	std::vector<std::unique_ptr<Model>> skeletons;
	std::vector<EntityMesh> meshes;

};

}

#endif
