#pragma once

#ifndef PUPPET_GRAPH
#define PUPPET_GRAPH

#include<concepts>

template<class T>
class UnorderedGraph { //i.e. ordered graph. iteration is easy, neighbor finding is hard. cardinality is possible
	std::unordered_set<std::pair<const T*, const T*>> edges;

	//bridges are always crossible in both directions

	//const T* next(T) is slow
	//const T* get(size_t i) is slow or undefined
	//it is undefined when not contiguous

}; 

template<class T> //i.e. unordered graph. iteration is hard (without loops), neighbor finding is easy.
class OrderedGraph {
	std::unordered_set<const std::unordered_set<const T*>*> neighbors;

	//bridges are not always crossible in both directions

	// std::unordered_set<const T*> next(T) is fast
	// const T* get(size_t i) is fast or undefined when any set size is > 1

};

#endif