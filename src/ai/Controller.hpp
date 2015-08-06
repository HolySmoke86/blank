#ifndef BLANK_AI_CONTROLLER_HPP_
#define BLANK_AI_CONTROLLER_HPP_


namespace blank {

class Entity;

class Controller {

public:
	explicit Controller(Entity &e) noexcept;
	virtual ~Controller();

	Entity &Controlled() noexcept { return entity; }
	const Entity &Controlled() const noexcept { return entity; }

	virtual void Update(int dt) = 0;

private:
	Entity &entity;

};

}

#endif
