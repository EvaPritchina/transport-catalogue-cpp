#pragma once

#include <string>
#include <vector>
#include "json.h"

namespace json {
class Builder {
public:
    Builder()
    :root_(),
     nodes_stack_{&root_}{}

    class ArraySpecific;
    class DictSpecific;
    class StartSpecific;

    StartSpecific Key(std::string key);
    Builder& Value(Node::Value value);
    DictSpecific StartDict();
    ArraySpecific StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();
    void Add(Node::Value value, bool simple);

private:
    Node GetNode(Node::Value value);
    Node::Value& ValueAndAssert();
    Node root_;
    std::vector<Node*> nodes_stack_;
};

class Builder::ArraySpecific{
public:
    ArraySpecific(Builder& builder)
    :builder_(builder){}

    ArraySpecific Value(Node::Value value);
    ArraySpecific StartArray();
    Builder& EndArray();
    DictSpecific StartDict();
private:
    Builder& builder_;
};

class Builder::DictSpecific{
public:
    DictSpecific(Builder& builder)
    :builder_(builder){}

    StartSpecific Key(std::string key);
    Builder& EndDict();

private:
    Builder& builder_;

};

class Builder::StartSpecific{
public:
    StartSpecific(Builder& builder)
    :builder_(builder){}
    
    DictSpecific StartDict();
    ArraySpecific StartArray();
    DictSpecific Value(Node::Value value);

private:
    Builder& builder_;

};

}  // namespace json
