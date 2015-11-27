#ifndef BLANK_WORLD_ENTITYCONTROLLER_HPP_
#define BLANK_WORLD_ENTITYCONTROLLER_HPP_

namespace blank {

class Entity;

struct EntityController {

	virtual ~EntityController();

	virtual void Update(Entity &, float dt) = 0;

};

}

#endif
