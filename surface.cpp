#include<unordered_set>

#include "Model.h"
#include "surface.hpp"

MeshSurface::MeshSurface(std::string fname) : MeshSurface(fname, Model::default_path) {}

MeshSurface::MeshSurface(std::string fname, std::string path){
	Model model(fname, path, false);
	for (int i = 0; i < model.vlen(); i++) {
		verts_.emplace_back(model.getVerts()[3 * i], model.getVerts()[3 * i + 1], model.getVerts()[3 * i + 2]);
	}
	if (model.getLines().size() > 0) {
		for (int i = 0; i < model.getLines().size() / 2; i++) {
			std::pair<int, int>e(model.getLines()[2 * i], model.getLines()[2 * i + 1]);
			edges_.push_back(e);
		}
	}
	else {
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
	calculateBoundingBox();

}