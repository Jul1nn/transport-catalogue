#include "json_builder.h"

#include <stdexcept>
#include <algorithm>

namespace json {

using namespace std;

void Builder::RootChecker() {
	if ((!root_.IsNull() || create_checker_) && nodes_stack_.empty()) {
		throw logic_error("Node is already finished"s);
	}
}

Builder& Builder::Value(Node::Value val) {
	RootChecker();
	if (!nodes_stack_.empty()) {
		if (nodes_stack_.back()->IsDict()) {
			throw logic_error("Trying to add Value in Dict without Key"s);
		}
		if (nodes_stack_.back()->IsNull()) {
			*nodes_stack_.back() = Node(move(val));
			nodes_stack_.pop_back();
		}
		if (nodes_stack_.back()->IsArray()) {
			nodes_stack_.back()->TakeAsArray().push_back(Node(move(val)));
		}
	}
	else {
		root_ = Node(move(val));
		create_checker_ = true;
	}
	return *this;
}

DictItemContext Builder::StartDict() {
	RootChecker();
	nodes_.emplace_back(Dict());
	nodes_stack_.push_back(&nodes_.back());
	return DictItemContext(*this);
}

KeyItemContext Builder::Key(std::string s) {
	RootChecker();
	if (nodes_stack_.back()->IsNull()) {
		throw logic_error("Trying to create Key after Key"s);
	}
	if (nodes_stack_.empty() || nodes_stack_.back()->IsArray()) {
		throw logic_error("Trying to create Key outside Dict"s);
	}
	nodes_stack_.back()->TakeAsDict().emplace(s, Node());
	nodes_stack_.push_back(const_cast<Node*>(&nodes_stack_.back()->AsDict().at(s)));
	return KeyItemContext(*this);
}

ArrayItemContext Builder::StartArray() {
	RootChecker();
	nodes_.emplace_back(Array());
	nodes_stack_.emplace_back(&nodes_.back());
	return ArrayItemContext(*this);
}

Builder& Builder::EndDict() {
	RootChecker();
	if (!nodes_stack_.back()->IsDict()) {
		throw logic_error("Trying to End not a Dict"s);
	}
	Node n = move(nodes_.back());
	nodes_stack_.pop_back();
	nodes_.pop_back();
	return Value(n.GetValue());
}

Builder& Builder::EndArray() {
	RootChecker();
	if (!nodes_stack_.back()->IsArray()) {
		throw logic_error("Trying to End not an Array"s);
	}
	Node n = move(nodes_.back());
	nodes_stack_.pop_back();
	nodes_.pop_back();
	return Value(n.GetValue());
}

Node Builder::Build() {
	if (root_.IsNull() && !create_checker_) {
		throw logic_error("Node was not constructed"s);
	}
	if (!nodes_stack_.empty()) {
		throw logic_error("Node construction was not finished"s);
	}
	return root_;
}

ArrayItemContext ArrayItemContext::Value(Node::Value val) {
	return { GetBuilder().Value(val) };
}

DictItemContext ArrayItemContext::StartDict() {
	return GetBuilder().StartDict();
}

ArrayItemContext ArrayItemContext::StartArray() {
	return GetBuilder().StartArray();
}

Builder& ArrayItemContext::EndArray() {
	return GetBuilder().EndArray();
}

DictItemContext KeyItemContext::Value(Node::Value val) {
	return { GetBuilder().Value(val) };
}

DictItemContext KeyItemContext::StartDict() {
	return GetBuilder().StartDict();
}

ArrayItemContext KeyItemContext::StartArray() {
	return GetBuilder().StartArray();
}

KeyItemContext DictItemContext::Key(std::string s) {
	return GetBuilder().Key(s);
}

Builder& DictItemContext::EndDict() {
	return GetBuilder().EndDict();
}

} //nfmespace json