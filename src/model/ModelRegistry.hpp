#ifndef BLANK_MODEL_MODELREGISTRY_HPP_
#define BLANK_MODEL_MODELREGISTRY_HPP_

#include "Model.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>


namespace blank {

class ModelRegistry {

public:
	using size_type = std::size_t;
	using reference = Model &;
	using const_reference = const Model &;

public:
	ModelRegistry();

	reference Add(const std::string &);

	size_type size() const noexcept { return models.size(); }

	// by offset
	reference operator[](size_type i) noexcept { return *models[i]; }
	const_reference operator[](size_type i) const noexcept { return *models[i]; }
	// by ID
	reference Get(std::uint16_t i) { return *models[i - 1]; }
	const_reference Get(std::uint16_t i) const { return *models[i - 1]; }
	// by name
	reference Get(const std::string &);
	const_reference Get(const std::string &) const;

private:
	std::vector<std::unique_ptr<Model>> models;
	std::map<std::string, Model *> name_index;

};

}

#endif
