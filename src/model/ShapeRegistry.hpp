#ifndef BLANK_MODEL_SHAPEREGISTRY_HPP_
#define BLANK_MODEL_SHAPEREGISTRY_HPP_

#include "Shape.hpp"

#include <map>
#include <string>


namespace blank {

class ShapeRegistry {

public:
	ShapeRegistry();

	Shape &Add(const std::string &);

	Shape &Get(const std::string &);
	const Shape &Get(const std::string &) const;

private:
	std::map<std::string, Shape> shapes;

};

}

#endif
