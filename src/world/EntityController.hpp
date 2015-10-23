#ifndef BLANK_WORLD_ENTITYCONTROLLER_HPP_
#define BLANK_WORLD_ENTITYCONTROLLER_HPP_

#include <glm/glm.hpp>


namespace blank {

class Entity;
class EntityState;

struct EntityController {

	virtual ~EntityController();

	virtual void Update(Entity &, float dt) = 0;

	virtual glm::vec3 ControlForce(const EntityState &) const = 0;

};

}

#endif
