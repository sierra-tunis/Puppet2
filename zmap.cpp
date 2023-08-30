#include "ZMapper.h"
#include "zmap.h"

void Zmap::createData(const GameObject& level, unsigned int n_steps, const std::vector<const GameObject*>& neighbors, void* ZMapper_ptr) {
	 //ZMapper zmapper;//this will have to change
	 ZMapper& zmapper = *(static_cast<ZMapper*>(ZMapper_ptr));
	 const Model& model = *level.getModel();
	 float z_step = model.getBoundingBox()[1] / static_cast<float>(n_steps - 1);
	 float z = model.getBoxCenter()[1] - model.getBoundingBox()[1] / 2 - z_step/2;
	 if (n_steps < 2) {
		 std::cerr << "n_steps must be greater than 2";
	 }
	 int layer = 0;
	 if (z_step == 0.) {
		 std::cerr << "level model is empty!\n";
		 return;
	 }
	 while (z <= model.getBoxCenter()[1] + model.getBoundingBox()[1] / 2 + z_step) {
		 std::vector<uint8_t> data(x_resolution_ * y_resolution_ * 4);
		 zmapper.renderZstep(level, y_resolution_, x_resolution_, z, z_step,xy_padding_, &data, neighbors, "\\ZmapImages\\"+level.getName()+"\\"+ "zmap_layer" + std::to_string(layer) + "_" + level.getName() + ".png");
		 addLayer(data, z, z_step);
		 z += z_step;
		 layer++;
	 }
	 size_t size_total = 0;
	 for (int i = 0; i < x_resolution_ * y_resolution_; i++) {
		 size_total += raw_data_[i].size()*sizeof(zdata) + sizeof(raw_data_);
	 }

	 std::cout << "size of zmap for " + level.getName() + ": " << size_total/1000 << "kB (zdata=" << sizeof(zdata) <<"bytes)\n";

}
