#include "json_builder.h"
#include <exception>
#include <variant>
#include <utility>

using namespace std::literals;

namespace json {

    Node Builder::GetNode(Node::Value value) {
        if (std::holds_alternative<int>(value)) return Node(std::get<int>(value));
        if (std::holds_alternative<double>(value)) return Node(std::get<double>(value));
        if (std::holds_alternative<std::string>(value)) return Node(std::get<std::string>(value));
        if (std::holds_alternative<std::nullptr_t>(value)) return Node(std::get<std::nullptr_t>(value));
        if (std::holds_alternative<bool>(value)) return Node(std::get<bool>(value));
        if (std::holds_alternative<Dict>(value)) return Node(std::get<Dict>(value));
        if (std::holds_alternative<Array>(value)) return Node(std::get<Array>(value));
        return {};
    }

    Node::Value& Builder::ValueAndAssert() {
        if (nodes_stack_.empty()) {
            throw std::logic_error("unable to get value"s);
        }
        return nodes_stack_.back()->GetValue();
    }

    Builder::StartSpecific Builder::Key(std::string key) {
        Node::Value& back_node = ValueAndAssert();
        if (!GetNode(back_node).IsMap()) {
            throw std::logic_error("unable to create key"s);
        }
        auto& dict = std::get<Dict>(back_node);
		nodes_stack_.push_back(&dict[std::move(key)]);
        return *this;
    }

    Builder& Builder::Value(Node::Value value) {
        Add(value, true);
        return *this;
    }

    Builder::DictSpecific Builder::StartDict() {
        Add(Dict{}, false);
        return *this;
    }

    Builder::ArraySpecific Builder::StartArray() {
        Add(Array{}, false);
        return *this;
    }

    Builder& Builder::EndDict() {
        if (!(GetNode(ValueAndAssert())).IsMap()) {
            throw std::logic_error("incorrect circs for end dict"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::EndArray() {
        if (!(GetNode(ValueAndAssert())).IsArray()) {
            throw std::logic_error("incorrect circs for end array"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }
    Node Builder::Build() {
        if(root_.IsNull() || nodes_stack_.size() > 1) {
            throw std::logic_error("unable to start build");
        }
        return std::move(root_);
    }

    void Builder::Add(Node::Value value, bool simple){
        Node::Value& back_node = ValueAndAssert();
        if(GetNode(back_node).IsArray()){
            auto& array = std::get<Array>(back_node);
            array.emplace_back(std::move(GetNode(value)));
            if(!simple){
                nodes_stack_.push_back(&array.back());
            }
        }
        else {
            if(!nodes_stack_.empty()){
                if(!GetNode(back_node).IsNull()){
                    throw std::logic_error("incorrect circs for set value"s);
                }
                back_node = std::move(value);
                if(simple){
                    nodes_stack_.pop_back();
                }
            }
    
        }
    }   

    Builder::ArraySpecific Builder::ArraySpecific::Value(Node::Value value){
        return ArraySpecific(builder_.Value(value));
    }

    Builder::ArraySpecific Builder::ArraySpecific::StartArray(){
       return builder_.StartArray();
    }

    Builder& Builder::ArraySpecific::EndArray(){
        return builder_.EndArray();
    }

    Builder::DictSpecific Builder::ArraySpecific::StartDict(){
      return builder_.StartDict();
    }

    Builder::StartSpecific Builder::DictSpecific::Key(std::string key){
        return builder_.Key(key);
    }

    Builder& Builder::DictSpecific::EndDict(){
        return builder_.EndDict();
    }

    Builder::DictSpecific Builder::StartSpecific::StartDict(){
        return builder_.StartDict();
    }

    Builder::ArraySpecific Builder::StartSpecific::StartArray(){
        return builder_.StartArray();
    }
    
    Builder::DictSpecific Builder::StartSpecific::Value(Node::Value value){
        return DictSpecific(builder_.Value(value));
    }


}  // namespace json
