#pragma once

#include <utility>

template <typename T>
class ArrayPtr {
public:
    ArrayPtr() = default;

    explicit ArrayPtr(size_t size) noexcept
        : ptr_(size > 0 ? new T[size] : nullptr) {
    }

    // Запрещаем копирование указателя
    ArrayPtr(const ArrayPtr&) = delete;

    // удаляем присваивание
    ArrayPtr& operator=(const ArrayPtr&) = delete;

    //конструктор перемещения
    ArrayPtr(ArrayPtr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    }

    //перемещающий конструктор присваивания 
    ArrayPtr& operator=(ArrayPtr&& other) noexcept {
        if (this != &other) {
            std::swap(ptr_, other.ptr_);
        }
        return *this;
    }

    ~ArrayPtr() {
        delete[] ptr_;
    }

    T* GetRawPtr() const noexcept {
        return ptr_;
    }

    T* Release() noexcept {
        return std::exchange(ptr_, nullptr);
    }

    explicit operator bool() const {
        return ptr_ != nullptr;
    }

private:
    T* ptr_ = nullptr;
};
