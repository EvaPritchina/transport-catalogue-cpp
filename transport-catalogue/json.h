#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {
  class Node;

  using Dict = std::map < std::string, Node >;
  using Array = std::vector < Node >;

  class ParsingError: public std::runtime_error {
    public: using runtime_error::runtime_error;
  };

  class Node {
    public:

    using Value = std::variant <std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node() = default;
    Node(Array array): value_(std::move(array)) {}
    Node(std::nullptr_t): Node() {}
    Node(bool value): value_(value) {}
    Node(Dict map): value_(std::move(map)) {}
    Node(int value): value_(value) {}
    Node(std::string value): value_(std::move(value)) {}
    Node(double value): value_(value) {}

    const Array & AsArray() const {
      using namespace std::literals;

      if (!IsArray()) throw std::logic_error("value is not an array"s);
      else {
        return std::get < Array > (value_);
      }
    }

    const Dict & AsMap() const {
      using namespace std::literals;

      if (!IsMap()) throw std::logic_error("value is not a dictionary"s);
      else {
        return std::get < Dict > (value_);
      }
    }

    const std::string & AsString() const {
      using namespace std::literals;

      if (!IsString()) throw std::logic_error("value is not a string"s);
      else {
        return std::get < std::string > (value_);
      }
    }

    int AsInt() const {
      using namespace std::literals;

      if (!IsInt()) throw std::logic_error("value is not an int"s);
      else {
        return std::get < int > (value_);
      }
    }

    double AsDouble() const {
      using namespace std::literals;

      if (!IsDouble()) {
        throw std::logic_error("value is not a double"s);
      } 
      else if (IsPureDouble()) {
        return std::get < double > (value_);
      } else {
        return AsInt();
      }
    }

    bool AsBool() const {
      using namespace std::literals;

      if (!IsBool()) {
        throw std::logic_error("value is not a bool"s);
      } 
      else {
        return std::get < bool > (value_);
      }
    }

    bool IsNull() const {
      return std::holds_alternative < std::nullptr_t > (value_);
    }
    bool IsInt() const {
      return std::holds_alternative < int > (value_);
    }
    bool IsDouble() const {
      return IsPureDouble() || IsInt();
    }
    bool IsPureDouble() const {
      return std::holds_alternative < double > (value_);
    }
    bool IsBool() const {
      return std::holds_alternative < bool > (value_);
    }
    bool IsString() const {
      return std::holds_alternative < std::string > (value_);
    }
    bool IsArray() const {
      return std::holds_alternative < Array > (value_);
    }
    bool IsMap() const {
      return std::holds_alternative < Dict > (value_);
    }

    const Value & GetValue() const {
      return value_;
    };

    private: Value value_;
  };

  inline bool operator == (const Node&lhs, const Node & rhs) {
    return lhs.GetValue() == rhs.GetValue();
  }

  inline bool operator != (const Node& lhs, const Node & rhs) {
    return !(lhs == rhs);
  }

  class Document {
    public: explicit Document(Node root): root_(std::move(root)) {}

    const Node& GetRoot() const {
      return root_;
    }

    private: Node root_;
  };

  Document Load(std::istream & input);

  inline bool operator == (const Document & lhs, const Document & rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
  }

  inline bool operator != (const Document & lhs, const Document & rhs) {
    return !(lhs == rhs);
  }

  void Print(const Document & doc, std::ostream & output);
}
