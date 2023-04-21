#include "ZMapper.h"
#include "zmap.h"

void Zmap::createData(const Model& model, unsigned int n_steps, std::vector<const Model*> secondary_models){
	 ZMapper zmapper;
	 float z = model.getBoxCenter()[1] - model.getBoundingBox()[1] / 2;
	 if (n_steps < 2) {
		 std::cerr << "n_steps must be greater than 2";
	 }
	 while (z <= model.getBoxCenter()[1] + model.getBoundingBox()[1] / 2) {
		 std::vector<uint8_t> data(x_resolution_ * y_resolution_ * 4);
		 float z_step = model.getBoundingBox()[1] / static_cast<float>(n_steps - 1);
		 zmapper.renderZstep(model, y_resolution_, x_resolution_, z, z_step, &data, secondary_models);
		 addLayer(data, z, z_step);
		 z += z_step;
	 }
}
