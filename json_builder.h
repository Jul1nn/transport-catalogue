#pragma once

#include "json.h"
#include <deque>

namespace json {

/*��� ������ � ���������� json::Builder �� ������ ��������������� � ��������� ���������:
1 - ��������������� ����� Key ������ �� Value, �� StartDict � �� StartArray.
2 - ����� ������ Value, �������������� �� ������� Key, ������ �� Key � �� EndDict.
3 - �� ������� StartDict ������� �� Key � �� EndDict.
4 - �� ������� StartArray ������� �� Value, �� StartDict, �� StartArray � �� EndArray.
5 - ����� ������ StartArray � ����� Value ������� �� Value, �� StartDict, �� StartArray � �� EndArray.
*/

/*�� ����� ������� �������, ��� �������������, ��� ����� StartDict ���������� Key ��� EndDict.
������� �� ������ StartDict ������ ���������������� ������ DictItemContext �� ���������� ����������:
������ ������ �� Builder;
������������ ������ ������ Key � EndDict, ������������ � Builder.
����� �������� ������������ ���� � ��������������, ������ ������������ ������������.

3 - DictItemContext - Key, EndDict - after StartDict
1 - KeyItemContext - Value, StartDict, StartArray - after Key
2 - DictItemContext - after Value after Key
4 - ArrayItemContext - Value, StartDict, StartArray, EndArray - after StartArray
5 - ArrayItemContext - after StartArray after Value (any amount of calls)

*/

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

	/* ��� ����������� ������� ����� ��������� �������� ����� ��� ��������� ���� ����-��������.
	��������� ����� ������ ����������� ������ �������� ��������������� ����� ����� �������� � ������� ������
	Value ��� �������� ��� ����������� � ������� StartDict ��� StartArray */
	KeyItemContext Key(std::string);

	/* ����� ��������, ��������������� ����� ��� ����������� �������, ��������� ������� �������
	���, ���� ������� ����� ����� ������������ json::Builder, �� ���������� ��������������� JSON-�������.
	����� ��������� ��� ������� ������ � ����� ��� ������ � ��� � ����� ������ ��� �������. */
	Builder& Value(Node::Value);

	/* �������� ����������� �������� ��������-�������. ���������� � ��� �� ����������, ��� � Value.
	��������� ������� ����������� ������ ���� Key ��� EndDict. */
	DictItemContext StartDict();

	/* �������� ����������� �������� ��������-�������.
	���������� � ��� �� ����������, ��� � Value.
	��������� ������� ����������� ������ ���� EndArray ��� �����, �������� ����� ��������: Value, StartDict ��� StartArray */
	ArrayItemContext StartArray();

	/* ��������� ����������� �������� ��������-�������. ��������� ������������� ������� Start* ������ ���� StartDict */
	Builder& EndDict();

	/* ��������� ����������� �������� ��������-�������. ��������� ������������� ������� Start* ������ ���� StartArray */
	Builder& EndArray();

	/* ���������� ������ json::Node, ���������� JSON, ��������� ����������� �������� �������.
	� ����� ������� ��� ������� Start* ������ ���� ������ ��������������� End*.
	��� ���� ��� ������ ������ ���� ��������, �� ���� ����� json::Builder{}.Build() ����������. */
	Node Build();

private:

	//��� �������������� ������.
	Node root_;

	//���� ���������� �� �� ������� JSON, ������� ��� �� ���������: �� ���� ������� ����������� �������� � ������� ��� ���������.
	std::vector<Node*> nodes_stack_;

	//��������� ����� ������ Value.
	bool create_checker_ = false;

	//���������, ��������� �� ��������������� ����.
	void RootChecker();

	std::deque<Node> nodes_;

};

}