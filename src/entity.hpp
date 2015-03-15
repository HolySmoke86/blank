#ifndef BLANK_ENTITY_HPP_
#define BLANK_ENTITY_HPP_

#include "block.hpp"
#include "chunk.hpp"
#include "geometry.hpp"
#include "model.hpp"
#include "shape.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


namespace blank {

class Shape;

class Entity {

public:
	Entity();

	bool HasShape() const { return shape; }
	const Shape *GetShape() const { return shape; }
	void SetShape(Shape *, const glm::vec3 &color);
	void SetShapeless();

	const glm::vec3 &Velocity() const { return velocity; }
	void Velocity(const glm::vec3 &);

	const Block::Pos &Position() const { return position; }
	void Position(const Block::Pos &);
	void Move(const glm::vec3 &delta);

	const Chunk::Pos ChunkCoords() const { return chunk; }

	const glm::quat &AngularVelocity() const { return angular_velocity; }
	void AngularVelocity(const glm::quat &);

	const glm::mat4 &Rotation() const { return rotation; }
	void Rotation(const glm::mat4 &);
	void Rotate(const glm::quat &delta);

	glm::mat4 Transform(const Chunk::Pos &chunk_offset) const;
	Ray Aim(const Chunk::Pos &chunk_offset) const;

	void Update(int dt);

	void Draw();

private:
	Shape *shape;
	Model model;

	glm::vec3 velocity;
	Block::Pos position;
	Chunk::Pos chunk;

	glm::quat angular_velocity;
	glm::mat4 rotation;

};

}

#endif
