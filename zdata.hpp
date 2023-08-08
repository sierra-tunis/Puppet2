#pragma once

#ifndef PUPPET_ZDATA
#define PUPPET_ZDATA

#include <cmath>
#include <array>

struct zdata {
	uint8_t room_id;
	float z;
	//room_id is the alpha bit and is transparent from the background. 0 is reserved for inaccessible
	zdata(std::array<float, 3> data, uint8_t room_id) :z(data[0]), room_id(room_id) {}

	static constexpr uint8_t BaseRoom = 0;

	static zdata floor() {
		return zdata({ -INFINITY,0,0 }, BaseRoom);
	}
	static zdata ceiling() {
		return zdata({ INFINITY,0,0 }, BaseRoom);
	}

	bool operator!=(const zdata& other) const {
		return (this->z != other.z);
	}

	zdata() :z(0), room_id(zdata::room_id) {

	}


};

#endif