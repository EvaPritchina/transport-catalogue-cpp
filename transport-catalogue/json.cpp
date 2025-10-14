#include "json.h"

using namespace std;

namespace json {

  namespace {

    Node LoadNode(istream & input);
    std::string LoadLiteral(std::istream & input) {
      std::string str;
      while (std::isalpha(input.peek())) {
        str.push_back(static_cast < char > (input.get()));
      }
      return str;
    }

    Node LoadArray(std::istream & input) {
      std::vector < Node > result;
      for (char current_char; input >> current_char && current_char != ']';) {
        if (current_char != ',') {
          input.putback(current_char);
        }
        result.push_back(LoadNode(input));
      }
      if (!input) {
        throw ParsingError("Array parsing error"s);
      }
      return Node(result);
    }

    Node LoadNull(std::istream & input) {
      if (auto literal = LoadLiteral(input); literal == "null"sv) {
        return Node {
          nullptr
        };
      } else {
        throw ParsingError("Failed to parse '"s + literal + "' as null"s);
      }
    }

    Node LoadBool(std::istream & input) {
      const auto str = LoadLiteral(input);
      if (str == "true"
        sv) {
        return Node {
          true
        };
      } else if (str == "false"
        sv) {
        return Node {
          false
        };
      } else {
        throw ParsingError("Failed to parse '"s + str + "' as bool"s);
      }
    }

    Node LoadNumber(std::istream & input) {
      std::string parsed_num;
      auto read_char = [ & parsed_num, & input] {
        parsed_num += static_cast < char > (input.get());
        if (!input) {
          throw ParsingError("Failed to read number from stream"s);
        }
      };
      auto read_digits = [ & input, read_char] {
        if (!std::isdigit(input.peek())) {
          throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
          read_char();
        }
      };
      if (input.peek() == '-') {
        read_char();
      }
      if (input.peek() == '0') {
        read_char();
      } else {
        read_digits();
      }
      bool is_int = true;
      if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
      }
      if (int current_char = input.peek(); current_char == 'e' || current_char == 'E') {
        read_char();
        if (current_char = input.peek(); current_char == '+' || current_char == '-') {
          read_char();
        }
        read_digits();
        is_int = false;
      }

      try {
        if (is_int) {
          try {
            return Node(std::stoi(parsed_num));
          } catch (...) {}
        }
        return Node(std::stod(parsed_num));
      } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
      }
    }

    std::string LoadString(std::istream & input) {
      using namespace std::literals;
      auto it = std::istreambuf_iterator < char > (input);
      auto end = std::istreambuf_iterator < char > ();
      std::string s;
      while (true) {
        if (it == end) {
          throw ParsingError("String parsing error");
        }
        const char ch = * it;
        if (ch == '"') {
          ++it;
          break;
        } else if (ch == '\\') {
          ++it;
          if (it == end) {
            throw ParsingError("String parsing error");
          }
          const char escaped_char = * (it);
          switch (escaped_char) {
          case 'n':
            s.push_back('\n');
            break;
          case 't':
            s.push_back('\t');
            break;
          case 'r':
            s.push_back('\r');
            break;
          case '"':
            s.push_back('"');
            break;
          case '\\':
            s.push_back('\\');
            break;
          default:
            throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
          }
        } else if (ch == '\n' || ch == '\r') {
          throw ParsingError("Unexpected end of line"s);
        } else {
          s.push_back(ch);
        }
        ++it;
      }
      return s;
    }

    Node LoadDict(std::istream & input) {
      Dict dict;
      for (char current_char; input >> current_char && current_char != '}';) {
        if (current_char == '"') {
          std::string key = LoadString(input);
          if (input >> current_char && current_char == ':') {
            if (dict.find(key) != dict.end()) {
              throw ParsingError("Duplicate key '"
                s + key + "' have been found");
            }
            dict.emplace(std::move(key), LoadNode(input));
          } else {
            throw ParsingError(": is expected but '"
              s + current_char + "' has been found"
              s);
          }
        } else if (current_char != ',') {
          throw ParsingError(R "(',' is expected but ')"
            s + current_char + "' has been found"
            s);
        }
      }
      if (!input) {
        throw ParsingError("Dictionary parsing error"
          s);
      }
      return Node(dict);
    }

    Node LoadNode(std::istream & input) {
      char c;
      if (!(input >> c)) {
        throw ParsingError(""
          s);
      }
      switch (c) {
      case '[':
        return LoadArray(input);
      case '{':
        return LoadDict(input);
      case '"':
        return LoadString(input);
      case 't':
      case 'f':
        input.putback(c);
        return LoadBool(input);
      case 'n':
        input.putback(c);
        return LoadNull(input);
      default:
        input.putback(c);
        return LoadNumber(input);
      }
    }
  }

  Document Load(std::istream & input) {
    return Document {
      LoadNode(input)
    };
  }

  struct PrintContext {
    std::ostream & out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
      for (int i = 0; i < indent; ++i) {
        out.put(' ');
      }
    }

    [[nodiscard]] PrintContext Indented() const {
      return {
        out,
        indent_step,
        indent_step + indent
      };
    }
  };

  void PrintNode(const Node & node, const PrintContext & ctx);

  template < typename Value >
  void PrintValue(const Value & value, const PrintContext & ctx) {
    ctx.out << value;
    }

  void PrintValue(const std::string & value,
    const PrintContext & ctx) {
    ctx.out.put('"');
    for (const char current_char: value) {
      switch (current_char) {
      case '\r':
        ctx.out << R "(\r)";
        break;
      case '\n':
        ctx.out << R "(\n)";
        break;
      case '\t':
        ctx.out << R "(\t)";
        break;
      case '"':
        ctx.out << R "(\")";
        break;
      case '\\':
        ctx.out << R "(\\)";
        break;
      default:
        ctx.out.put(current_char);
        break;
      }
    }
    ctx.out.put('"');
  }

  void PrintValue(const std::nullptr_t &, const PrintContext & ctx) {
    ctx.out << "null"s;
  }

  void PrintValue(bool value, const PrintContext & ctx) {
    ctx.out << std::boolalpha << value;
  }

  [[maybe_unused]] void PrintValue(Array nodes, const PrintContext & ctx) {
    std::ostream & out = ctx.out;
    out << "[\n"
    sv;

    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const Node & node: nodes) {
      if (first) {
        first = false;
      } else {
        out << ",\n"
        sv;
      }
      inner_ctx.PrintIndent();
      PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put(']');
  }

  [[maybe_unused]] void PrintValue(Dict nodes, const PrintContext & ctx) {
    std::ostream & out = ctx.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto & [key, node]: nodes) {
      if (first) {
        first = false;
      } else {
        out << ",\n"sv;
      }
      inner_ctx.PrintIndent();
      PrintNode(key, inner_ctx);
      out << ": "sv;
      PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put('}');
  }

  void PrintNode(const Node & node, const PrintContext & ctx) {
    std::visit(
      [&ctx](const auto & value) {
        PrintValue(value, ctx);
      },
      node.GetValue());
  }

  void Print(const Document & doc, std::ostream & output) {
    PrintNode(doc.GetRoot(), PrintContext {
      output
    });
  }
}
