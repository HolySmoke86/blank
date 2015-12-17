#ifndef BLANK_GRAPHICS_ENTITYMESH_HPP_
#define BLANK_GRAPHICS_ENTITYMESH_HPP_

#include "VertexArray.hpp"

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>


namespace blank {

class EntityMesh {

public:
	using Position = glm::vec3;
	using TexCoord = glm::vec3;
	using ColorMod = glm::tvec3<unsigned char>;
	using Normal = glm::vec3;
	using Index = unsigned int;

	using Positions = std::vector<Position>;
	using TexCoords = std::vector<TexCoord>;
	using ColorMods = std::vector<ColorMod>;
	using Normals = std::vector<Normal>;
	using Indices = std::vector<Index>;

	enum Attribute {
		ATTRIB_VERTEX,
		ATTRIB_TEXCOORD,
		ATTRIB_HSL,
		ATTRIB_RGB,
		ATTRIB_NORMAL,
		ATTRIB_INDEX,
		ATTRIB_COUNT,
	};

	struct Buffer {

		Positions vertices;
		TexCoords tex_coords;
		ColorMods hsl_mods;
		ColorMods rgb_mods;
		Normals normals;
		Indices indices;

		void Clear() noexcept {
			vertices.clear();
			tex_coords.clear();
			hsl_mods.clear();
			rgb_mods.clear();
			normals.clear();
			indices.clear();
		}

		void Reserve(size_t p, size_t i) {
			vertices.reserve(p);
			tex_coords.reserve(p);
			hsl_mods.reserve(p);
			rgb_mods.reserve(p);
			normals.reserve(p);
			indices.reserve(i);
		}

	};

	using VAO = VertexArray<ATTRIB_COUNT>;

public:
	void Update(const Buffer &) noexcept;

	void Draw() const noexcept;

private:
	VAO vao;

};

}

#endif
