#ifndef BLANK_MODEL_SKELETONS_HPP_
#define BLANK_MODEL_SKELETONS_HPP_

#include <cstdint>
#include <memory>
#include <vector>


namespace blank {

class CompositeModel;
class EntityMesh;

class Skeletons {

public:
	using size_type = std::size_t;
	using reference = CompositeModel &;
	using const_reference = const CompositeModel &;

public:
	Skeletons();
	~Skeletons();

	void LoadHeadless();
	void Load();

	size_type size() const noexcept { return skeletons.size(); }

	reference operator[](size_type i) noexcept { return *skeletons[i]; }
	const_reference operator[](size_type i) const noexcept { return *skeletons[i]; }

	CompositeModel *ByID(std::uint16_t) noexcept;
	const CompositeModel *ByID(std::uint16_t) const noexcept;

private:
	std::vector<std::unique_ptr<CompositeModel>> skeletons;
	std::vector<EntityMesh> meshes;

};

}

#endif
