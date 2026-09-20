#pragma once

#include <iterator>
#include <utility>

namespace ranges {

template <typename It>
class Range {
public:
    using ValueType = typename std::iterator_traits<It>::value_type;

    Range(It begin, It end) : begin_(std::move(begin)), end_(std::move(end)) {
    }

    [[nodiscard]] It begin() const {
        return begin_;
    }

    [[nodiscard]] It end() const {
        return end_;
    }

private:
    It begin_;
    It end_;
};

template <typename C>
auto AsRange(const C& container) {
    return Range{container.begin(), container.end()};
}

}  // namespace ranges