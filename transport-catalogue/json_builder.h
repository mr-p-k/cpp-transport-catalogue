#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include <stack>

#include "json.h"

namespace json {

class Builder final {
private:
    class BaseContext;

    class DictItemContext;

    class DictValueContext;

    class ArrayItemContext;

public:
    Builder();

    DictValueContext Key(std::string key);

    BaseContext Value(Node::Value value);

    DictItemContext StartDict();

    ArrayItemContext StartArray();

    BaseContext EndDict();

    BaseContext EndArray();

    Node Build();

private:
    void AddToDict(Node value);

    void AddToArray(Node* parent, Node value);

    Node root_;

    std::stack<Node*> nodes_stack_;

    std::stack<Dict::iterator> dict_stack_;

    class BaseContext {
    public:
        explicit BaseContext(Builder& builder);

        BaseContext(const BaseContext&) = delete;

        BaseContext(BaseContext&& base) noexcept;

        BaseContext& operator=(const BaseContext&) = delete;

        BaseContext& operator=(BaseContext&&) noexcept = delete;

        ~BaseContext() = default;

        Node Build();

        DictValueContext Key(std::string key);

        BaseContext Value(Node::Value value);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        BaseContext EndDict();

        BaseContext EndArray();

    private:
        Builder& builder_;
    };

    class DictItemContext final : public BaseContext {
    public:
        explicit DictItemContext(BaseContext base);

        Node Build() = delete;

        BaseContext Value(Node::Value value) = delete;

        DictItemContext StartDict() = delete;

        ArrayItemContext StartArray() = delete;

        BaseContext EndArray() = delete;
    };

    class DictValueContext final : public BaseContext {
    public:
        explicit DictValueContext(BaseContext base);

        DictItemContext Value(Node::Value value);

        Node Build() = delete;

        DictValueContext Key(std::string key) = delete;

        BaseContext EndDict() = delete;

        BaseContext EndArray() = delete;
    };

    class ArrayItemContext final : public BaseContext {
    public:
        explicit ArrayItemContext(BaseContext base);

        ArrayItemContext Value(Node::Value value);

        Node Build() = delete;

        DictValueContext Key(std::string key) = delete;

        BaseContext EndDict() = delete;
    };
};

}  // namespace json

#endif  // JSON_BUILDER_H
