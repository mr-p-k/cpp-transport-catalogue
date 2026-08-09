#include "json_builder.h"

#include <utility>

namespace json {

using std::string_literals::operator""s;

Builder::BaseContext::BaseContext(Builder& builder) : builder_(builder) {
}

Builder::BaseContext::BaseContext(BaseContext&& base) noexcept
    : builder_(base.builder_) {
}

Node Builder::BaseContext::Build() {
    return builder_.Build();
}

Builder::DictValueContext Builder::BaseContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder::BaseContext Builder::BaseContext::Value(Node::Value value) {
    return builder_.Value(std::move(value));
}

Builder::DictItemContext Builder::BaseContext::StartDict() {
    return builder_.StartDict();
}

Builder::ArrayItemContext Builder::BaseContext::StartArray() {
    return builder_.StartArray();
}

Builder::BaseContext Builder::BaseContext::EndDict() {
    return builder_.EndDict();
}

Builder::BaseContext Builder::BaseContext::EndArray() {
    return builder_.EndArray();
}

Builder::Builder() : root_(nullptr) {
    nodes_stack_.emplace(&root_);
}

Builder::DictValueContext Builder::Key(std::string key) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Key on empty context"s);
    }

    const Node* node = nodes_stack_.top();

    if (node->IsDict()) {
        auto& dict = const_cast<Dict&>(node->AsDict());

        if (!dict_stack_.empty()) {
            auto latest_dict_it = dict_stack_.top();

            if (auto it = dict.find(latest_dict_it->first); it != dict.end()) {
                throw std::logic_error("key not finished"s);
            }
        }

        auto [it, is_ok] = dict.emplace(std::move(key), Node{});

        dict_stack_.emplace(it);
    } else {
        throw std::logic_error("Key call after StartDict only"s);
    }

    return DictValueContext{BaseContext{*this}};
}

Builder::BaseContext Builder::Value(Node::Value value) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Add value on empty context"s);
    }

    const auto kVisitor = [](auto&& val) -> Node {
        return Node(std::forward<decltype(val)>(val));
    };

    const Node* node = nodes_stack_.top();

    if (node == &root_ && root_ == nullptr) {
        root_ = std::visit(kVisitor, value);

        nodes_stack_.pop();
    } else {
        if (node->IsArray()) {
            auto& arr_node = const_cast<Array&>(node->AsArray());

            arr_node.emplace_back(std::visit(kVisitor, value));
        } else if (node->IsDict()) {
            if (dict_stack_.empty()) {
                throw std::logic_error("value to dict without key"s);
            }

            auto dict_it = dict_stack_.top();

            dict_it->second = std::visit(kVisitor, value);

            dict_stack_.pop();
        } else {
            throw std::logic_error("value not in right context"s);
        }
    }

    return BaseContext{*this};
}

Builder::DictItemContext Builder::StartDict() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("StartDict on empty context"s);
    }

    Node dict_node{Dict{}};
    Node* node = nodes_stack_.top();

    if (node == &root_ && root_ == nullptr) {
        root_ = std::move(dict_node);
    } else {
        if (node->IsArray()) {
            AddToArray(node, std::move(dict_node));
        } else if (node->IsDict() && !dict_stack_.empty()) {
            AddToDict(std::move(dict_node));
        } else {
            throw std::logic_error("StartDict not in right context"s);
        }
    }

    return DictItemContext{BaseContext{*this}};
}

Builder::ArrayItemContext Builder::StartArray() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("StartArray on empty context"s);
    }

    Node arr_node{Array{}};

    Node* node = nodes_stack_.top();

    if (node == &root_ && root_ == nullptr) {
        root_ = std::move(arr_node);
    } else {
        if (node->IsArray()) {
            AddToArray(node, std::move(arr_node));
        } else if (node->IsDict() && !dict_stack_.empty()) {
            AddToDict(std::move(arr_node));
        } else {
            throw std::logic_error("StartArray not in right context"s);
        }
    }

    return ArrayItemContext{BaseContext{*this}};
}

Builder::BaseContext Builder::EndDict() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("EndDict on empty context"s);
    }

    const Node* node = nodes_stack_.top();

    if (node->IsDict()) {
        nodes_stack_.pop();
    } else {
        throw std::logic_error("EndDict while not dict context"s);
    }

    return BaseContext{*this};
}

Builder::BaseContext Builder::EndArray() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("EndArray on empty context"s);
    }

    const Node* node = nodes_stack_.top();

    if (node->IsArray()) {
        nodes_stack_.pop();
    } else {
        throw std::logic_error("EndArray not in array context"s);
    }

    return BaseContext{*this};
}

Node Builder::Build() {
    if (!nodes_stack_.empty() || !dict_stack_.empty()) {
        throw std::logic_error("elements not finished"s);
    }

    return root_;
}

void Builder::AddToDict(Node value) {
    if (dict_stack_.empty()) {
        throw std::logic_error("in dict first add key, than value"s);
    }

    auto dict_it = dict_stack_.top();

    dict_it->second = std::move(value);

    dict_stack_.pop();

    nodes_stack_.emplace(&dict_it->second);
}

void Builder::AddToArray(Node* parent, Node value) {
    auto& arr_ref = const_cast<Array&>(parent->AsArray());

    arr_ref.emplace_back(value);

    nodes_stack_.emplace(&arr_ref.back());
}

Builder::DictItemContext::DictItemContext(BaseContext base)
    : BaseContext(std::move(base)) {
}

Builder::DictValueContext::DictValueContext(BaseContext base)
    : BaseContext(std::move(base)) {
}

Builder::DictItemContext Builder::DictValueContext::Value(Node::Value value) {
    return DictItemContext{Builder::BaseContext::Value(std::move(value))};
}

Builder::ArrayItemContext::ArrayItemContext(BaseContext base)
    : BaseContext(std::move(base)) {
}

Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
    return ArrayItemContext{BaseContext::Value(std::move(value))};
}

}  // namespace json