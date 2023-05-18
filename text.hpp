#pragma once

#ifndef PUPPET_TEXT
#define PUPPET_TEXT

#include <Eigen/Dense>
#include <string>

struct Textbox {
	std::string text;
	std::string font;
	float top, left;
	float box_width, box_height;
	float font_size;
	enum class wrap_type {};
	enum class allignment {};
	float line_space;
	bool hidden_;
	virtual bool isHidden() const {
		return hidden_;
	}
	/*...*/

	Textbox() :text(""), font(""), top(0), left(0), box_width(1), box_height(1), font_size(1) {}

	virtual Eigen::Matrix4f getPosition() const {
		Eigen::Matrix4f ret;
		ret << 1, 0, 0, left,
			0, 1, 0, top,
			0, 0, 1, 0,
			0, 0, 0, 1;
		return ret;
	}

	virtual size_t getID() const {
		return std::hash<std::string>()(text);
	}

};


#endif