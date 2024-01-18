#pragma once

#include "json.h"
#include <deque>

namespace json {

/*Код работы с обновлённым json::Builder не должен компилироваться в следующих ситуациях:
1 - Непосредственно после Key вызван не Value, не StartDict и не StartArray.
2 - После вызова Value, последовавшего за вызовом Key, вызван не Key и не EndDict.
3 - За вызовом StartDict следует не Key и не EndDict.
4 - За вызовом StartArray следует не Value, не StartDict, не StartArray и не EndArray.
5 - После вызова StartArray и серии Value следует не Value, не StartDict, не StartArray и не EndArray.
*/

/*На одном примере разберём, как гарантировать, что после StartDict вызывается Key или EndDict.
Верните из метода StartDict объект вспомогательного класса DictItemContext со следующими свойствами:
хранит ссылку на Builder;
поддерживает только методы Key и EndDict, делегируемые в Builder.
Чтобы избежать дублирования кода с делегированием, удобно использовать наследование.

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

	/* При определении словаря задаёт строковое значение ключа для очередной пары ключ-значение.
	Следующий вызов метода обязательно должен задавать соответствующее этому ключу значение с помощью метода
	Value или начинать его определение с помощью StartDict или StartArray */
	KeyItemContext Key(std::string);

	/* Задаёт значение, соответствующее ключу при определении словаря, очередной элемент массива
	или, если вызвать сразу после конструктора json::Builder, всё содержимое конструируемого JSON-объекта.
	Может принимать как простой объект — число или строку — так и целый массив или словарь. */
	Builder& Value(Node::Value);

	/* Начинает определение сложного значения-словаря. Вызывается в тех же контекстах, что и Value.
	Следующим вызовом обязательно должен быть Key или EndDict. */
	DictItemContext StartDict();

	/* Начинает определение сложного значения-массива.
	Вызывается в тех же контекстах, что и Value.
	Следующим вызовом обязательно должен быть EndArray или любой, задающий новое значение: Value, StartDict или StartArray */
	ArrayItemContext StartArray();

	/* Завершает определение сложного значения-словаря. Последним незавершённым вызовом Start* должен быть StartDict */
	Builder& EndDict();

	/* Завершает определение сложного значения-массива. Последним незавершённым вызовом Start* должен быть StartArray */
	Builder& EndArray();

	/* Возвращает объект json::Node, содержащий JSON, описанный предыдущими вызовами методов.
	К этому моменту для каждого Start* должен быть вызван соответствующий End*.
	При этом сам объект должен быть определён, то есть вызов json::Builder{}.Build() недопустим. */
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

}