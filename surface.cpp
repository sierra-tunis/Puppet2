#include<unordered_set>

#include "Model.h"
#include "surface.hpp"


MeshSurface::MeshSurface(std::string fname){
	Model model(fname);
	for (int i = 0; i < model.vlen(); i++) {
		verts_.emplace_back(model.getVerts()[3 * i], model.getVerts()[3 * i + 1], model.getVerts()[3 * i + 2]);
	}
	std::unordered_set<std::pair<int, int>, edgeHasher> edges;
	for (int i = 0; i < model.flen(); i++) {
		for (int first = 0; first < 3; first++) {
			for (int second = 0; second < 3; second++) {
				if (first == second) continue;
				std::pair<int, int>e(model.getFaces()[3 * i + first], model.getFaces()[3 * i + second]);
				if (!edges.contains(e)) {
					edges.emplace(e);
					edges_.push_back(e);
				}
			}
		}
		faces_.push_back(std::tuple<int, int, int>{model.getFaces()[3 * i], model.getFaces()[3 * i + 1], model.getFaces()[3 * i + 2]});
	}
}