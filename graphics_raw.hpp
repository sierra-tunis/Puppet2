#pragma once

#ifndef PUPPET_GRAPHICSRAW
#define PUPPET_GRAPHICSRAW

#include<concepts>

template<class T>
concept Identifiable = requires(const T & t) {
	{t.getID()}->std::convertible_to<size_t>;
	{t.isHidden()}->std::convertible_to<bool>;
};

template <class Object>
class GraphicsRaw {
public:

	struct Error {

	};

	void refresh(const Object& obj) {
		unload(obj);
		add(obj);
	}

	virtual void add(const Object& obj) = 0;

	virtual void remove(const Object& obj) = 0;

	virtual void unload(const Object& obj) = 0;

};


#endif