#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
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
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node{std::stoi(parsed_num)};
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node{std::stod(parsed_num)};
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
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
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s +
                                       std::to_string(escaped_char));
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами
            // \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в
            // результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node{s};
}

Node LoadBool(istream& input) {
    std::string bool_str;

    char c = 0;
    while (input >> c) {
        if (c == ',' || c == '}' || c == ']') {
            input.putback(c);

            break;
        }

        bool_str.push_back(c);
    }

    using std::string_view_literals::operator""sv;

    bool value = true;
    if (bool_str == "true"sv) {
        value = true;
    } else if (bool_str == "false"sv) {
        value = false;
    } else {
        throw ParsingError("expect true or false, but receive: "s + bool_str);
    }

    return Node{value};
}

Node LoadArray(istream& input) {
    Array result;

    char c = 0;
    while (input >> c && c != ']') {
        if (c != ',') {
            input.putback(c);
        }

        result.push_back(LoadNode(input));
    }

    if (c != ']') {
        throw ParsingError("] expected");
    }

    return {std::move(result)};
}

Node LoadDict(istream& input) {
    Dict result;

    char c = 0;
    while (input >> c && c != '}') {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({std::move(key), LoadNode(input)});
    }

    if (c != '}') {
        throw ParsingError("} expected");
    }

    return {std::move(result)};
}

Node LoadNull(istream& input) {
    std::string null_str;

    char c = 0;
    while (input >> c) {
        if (c == ',' || c == '}' || c == ']') {
            input.putback(c);

            break;
        }

        null_str.push_back(c);
    }

    using std::string_literals::operator""s;

    return (null_str == "null"s)
               ? Node{nullptr}
               : throw ParsingError("expect null, but receive: "s + null_str);
}

Node LoadNode(istream& input) {
    using std::string_literals::operator""s;

    char c = 0;
    input >> c;

    switch (c) {
        case '[':
            return LoadArray(input);
            break;
        case '{':
            return LoadDict(input);
            break;
        case 'n':
            input.putback(c);
            return LoadNull(input);
            break;
        case '"':
            return LoadString(input);
            break;
        case 't':
        case 'f':
            input.putback(c);
            return LoadBool(input);
            break;
        case ']':
            throw ParsingError("] is unexpected"s);
            break;
        case '}':
            throw ParsingError("} is unexpected"s);
        default:
            input.putback(c);
            return LoadNumber(input);
            break;
    }
}

}  // namespace

/*
 * begin check value type
 */
bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(*this);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(*this);
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(*this);
}

bool Node::IsDouble() const {
    return std::holds_alternative<int>(*this) ||
           std::holds_alternative<double>(*this);
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(*this);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(*this);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(*this);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(*this);
}
/*
 * end check value type
 */

/*
 * begin getters
 */
const Value& Node::GetValue() const {
    return *this;
}

bool Node::AsBool() const {
    return IsBool()
               ? std::get<bool>(*this)
               : throw std::logic_error("bool " + std::to_string(__LINE__));
}

int Node::AsInt() const {
    return IsInt() ? std::get<int>(*this) : throw std::logic_error("int");
}

double Node::AsDouble() const {
    double value = 0.0;
    if (IsInt()) {
        value = static_cast<double>(std::get<int>(*this));
    } else if (IsDouble() || IsPureDouble()) {
        value = std::get<double>(*this);
    } else {
        throw std::logic_error("double");
    }

    return value;
}

const string& Node::AsString() const {
    return IsString() ? std::get<std::string>(*this)
                      : throw std::logic_error("string");
}

const Array& Node::AsArray() const {
    return IsArray() ? std::get<Array>(*this) : throw std::logic_error("array");
}

const Dict& Node::AsMap() const {
    return IsMap() ? std::get<Dict>(*this) : throw std::logic_error("dict");
}
/*
 * end getters
 */

bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.GetValue() == rhs.GetValue();
}

bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

Document::Document(Node root) : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void PrintValue(std::nullptr_t, PrintContext context) {
    using std::string_view_literals::operator""sv;

    std::ostream& out = context.out;

    out << "null"sv;
}

void PrintValue(bool value, PrintContext context) {
    using std::string_view_literals::operator""sv;

    std::ostream& out = context.out;

    out << (value ? "true"sv : "false"sv);
}

void PrintValue(const std::string& value, PrintContext context) {
    using std::string_view_literals::operator""sv;

    std::ostream& out = context.out;

    std::string_view value_view{value};

    out << "\""sv;
    for (auto c : value_view) {
        switch (c) {
            case '\\':
                out << "\\\\"sv;
                break;
            case '\t':
                out << "\\t"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '"':
                out << "\\\""sv;
                break;
            case '\r':
                out << "\\r"sv;
                break;
            default:
                out << c;
                break;
        }
    }
    out << "\""sv;
}

void PrintValue(const Array& array, PrintContext context) {
    using std::string_view_literals::operator""sv;

    std::ostream& out = context.out;

    out << "["sv;

    bool is_first = true;
    for (const auto& item : array) {
        if (!is_first) {
            out << ","sv;
        } else {
            is_first = false;
        }

        PrintNode(item, context);
    }

    out << "]"sv;
}

void PrintValue(const Dict& map, PrintContext context) {
    using std::string_view_literals::operator""sv;

    std::ostream& out = context.out;

    out << "{"sv;

    bool is_first = true;
    for (const auto& pair : map) {
        if (!is_first) {
            out << ","sv;
        } else {
            is_first = false;
        }

        PrintValue(pair.first, context);
        out << ":"sv;
        PrintNode(pair.second, context);
    }

    out << "}"sv;
}

void PrintNode(const Node& node, PrintContext context) {
    // context.PrintIndent();
    auto func = [&context](const auto& value) { PrintValue(value, context); };

    std::visit(func, node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    const auto& root = doc.GetRoot();

    PrintNode(root, PrintContext{output, 4, 0});
}

}  // namespace json