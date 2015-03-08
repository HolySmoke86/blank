#ifndef BLANK_SHAPE_HPP_
#define BLANK_SHAPE_HPP_

#include "geometry.hpp"

#include <vector>
#include <glm/glm.hpp>


namespace blank {

struct Shape {

	virtual size_t VertexCount() const = 0;
	virtual void Vertices(std::vector<glm::vec3> &, const glm::vec3 &pos = { 0.0f, 0.0f, 0.0f }) const = 0;
	virtual void Normals(std::vector<glm::vec3> &) const = 0;

	virtual size_t OutlineCount() const = 0;
	virtual void Outline(std::vector<glm::vec3> &, const glm::vec3 &pos = { 0.0f, 0.0f, 0.0f }) const = 0;

	virtual bool Intersects(const Ray &, const glm::mat4 &, float &dist, glm::vec3 &normal) const = 0;

};


class CuboidShape
: public Shape {

public:
	CuboidShape(const AABB &bounds);

	size_t VertexCount() const override;
	void Vertices(std::vector<glm::vec3> &, const glm::vec3 &) const override;
	void Normals(std::vector<glm::vec3> &) const override;

	size_t OutlineCount() const override;
	void Outline(std::vector<glm::vec3> &, const glm::vec3 &) const override;

	bool Intersects(const Ray &, const glm::mat4 &, float &, glm::vec3 &) const override;

private:
	AABB bb;

};


class StairShape
: public Shape {

public:
	StairShape(const AABB &bounds, const glm::vec2 &clip);

	size_t VertexCount() const override;
	void Vertices(std::vector<glm::vec3> &, const glm::vec3 &) const override;
	void Normals(std::vector<glm::vec3> &) const override;

	size_t OutlineCount() const override;
	void Outline(std::vector<glm::vec3> &, const glm::vec3 &) const override;

	bool Intersects(const Ray &, const glm::mat4 &, float &, glm::vec3 &) const override;

private:
	AABB top, bot;

};

}

#endif
