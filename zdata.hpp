#pragma once

#ifndef PUPPET_ZDATA
#define PUPPET_ZDATA

#define _USE_MATH_DEFINES
#include <cmath>
#include <array>

struct zdata {
	float z, x_slope, y_slope;
	uint8_t room_id; //room_id is the alpha bit and is transparent from the background. 0 is reserved for inaccessible
	zdata(std::array<float, 3> data, uint8_t room_id) :z(data[0]), x_slope(data[1]), y_slope(data[2]), room_id(room_id) {}

	static constexpr uint8_t BaseRoom = 0;

	static zdata floor() {
		return zdata({ -INFINITY,0,0 }, BaseRoom);
	}
	static zdata ceiling() {
		return zdata({ INFINITY,0,0 }, BaseRoom);
	}

	bool operator!=(const zdata& other) {
		return (this->z != other.z || this->x_slope != other.x_slope || this->y_slope != other.y_slope);
	}

	zdata() :z(0), x_slope(0), y_slope(0), room_id(zdata::room_id) {

	}


};

#endif