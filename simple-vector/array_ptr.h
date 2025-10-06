#pragma once

#include <stdexcept> // содержит std::logic_error

template <typename T>
class ArrayPtr {
public:
    ArrayPtr() = default;

    explicit ArrayPtr(T* raw_ptr) noexcept
        : ptr_(raw_ptr) {
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
            delete[] ptr_;
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
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
        T* p = ptr_;
        ptr_ = nullptr;
        return p;
    }

    explicit operator bool() const {
        return ptr_ != nullptr;
    }

private:
    T* ptr_ = nullptr;
};
