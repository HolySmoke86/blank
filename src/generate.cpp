#include "app/Assets.hpp"
#include "shared/WorldResources.hpp"
#include "world/Chunk.hpp"
#include "world/Generator.hpp"

#include <chrono>
#include <iostream>
#include <glm/gtx/io.hpp>

using namespace blank;
using namespace std;
using namespace chrono;


int main() {
	AssetLoader loader("assets/");
	WorldResources res;
	res.Load(loader, "default");
	Generator::Config conf;
	Generator gen(conf);
	gen.LoadTypes(res.block_types);
	Chunk chunk(res.block_types);

	const ExactLocation::Coarse begin(-6);
	const ExactLocation::Coarse end(6);
	size_t total_chunks = ((end.x - begin.x) * (end.y - begin.y) * (end.z - begin.z));

	size_t candidates = 0;
	for (const BlockType &type : res.block_types) {
		if (type.generate) {
			++candidates;
		}
	}

	cout << "generating " << total_chunks << " chunks from " << begin << " to " << (end - 1) << endl;
	cout << candidates << " of " << res.block_types.size() << " block types applicable for generation" << endl;
	auto enter = high_resolution_clock::now();

	for (int z = begin.z; z < end.z; ++z) {
		for (int y = begin.y; y < end.y; ++y) {
			for (int x = begin.x; x < end.x; ++x) {
				chunk.Position({ x, y, z });
				gen(chunk);
			}
		}
	}

	auto exit = high_resolution_clock::now();
	cout << duration_cast<milliseconds>(exit - enter).count() << "ms ("
		<< (duration_cast<nanoseconds>(exit - enter).count() / total_chunks / 1.0e6f) << "ms chunk avg)" << endl;
	return 0;
}
