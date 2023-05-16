#pragma once
#ifndef QUERY
#define QUERY

#include <GLAD/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>

class Query {
public:
	unsigned int id;
	unsigned int type;
	bool inUse = false;
	bool resultReady = false;
	int result = 0;
	int param;

	void loadQuery(int type) {
		this->type = type;
		if(!id) glGenQueries(1, &id);

		//Activate and deactivate the query as glGenQueries activates it which gives errors
		glBeginQuery(this->type, this->id);
		glEndQuery(this->type);
	}
	Query(int type) {
		createQuery(type);
	}
	Query() {};
	~Query() {
		glDeleteQueries(1, &id);
	}

	void begin() {
		if (!inUse) {
			if (isResultReady())
				getResult();

			glBeginQuery(this->type, this->id);
			this->inUse = true;
		}
	}
	void end() {
		glEndQuery(this->type);
		this->inUse = false;
	}
	bool isResultReady() {
		int param;

		glGetQueryObjectiv(this->id, GL_QUERY_RESULT_AVAILABLE, &param);

		if (param == 0)
			return true;
		else if (param == 1) {
			return false;
		}
		else //Ignoring errors
			return false;
	}
	int getResult() {
		glGetQueryObjectiv(this->id, GL_QUERY_RESULT, &this->result);

		return this->result;
	}

};
#endif