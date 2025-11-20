#pragma once
#include <string>
#include <atomic>

namespace qsteedcpp {

// A unique identifier for a tensor dimension (an "index" or a "leg").
class Index {
private:
    long id_;
    long dim_;
    std::string name_;

    inline static std::atomic<long> next_id_ = 0;
public:
    Index(long dim, std::string name = "") 
        : id_(next_id_.fetch_add(1, std::memory_order_relaxed)), dim_(dim), name_(name) {}

    long id() const { return id_; }
    long dim() const { return dim_; }
    const std::string& name() const { return name_; }

    bool operator==(const Index& other) const {
        return id_ == other.id_;
    }

    bool operator<(const Index& other) const {
        return id_ < other.id_;
    }
};

} // namespace qsteedcpp
