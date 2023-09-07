#pragma once

#include <cassert>
#include <initializer_list>
#include "array_ptr.h"
#include <stdexcept>
#include <iostream>

struct ReserveProxyObj {
    size_t p_capacity;
};

ReserveProxyObj Reserve(size_t capacity) {
    return ReserveProxyObj{capacity};
}

template<typename Type>
class SimpleVector {
public:
    using Iterator = Type *;
    using ConstIterator = const Type *;

    SimpleVector() noexcept = default;


    SimpleVector(SimpleVector &&other) noexcept: elements_(other.capacity_) { swap(other); }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_data(new_capacity);

            std::move(elements_.Get(), elements_.Get() + size_, new_data.Get());

            elements_.swap(new_data);

            capacity_ = new_capacity;
        }
    };

    explicit SimpleVector(ReserveProxyObj obj) : elements_(obj.p_capacity), size_(0), capacity_(obj.p_capacity) {}

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) {
        ArrayPtr<Type> to_swap(size);
        std::fill(to_swap.Get(), to_swap.Get() + size, 0);
        to_swap.swap(elements_);
        size_ = size;
        capacity_ = size;
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type &value) {
        ArrayPtr<Type> to_swap(size);
        std::fill(to_swap.Get(), to_swap.Get() + size, value);
        elements_.swap(to_swap);
        size_ = size;
        capacity_ = size;
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {
        ArrayPtr<Type> to_swap(init.size());
        std::copy(init.begin(), init.end(), to_swap.Get());
        to_swap.swap(elements_);
        size_ = init.size();
        capacity_ = init.size();
    }


    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type &operator[](size_t index) noexcept {
        return elements_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type &operator[](size_t index) const noexcept {
        return elements_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type &At(size_t index) {
        if (index >= size_) throw std::out_of_range("Out of range");
        return elements_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type &At(size_t index) const {
        if (index >= size_) throw std::out_of_range("Out of range");
        return elements_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        } else if (new_size <= capacity_) {
            std::for_each(elements_.Get() + size_, elements_.Get() + new_size, [](auto& i){ i = Type{}; });
            size_ = new_size;
        } else {
            ArrayPtr<Type> new_mas(new_size);
            std::move(elements_.Get(), elements_.Get() + capacity_, new_mas.Get());
            std::for_each(new_mas.Get() + capacity_, new_mas.Get() + new_size, [](auto& i){ i = Type{}; });

            elements_.swap(new_mas);
            capacity_ = new_size;
            size_ = new_size;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return elements_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return elements_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return elements_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return elements_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return elements_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return elements_.Get() + size_;
    }

    SimpleVector(const SimpleVector &other) {
        SimpleVector<Type> temp(other.GetSize());
        std::copy(other.begin(), other.end(), temp.begin());
        this->swap(temp);
    }

    SimpleVector &operator=(const SimpleVector &rhs) {
        if (*this != rhs) {
            SimpleVector<Type> to_swap(rhs.GetSize());
            std::copy(rhs.begin(), rhs.end(), to_swap.begin());
            this->swap(to_swap);
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type &item) {
        if (size_ != capacity_) {
            elements_[size_] = item;
            ++size_;
        } else {
            auto new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            ArrayPtr<Type> new_data(new_capacity);
            std::copy(elements_.Get(), elements_.Get() + size_, new_data.Get());
            new_data[size_] = item;
            elements_.swap(new_data);
            ++size_;
            capacity_ = new_capacity;
        }
    }

    void PushBack(Type &&item) {
        if (size_ != capacity_) {
            elements_[size_] = std::move(item);
            ++size_;
        } else {
            auto new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            ArrayPtr<Type> new_data(new_capacity);
            std::move(elements_.Get(), elements_.Get() + size_, new_data.Get());
            new_data[size_] = std::move(item);
            elements_.swap(new_data);
            ++size_;
            capacity_ = new_capacity;
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type &value) {
        SimpleVector<Type>::Iterator to_return;
        if (size_ < capacity_) {
            to_return = std::copy_backward(pos, this->cend(), this->end() + 1);
            *(--to_return) = value;
            ++size_;
        } else {
            ArrayPtr<Type> temp(size_ + 1);
            to_return = (std::copy(this->cbegin(), pos, &temp[0]));
            *to_return = value;
            std::copy(pos, cend(), to_return + 1);
            elements_.swap(temp);
            ++size_;
            capacity_ = std::max(capacity_ * 2, size_ + 1);
        }

        return to_return;
    }

    Iterator Insert(ConstIterator pos, Type &&value) {

        auto t = pos - elements_.Get();

        if (size_ != capacity_) {
            std::move_backward(const_cast<Iterator>(pos), end(), end() + 1);

            elements_[pos - elements_.Get()] = std::move(value);
            ++size_;

            return &elements_[t];
        } else {
            ArrayPtr<Type> temp(capacity_ == 0 ? 1 : capacity_ * 2);
            std::move(elements_.Get(), elements_.Get() + size_, temp.Get());
            temp[t] = std::move(value);
            std::move(elements_.Get() + t, elements_.Get() + size_, temp.Get() + size_ + 1);
            elements_.swap(temp);
            ++size_;
            capacity_ = std::max(capacity_ * 2, size_ + 1);

            return &elements_[t];
        }

    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_ != 0) {
            Resize(size_ - 1);
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        auto t = pos - elements_.Get();

        std::move(elements_.Get() + t + 1, elements_.Get() + size_, elements_.Get() + t);
        --size_;
        return &elements_[t];
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector &other) noexcept {
        elements_.swap(other.elements_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:
    ArrayPtr<Type> elements_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template<typename Type>
inline bool operator==(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    if (lhs.GetSize() == rhs.GetSize()) {
        if (lhs >= rhs && rhs >= lhs) {
            return true;
        }
    }
    return false;
}

template<typename Type>
inline bool operator!=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return !(lhs == rhs);
    return true;
}

template<typename Type>
inline bool operator<(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename Type>
inline bool operator<=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return !(rhs < lhs);
}

template<typename Type>
inline bool operator>(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return rhs < lhs;
}

template<typename Type>
inline bool operator>=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
    return !(lhs < rhs);
}