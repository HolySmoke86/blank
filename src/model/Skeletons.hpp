#ifndef BLANK_MODEL_SKELETONS_HPP_
#define BLANK_MODEL_SKELETONS_HPP_

#include <cstdint>
#include <memory>
#include <vector>


namespace blank {

class CompositeModel;
class EntityModel;

class Skeletons {

public:
	Skeletons();
	~Skeletons();

	void LoadHeadless();
	void Load();

	std::size_t Size() const noexcept { return skeletons.size(); }

	CompositeModel &operator[](std::size_t i) noexcept { return *skeletons[i]; }
	const CompositeModel &operator[](std::size_t i) const noexcept { return *skeletons[i]; }

	CompositeModel *ByID(std::uint16_t) noexcept;
	const CompositeModel *ByID(std::uint16_t) const noexcept;

private:
	std::vector<std::unique_ptr<CompositeModel>> skeletons;
	std::vector<EntityModel> models;

};

}

#endif
