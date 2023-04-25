#pragma once

#ifndef PUPPET_GRAPHICS
#define PUPPET_GRAPHICS

#include <glad/glad.h>
#include <iostream>
#include <unordered_map>
#include <concepts>


#include "InternalObject.h"
#include "GraphicsObject.h"

//#include <gl/GL.h>

/*
template<class T>
concept Cachable = std::is_base_of<DataCache,T>::value;
*/
template <class T>
void write_png(std::string fname, int screenshot_width, int screenshot_height, std::vector<T>* img);

template<class T, class obj_T>
concept Cacher = requires(obj_T obj) {
	new T(obj);
};
/*
template <class Object, Cacher cache_T>
class Graphics {
*/

template<class T>
concept Identifiable = requires(const T & t) {
	{t.getID()}->std::convertible_to<size_t>;
};

template <Identifiable Object, class ... data>
class Graphics {
private:
	int FBO_;
	int render_buffer_;
	int screenshot_width_;
	int screenshot_height_;
	//std::vector<int>* screenshot_data_;


	static void check_compile_error(unsigned int shader) {
		int  success;
		char infoLog[512];
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}

	static unsigned int compile_vertex() {
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertex_code, NULL);
		glCompileShader(vertexShader);

		check_compile_error(vertexShader);
		std::cout << vertex_code;
		return vertexShader;
	}

	static unsigned int compile_fragment() {
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragment_code, NULL);
		glCompileShader(fragmentShader);
		check_compile_error(fragmentShader);
		std::cout << fragment_code;
		return fragmentShader;
	}


protected:
	typedef std::tuple<data...> Cache;

	/*
	struct DataCache {
		std::tuple<data...> data;
		DataCache(std::tuple<data...> data) :data(data) {};
	};
	

	virtual std::tuple<data...> getTuple(Cache cache) const = 0;
	virtual Cache getCache < std::tuple<data...> const = 0;
	*/

	static unsigned int compile_program() {

		unsigned int vertexShader = compile_vertex();
		unsigned int fragmentShader = compile_fragment();
		unsigned int shaderProgram;
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		check_compile_error(shaderProgram);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		std::cout << shaderProgram;
		return shaderProgram;
	}
	
	virtual void drawObj(const Object& obj , Cache cache) const = 0;

	virtual void beginDraw() const {
	};

	virtual void endDraw() const {
		
	};

	virtual Cache makeDataCache(const Object& obj) const = 0;
	virtual void deleteDataCache(Cache cache) const = 0;

	const int gl_id;
	static const char* vertex_code;
	static const char* fragment_code;
	static std::unordered_map<int, const Object*> draw_targets_; //needs to be map for removal
	static std::unordered_map<int, Cache> cached_data_;

public:
	
	void drawAll() const {
		glUseProgram(gl_id);
		beginDraw();
		for (const auto& obj : draw_targets_) {
			this->drawObj(*(obj.second), cached_data_[obj.first]);
		}
		endDraw();
		glBindVertexArray(0);
		glUseProgram(0);
	}



	//instead of getID it should just hash obj. the hash for GameObj can just be return id_
	void add(const Object& obj) {
		draw_targets_.insert({ obj.getID(), &obj });
		if (cached_data_.find(obj.getID()) == cached_data_.end()) {
			cached_data_.insert({ obj.getID(),this->makeDataCache(obj) });
		}
	}

	void remove(const Object& obj){
		draw_targets_.erase(obj.getID());
	}

	void unload(const Object& obj) {
		draw_targets_.erase(obj.getID());
		deleteDataCache(cached_data_[obj.getID()]);
		cached_data_.erase(obj.getID());
	}

	//virtual G* makeGrobj(const GameObject& obj) const = 0;
	
	void startScreenshot( size_t width, size_t height) {
		if (FBO_ == -1) {
			unsigned int fbo,depth_render_buffer, render_buffer;
			glGenFramebuffers(1, &fbo);
			this->FBO_ = static_cast<int>(fbo);
			glGenRenderbuffers(1, &render_buffer);
			this->render_buffer_ = static_cast<int>(render_buffer);
			glGenRenderbuffers(1, &depth_render_buffer);
			glBindRenderbuffer(GL_RENDERBUFFER,static_cast<unsigned int>(render_buffer_));
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height); 
			glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

			glBindFramebuffer(GL_FRAMEBUFFER,static_cast<unsigned int>(FBO_));
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, render_buffer_);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_render_buffer);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<unsigned int>(FBO_));
		screenshot_width_ = width;
		screenshot_height_ = height;
	}

	template <class T, unsigned int gl_data_type_enum>
	void finishScreenshot(std::vector<T>* img) {
		img->reserve(screenshot_width_ * screenshot_height_ * 4); //4 buffers
		glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<unsigned int>(FBO_));
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(0, 0, screenshot_width_, screenshot_height_, GL_RGBA, gl_data_type_enum,img->data());
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	template <class T, unsigned int gl_data_type_enum>
	void finishScreenshot(std::vector<T>* img, std::string fname) {
		finishScreenshot<T, gl_data_type_enum>(img);
		write_png<T>(fname, screenshot_width_, screenshot_height_, img);
		//stbi_write_png(fname.c_str(), screenshot_width_, screenshot_height_, 4, img->data(), screenshot_width_ * 4);//4 channels
	}


	Graphics() :gl_id(static_cast<int>(Graphics<Object,data...>::compile_program())),FBO_(-1) {
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			printf("Error during Graphics creation: 0x%x\n", error);
		}
	}

};
template <Identifiable Object, class...data>
std::unordered_map<int, std::tuple<data...>> Graphics<Object, data...>::cached_data_ = std::unordered_map<int, std::tuple<data...>>();
template <Identifiable Object, class...data>
std::unordered_map<int, const Object*> Graphics<Object, data...>::draw_targets_ = std::unordered_map<int, const Object*>();

//YOU CAN USE TEMPLATE ARGUMENTS IN OPENGL CODE BEFORE THEY COMPILE!!!

#endif