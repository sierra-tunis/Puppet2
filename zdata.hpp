#pragma once

#ifndef PUPPET_ZDATA
#define PUPPET_ZDATA

#include <cmath>
#include <array>

struct zdata {
	float z, x_slope, y_slope;
	int room_id; //room_id is the alpha bit and is transparent from the background. 0 is reserved for inaccessible
	zdata(std::array<float, 3> data, int room_id) :z(data[0]), x_slope(data[1]), y_slope(data[2]), room_id(room_id) {}
	static zdata floor() {
		return zdata({ -INFINITY,0,0 }, 0);
	}
	static zdata ceiling() {
		return zdata({ INFINITY,0,0 }, 0);
	}

	bool operator!=(const zdata& other) {
		return (this->z != other.z || this->x_slope != other.x_slope || this->y_slope != other.y_slope);
	}

	static constexpr int BaseRoom = 0;

};

#endif