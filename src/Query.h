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

	void loadQuery(int type) {
		this->type = type;
		if(!id) glGenQueries(1, &id);

		//Activate and deactivate the query as glGenQueries activates it which gives errors
		glBeginQuery(this->type, this->id);
		glEndQuery(this->type);
	}
	Query(int type) {
		loadQuery(type);
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
class QueryCounter{
public:
	unsigned int id;
	bool resultReady = false;
	int result = 0;

	void loadQuery() {
		if (!id) glGenQueries(1, &id);

		//Activate and deactivate the query as glGenQueries activates it which gives errors
		glBeginQuery(GL_TIMESTAMP, this->id);
		glEndQuery(GL_TIMESTAMP);
	}
	QueryCounter( int type) {
		loadQuery();
	}
	QueryCounter() = default;
	~QueryCounter() {
		glDeleteQueries(1, &id);
	}

	void queryTime() {
		if (isResultReady())
			getResult();

		glQueryCounter(this->id, GL_TIMESTAMP);
	}
	bool isResultReady() {
		int param;

		glGetQueryObjectiv(this->id, GL_QUERY_RESULT_AVAILABLE, &param);
		return param == GL_TRUE;
		//if (param == 0)
		//	return true;
		//else if (param == 1) {
		//	return false;
		//}
		//else //Ignoring errors
		//	return false;
	}
	void getResult() {
		glGetQueryObjectiv(this->id, GL_QUERY_RESULT, &this->result);
	}
};
#endif