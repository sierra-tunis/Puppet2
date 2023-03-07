#pragma once
/*
template<class T, T data, T... data1n>
class linkedList : public linkedList<T,data1n...>{

	//static inline int index = sizeof...(data1n);

public:
	virtual void printNum() {
		linkedList<T, data1n...>::printNum();
		std::cout << sizeof...(data1n) << "\n";
	}

	virtual void printData() {
		linkedList<T, data1n...>::printData();
		std::cout << data << "\n";
	}

	template<unsigned int i>
	static T getData() {
		if (sizeof...(data1n) != i) {
			return linkedList<T, data1n...>::getData<i>();
		} else {
			return data;
		}
	}

};

template<class T,T data0>
class linkedList<T,data0> {
	T data;
	const unsigned int index = 0;
public:
	virtual void printNum() {
		std::cout << index<< "\n";
	}
	virtual void printData() {
		std::cout << data << "\n";
	}
	template<unsigned int i>
	static T getData() {
		if (i!=0) {
			std::cout << "index out of bounds\n";
			return data;
		}
		else {
			return data;
		}
	}

};



*/