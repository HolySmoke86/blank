#include "shape.hpp"


namespace blank {

CuboidShape::CuboidShape(const AABB &b)
: Shape()
, bb(b) {
	bb.Adjust();
}


size_t CuboidShape::VertexCount() const {
	return 36;
}

void CuboidShape::Vertices(std::vector<glm::vec3> &out, const glm::vec3 &pos) const {
	out.reserve(36);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z); // front
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z); // back
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z); // top
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z); // bottom
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z); // left
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z); // right
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);
}

void CuboidShape::Normals(std::vector<glm::vec3> &out) const {
	out.reserve(36);
	out.insert(out.end(), 6, glm::vec3( 0.0f,  0.0f,  1.0f)); // front
	out.insert(out.end(), 6, glm::vec3( 0.0f,  0.0f, -1.0f)); // back
	out.insert(out.end(), 6, glm::vec3( 0.0f,  1.0f,  0.0f)); // top
	out.insert(out.end(), 6, glm::vec3( 0.0f, -1.0f,  0.0f)); // bottom
	out.insert(out.end(), 6, glm::vec3(-1.0f,  0.0f,  0.0f)); // left
	out.insert(out.end(), 6, glm::vec3( 1.0f,  0.0f,  0.0f)); // right
}


size_t CuboidShape::OutlineCount() const {
	return 24;
}

void CuboidShape::Outline(std::vector<glm::vec3> &out, const glm::vec3 &pos) const {
	out.reserve(24);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.min.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.min.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.max.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.max.y, pos.z + bb.max.z);
	out.emplace_back(pos.x + bb.min.x, pos.y + bb.min.y, pos.z + bb.max.z);
}

bool CuboidShape::Intersects(const Ray &ray, const glm::mat4 &M, float &dist, glm::vec3 &normal) const {
	return Intersection(ray, bb, M, &dist, &normal);
}

}
