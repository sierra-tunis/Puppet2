#ifndef PUPPET_TEXTBOXOBJECT
#define PUPPET_TEXTBOXOBJECT

#include "text.hpp"
#include "GameObject.h"


class TextboxObject : public GameObject, public Textbox {
private:
	std::string last_text_;

	Eigen::Matrix4f getPosition() const override {
		return GameObject::getPosition();
	}

	size_t getID() const override {
		return GameObject::getID();
	}
	bool isHidden() const override{
		return GameObject::isHidden();
	}

};

#endif