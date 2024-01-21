#pragma once

#include "json.h"
#include <deque>

namespace json {

class Builder;

class BaseContext {
public:
	BaseContext() = default;
	
	BaseContext(Builder& builder)
		:builder_(builder) {}

	Builder& GetBuilder() {
		return builder_;
	}

private:
	Builder& builder_;

};

class DictItemContext;

class ArrayItemContext : public BaseContext {
public:
	ArrayItemContext(Builder& builder)
		:BaseContext(builder) {}

	ArrayItemContext Value(Node::Value);

	DictItemContext StartDict();

	ArrayItemContext StartArray();

	Builder& EndArray();
};

class KeyItemContext : public BaseContext {
public:
	KeyItemContext(Builder& builder)
		:BaseContext(builder) {}

	DictItemContext Value(Node::Value);

	DictItemContext StartDict();

	ArrayItemContext StartArray();

};

class DictItemContext : public BaseContext {
public:
	DictItemContext(Builder& builder)
		:BaseContext(builder) {}

	KeyItemContext Key(std::string s);

	Builder& EndDict();
};

class Builder {
public:
	Builder() = default;

	KeyItemContext Key(std::string);

	Builder& Value(Node::Value);

	DictItemContext StartDict();

	ArrayItemContext StartArray();

	Builder& EndDict();

	Builder& EndArray();

	Node Build();

private:

	//Сам конструируемый объект.
	Node root_;

	//Стек указателей на те вершины JSON, которые ещё не построены: то есть текущее описываемое значение и цепочка его родителей.
	std::vector<Node*> nodes_stack_;

	//Проверяет вызов метода Value.
	bool create_checker_ = false;

	//Проверяет, закончено ли конструирование узла.
	void RootChecker();

	std::deque<Node> nodes_;

};

} //namespace json