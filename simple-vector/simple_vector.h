#pragma once

#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <algorithm>
#include <utility>
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

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : size_{ size },
        capacity_{ size },
        items_{ size > 0 ? new Type[size]() : nullptr } {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : size_{ size },
        capacity_{ size },
        items_{ size > 0 ? new Type[size]() : nullptr } {
        // Напишите тело конструктора самостоятельно
        if (size > 0) {
            std::fill(begin(), end(), value);
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        : size_{ init.size() },
        capacity_{ init.size() },
        items_{ init.size() > 0 ? new Type[init.size()]() : nullptr } {
        // Напишите тело конструктора самостоятельно
        if (init.size() > 0) {
            std::copy(init.begin(), init.end(), begin());
        }
    }

    //конструктор с резервированием объёма
    SimpleVector(const ReserveProxyObj& proxy_obj)
        : size_{},
        capacity_{ proxy_obj.capacity_ },
        items_{ proxy_obj.capacity_ > 0 ? new Type[capacity_]() : nullptr } {
    }

    SimpleVector(const SimpleVector& other)
        : size_{ other.size_ },
        capacity_{ other.size_ },
        items_{ other.size_ > 0 ? new Type[other.size_] : nullptr } {
        if (other.size_ > 0) {
            std::copy(other.begin(), other.end(), begin());
        }
    }

    SimpleVector(SimpleVector&& other)
        : size_{ other.size_ },
        capacity_{ other.size_ },
        items_{ other.size_ > 0 ? new Type[other.size_] : nullptr } {
        if (other.size_ > 0) {
            std::copy(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()), begin());
            other.size_ = 0;
        }
    }

    SimpleVector& operator=(SimpleVector&& other) noexcept {
        if (this != &other) {
            size_ = other.size_;
            capacity_ = other.capacity_;
            items_ = std::move(other.items_);  // Перемещаем ArrayPtr
            other.size_ = 0; // Обнуляем other
            other.capacity_ = 0;
            // ArrayPtr сам удалится в other
        }
        return *this;
    }

    SimpleVector& operator =(const SimpleVector<Type>& other) {
        //copy and swap
        SimpleVector temp(other);
        swap(temp);
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
            //тут снова идиома copy and swap
            //делаем временный простой_вектор через конструктор
            //копируем туда данные, меняем значения
            //затем просто меняем вектора

            SimpleVector temp(size_ > 0 ? size_ * 2 : 1);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), temp.begin());
            *(temp.begin() + size_) = std::move(item);
            temp.size_ = size_ + 1;
            swap(temp);
        }
    }

    // Вставляет значение value в позицию pos.
// Возвращает итератор на вставленное значение
// Если перед вставкой значения вектор был заполнен полностью,
// вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, Type value) {
        // Напишите тело самостоятельно
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
        // Напишите тело самостоятельно
        size_t index = std::distance(begin(), const_cast<Type*>(pos));
        std::copy(std::make_move_iterator(const_cast<Type*>(pos) + 1), std::make_move_iterator(end()), const_cast<Type*>(pos));
        --size_;
        return begin() + index;
    }

    void swap(SimpleVector<Type>& other) noexcept { //готов
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        //тут свап работает с ArrayPtr изза перемещающих конструкторов
        std::swap(items_, other.items_);
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        // Напишите тело самостоятельно
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        // Напишите тело самостоятельно
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        // Напишите тело самостоятельно
        return *(begin() + index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        // Напишите тело самостоятельно
        return *(begin() + index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        // Напишите тело самостоятельно
        if (index >= size_) {
            throw std::out_of_range("out of range");
        }
        return *(begin() + index);
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        // Напишите тело самостоятельно
        if (index >= size_) {
            throw std::out_of_range("out of range");
        }
        return *(begin() + index);
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        // Напишите тело самостоятельно
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        // Напишите тело самостоятельно
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

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        // Напишите тело самостоятельно
        return items_.GetRawPtr();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        // Напишите тело самостоятельно
        return items_.GetRawPtr() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        // Напишите тело самостоятельно
        return items_.GetRawPtr();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        // Напишите тело самостоятельно
        return items_.GetRawPtr() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        // Напишите тело самостоятельно
        return items_.GetRawPtr();

    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        // Напишите тело самостоятельно
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