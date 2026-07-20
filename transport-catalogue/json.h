#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;

using Array = std::vector<Node>;

using Value =
    std::variant<std::nullptr_t, bool, int, double, std::string, Array, Dict>;

using Number = std::variant<int, double>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final : private Value {
public:
    using variant::variant;  // make public constructors from variant base class

    /* begin check value type */
    [[nodiscard]] bool IsNull() const;

    [[nodiscard]] bool IsBool() const;

    [[nodiscard]] bool IsInt() const;

    // Возвращает true, если в Node хранится int либо double.
    [[nodiscard]] bool IsDouble() const;

    // Возвращает true, если в Node хранится double.
    [[nodiscard]] bool IsPureDouble() const;

    [[nodiscard]] bool IsString() const;

    [[nodiscard]] bool IsArray() const;

    [[nodiscard]] bool IsMap() const;
    /* end check value type */

    /* begin getters */
    [[nodiscard]] const Value& GetValue() const;

    [[nodiscard]] bool AsBool() const;

    [[nodiscard]] int AsInt() const;

    // Возвращает значение типа double, если внутри хранится double либо int. В
    // последнем случае возвращается приведённое в double значение.
    [[nodiscard]] double AsDouble() const;

    [[nodiscard]] const std::string& AsString() const;

    [[nodiscard]] const Array& AsArray() const;

    [[nodiscard]] const Dict& AsMap() const;
    /* end getters */
};

bool operator==(const Node& lhs, const Node& rhs);

bool operator!=(const Node& lhs, const Node& rhs);

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);

bool operator==(const Document& lhs, const Document& rhs);

bool operator!=(const Document& lhs, const Document& rhs);

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

template <typename T>
void PrintValue(const T& value, PrintContext context) {
    std::ostream& out = context.out;

    out << value;
}

void PrintValue(std::nullptr_t, PrintContext context);

void PrintValue(bool value, PrintContext context);

void PrintValue(const std::string& value, PrintContext context);

void PrintValue(const Array& array, PrintContext context);

void PrintValue(const Dict& map, PrintContext context);

void PrintNode(const Node& node, PrintContext context);

void Print(const Document& doc, std::ostream& output);

}  // namespace json