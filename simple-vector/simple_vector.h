#pragma once

#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <assert.h>
#include "array_ptr.h"

struct ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity) : capacity_{ capacity } {}
    size_t capacity_{};
};

inline ReserveProxyObj Reserve(size_t capacity) {
    return ReserveProxyObj(capacity);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;
    SimpleVector() noexcept = default;

    //что бы € не делал, делегирование
    //explicit SimpleVector(size_t size)
    //: SimpleVector(size, Type()) {}
    //не проходит тесты с классом X в main 

    explicit SimpleVector(size_t size)
        : size_{ size },
        capacity_{ size },
        items_{ size } {
    }

    //ѕотому что здесь, как ни изворачивайс€, невозможно сделать move семантику
    //если value - это один объект. я пробовал создавать и перегрузку с универсальной ссылкой
    //и мув-итераторы, что только не пробовал. Ќикак. 
    //»»-шка пишет: "≈сли Type не копируемый, то нельз€ инициализировать все элементы одним значением."
    SimpleVector(size_t size, const Type& value)
        : size_{ size },
        capacity_{ size },
        items_{ size } {
        if (size > 0) {
            std::fill(begin(), end(), value);
        }
    }


    SimpleVector(std::initializer_list<Type> init)
        : size_{ init.size() },
        capacity_{ init.size() },
        items_{ init.size() } {
        if (init.size() > 0) {
            std::copy(init.begin(), init.end(), begin());
        }
    }

    SimpleVector(const ReserveProxyObj& proxy_obj)
        : size_{},
        capacity_{ proxy_obj.capacity_ },
        items_{ proxy_obj.capacity_ } {
    }

    SimpleVector(const SimpleVector& other)
        : size_{ other.size_ },
        capacity_{ other.size_ },
        items_{ other.size_ } {
        if (other.size_ > 0) {
            std::copy(other.begin(), other.end(), begin());
        }
    }

    SimpleVector(SimpleVector&& other)
        : size_{ other.size_ },
        capacity_{ other.size_ },
        items_{ other.size_ } {
        if (other.size_ > 0) {
            items_ = std::move(other.items_);
            other.size_ = 0;
        }
    }

    SimpleVector& operator=(SimpleVector&& other) noexcept {
        if (this != &other) {
            swap(other);
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    SimpleVector& operator =(const SimpleVector<Type>& other) {
        if (this != &other) {
            SimpleVector<Type> temp(other);
            swap(temp);
        }
        return *this;
    }

    ~SimpleVector() = default;

    void PopBack() noexcept {
        if (size_ != 0) {
            --size_;
        }
    }

    void PushBack(Type item) {
        if (size_ < capacity_) {
            *end() = std::move(item);
            ++size_;
        }
        else {

            SimpleVector temp(size_ > 0 ? size_ * 2 : 1);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), temp.begin());
            *(temp.begin() + size_) = std::move(item);
            temp.size_ = size_ + 1;
            swap(temp);
        }
    }

    Iterator Insert(ConstIterator pos, Type value) {
        if (pos < begin() || pos > end()) {
            throw std::out_of_range("Iterator is out of range for insertion");
        }
        size_t index = std::distance(begin(), const_cast<Type*>(pos));
        if (size_ < capacity_) {
            std::copy_backward(std::make_move_iterator(const_cast<Type*>(pos)), std::make_move_iterator(end()), end() + 1);
            *const_cast<Type*>(pos) = std::move(value);
            ++size_;
        }
        else {
            SimpleVector temp(size_ > 0 ? size_ * 2 : 1);
            temp.size_ = size_;
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), temp.begin());
            swap(temp);
            Insert(begin() + index, std::move(value));
        }
        return begin() + index;
    }

    Iterator Erase(ConstIterator pos) {
        if (pos < begin() || pos > end()) {
            throw std::out_of_range("Iterator is out of range for insertion");
        }
        size_t index = std::distance(begin(), const_cast<Type*>(pos));
        std::copy(std::make_move_iterator(const_cast<Type*>(pos) + 1), std::make_move_iterator(end()), const_cast<Type*>(pos));
        --size_;
        return begin() + index;
    }

    void swap(SimpleVector<Type>& other) noexcept { //готов
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        std::swap(items_, other.items_);
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        return *(begin() + index);
    }

    const Type& operator[](size_t index) const noexcept {
        // Ќапишите тело самосто€тельно
        return *(begin() + index);
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("out of range");
        }
        return *(begin() + index);
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("out of range");
        }
        return *(begin() + index);
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size < size_) {
            size_ = new_size;
        }
        else if (new_size > size_) {
            SimpleVector temp(new_size);
            temp.size_ = new_size;
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), temp.begin());
            swap(temp);
        }
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            SimpleVector temp(new_capacity);
            temp.size_ = size_;
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), temp.begin());
            swap(temp);
        }
    }

    Iterator begin() noexcept {
        return items_.GetRawPtr();
    }

    Iterator end() noexcept {
        return items_.GetRawPtr() + size_;
    }

    ConstIterator begin() const noexcept {
        return items_.GetRawPtr();
    }

    ConstIterator end() const noexcept {
        return items_.GetRawPtr() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return items_.GetRawPtr();

    }

    ConstIterator cend() const noexcept {
        return items_.GetRawPtr() + size_;
    }

private:
    size_t size_{};
    size_t capacity_{};
    ArrayPtr<Type> items_{};
};

template <typename Type>
inline bool  operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator >(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator <=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator >=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

template <typename Type>
inline bool operator ==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() == rhs.GetSize()) {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }
    return false;
}

template <typename Type>
inline bool operator !=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}