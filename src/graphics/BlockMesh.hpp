#ifndef BLANK_GRAPHICS_BLOCKMESH_HPP_
#define BLANK_GRAPHICS_BLOCKMESH_HPP_

#include "glm.hpp"
#include "VertexArray.hpp"

#include <vector>
#include <GL/glew.h>


namespace blank {

class BlockMesh {

public:
	using Position = glm::vec3;
	using TexCoord = glm::vec3;
	using ColorMod = TVEC3<unsigned char, glm::precision(0)>;
	using Light = float;
	using Index = unsigned int;

	using Positions = std::vector<Position>;
	using TexCoords = std::vector<TexCoord>;
	using ColorMods = std::vector<ColorMod>;
	using Lights = std::vector<Light>;
	using Indices = std::vector<Index>;

	enum Attribute {
		ATTRIB_VERTEX,
		ATTRIB_TEXCOORD,
		ATTRIB_HSL,
		ATTRIB_RGB,
		ATTRIB_LIGHT,
		ATTRIB_INDEX,
		ATTRIB_COUNT,
	};

	struct Buffer {

		Positions vertices;
		TexCoords tex_coords;
		ColorMods hsl_mods;
		ColorMods rgb_mods;
		Lights lights;
		Indices indices;

		void Clear() noexcept {
			vertices.clear();
			tex_coords.clear();
			hsl_mods.clear();
			rgb_mods.clear();
			lights.clear();
			indices.clear();
		}

		void Reserve(size_t p, size_t i) {
			vertices.reserve(p);
			tex_coords.reserve(p);
			hsl_mods.reserve(p);
			rgb_mods.reserve(p);
			lights.reserve(p);
			indices.reserve(i);
		}

	};

	using VAO = VertexArray<ATTRIB_COUNT>;

public:
	void Update(const Buffer &) noexcept;

	bool Empty() const noexcept {
		return vao.Empty();
	}

	void Draw() const noexcept {
		vao.DrawTriangleElements();
	}

private:
	VAO vao;

};

}

#endif
