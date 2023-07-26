#pragma once

#ifndef PUPPET_MYGAMEOBJECT //rename
#define PUPPET_MYGAMEOBJECT //rename

#include "GameObject.h"

class MyGameObject : public GameObject { //remember to rename class

protected:
	// Triggers when a constraint is broken during a translate call
	Eigen::Vector3f onInvalidTranslation(Eigen::Vector3f translation, BoundaryConstraint* broken_constraint) override {
		return GameObject::onInvalidTranslation(translation, broken_constraint);
	}

	// triggers when the constraint "bc" changes values
	void onInvalidConstraintChange(BoundaryConstraint* bc) override {}

	void onCollision(const GameObject* other, const CollisionPairBase* collision) override {};

	void onDecollision(const GameObject* other, const CollisionPairBase* collision) override {};

	void whileCollision(const GameObject* other, const CollisionPairBase* collision) override {};

	void onAnimationEnd(AnimationBase* animation) override {}


	inline void onKeyPress(int key) override {}; //triggers once //not virtual so it can be inlined as {} if it's not overridden

	inline void onKeyRelease(int key) override {};//triggers once

	inline void onMouseMove(float x, float y, float dx, float dy) override {}; //triggers repeatedly

	inline void onMouseDown(int key, float x, float y) override {};

	inline void onMouseUp(int key, float x, float y) override {};

	inline void onMouseScroll(float dx, float dy) override {};

	inline void onStep() override {};

public:

	const Model* getModel() const override { //these are virtual to allow for unique model/texture instances, i.e. level
		return GameObject::getModel();
	}

	const Texture* getTexture() const override {
		return GameObject::getTexture();
	}

	void clampTo(const GameObject* parent) override {// this has unintuitive behavior
		GameObject::clampTo(parent);
	}

	std::string getDebugInfo() const override {
		return GameObject::getDebugInfo();
	}
	//this is a terrible way of doing this since there is just total boilerplate code where you have to add all new objects to UI_container
	//also if the object is deleted, the UI elements associated with it would also be deleted???
	void openDebugUI(const GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) override {}
	void closeDebugUI(const GameObject* UI_container, GLFWwindow* window, GraphicsRaw<GameObject>& graphics_2d, GraphicsRaw<Textbox>& text_graphics) override {}

};

#endif