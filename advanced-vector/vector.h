#pragma once

#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <memory>
#include <algorithm>

template<typename T>
class RawMemory {
public:
    RawMemory() = default;

    explicit RawMemory(size_t capacity)
            : buffer_(Allocate(capacity)), capacity_(capacity) {
    }

    ~RawMemory() {
        Deallocate(buffer_);
    }

    T *operator+(size_t offset) noexcept {
        assert(offset <= capacity_);
        return buffer_ + offset;
    }

    const T *operator+(size_t offset) const noexcept {
        return const_cast<RawMemory &>(*this) + offset;
    }

    const T &operator[](size_t index) const noexcept {
        return const_cast<RawMemory &>(*this)[index];
    }

    T &operator[](size_t index) noexcept {
        assert(index < capacity_);
        return buffer_[index];
    }

    void Swap(RawMemory &other) noexcept {
        std::swap(buffer_, other.buffer_);
        std::swap(capacity_, other.capacity_);
    }

    const T *GetAddress() const noexcept {
        return buffer_;
    }

    T *GetAddress() noexcept {
        return buffer_;
    }

    [[nodiscard]] size_t Capacity() const {
        return capacity_;
    }

private:
    static T *Allocate(size_t n) {
        return n != 0 ? static_cast<T *>(operator new(n * sizeof(T))) : nullptr;
    }

    static void Deallocate(T *buf) noexcept {
        operator delete(buf);
    }

    T *buffer_ = nullptr;
    size_t capacity_ = 0;
};

template<typename T>
class Vector {
public:
    using iterator = T *;
    using const_iterator = const T *;

    Vector() = default;

    explicit Vector(size_t size)
            : data_(size),
              size_(size) {
        std::uninitialized_value_construct_n(data_.GetAddress(), size);
    }

    Vector(const Vector &other)
            : data_(other.size_),
              size_(other.size_) {
        std::uninitialized_copy_n(other.data_.GetAddress(), size_, data_.GetAddress());
    }

    Vector &operator=(const Vector &other) {
        if (this != &other) {
            if (other.size_ > data_.Capacity()) {
                Vector buf(other);
                Swap(buf);
            } else {
                std::copy_n(other.data_.GetAddress(), std::min(size_, other.size_), data_.GetAddress());

                if (size_ > other.size_) {
                    std::destroy_n(data_ + other.size_, size_ - other.size_);
                } else {
                    std::uninitialized_copy_n(other.data_ + size_, other.size_ - size_, data_ + size_);
                }
                size_ = other.size_;
            }
        }

        return *this;
    }

    Vector(Vector &&other) noexcept {
        Swap(other);
    }

    Vector &operator=(Vector &&other) noexcept {
        if (this != &other) {
            Swap(other);
        }
        return *this;
    }

    ~Vector() {
        std::destroy_n(data_.GetAddress(), size_);
    }

    iterator begin() noexcept;

    iterator end() noexcept;

    const_iterator begin() const noexcept;

    const_iterator end() const noexcept;

    const_iterator cbegin() const noexcept;

    const_iterator cend() const noexcept;

    template<typename... Args>
    iterator Emplace(const_iterator pos, Args &&... args) {
        assert(pos >= begin() && pos <= end());
        auto result = const_cast<iterator>(pos);

        if (result == end()) {
            return &EmplaceBack(std::forward<Args>(args)...);
        } else if (size_ == Capacity()) {
            auto distance = std::distance(begin(), result);

            RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);

            new(new_data + distance) T(std::forward<Args>(args)...);

            MoveCnstrN(begin(), distance, new_data.GetAddress());
            MoveCnstrN(result, std::distance(result, end()), new_data + distance + 1);

            std::destroy_n(begin(), size_);
            data_.Swap(new_data);
            ++size_;

            return begin() + distance;
        } else {
            T temporary(std::forward<Args>(args)...);

            std::uninitialized_move_n(end() - 1, 1, end());
            std::move_backward(result, end() - 1, end());

            (*result) = std::move(temporary);
            ++size_;

            return result;
        }
    }

    typename Vector::iterator Erase(const_iterator pos) {
        assert(pos >= begin() && pos < end());
        size_t dist = pos - begin();
        std::move(begin() + dist + 1, end(), begin() + dist);
        PopBack();
        return begin() + dist;
    }

    typename Vector::iterator Insert(const_iterator pos, const T &value) {
        return Emplace(pos, value);
    }

    typename Vector::iterator Insert(const_iterator pos, T &&value) {
        return Emplace(pos, std::move(value));
    }


    [[nodiscard]] size_t Size() const noexcept {
        return size_;
    }


    [[nodiscard]] size_t Capacity() const noexcept {
        return data_.Capacity();
    }


    const T &operator[](size_t index) const noexcept;

    T &operator[](size_t index) noexcept;

    void Reserve(size_t new_capacity) {
        if (new_capacity <= data_.Capacity()) {
            return;
        }

        RawMemory<T> new_data(new_capacity);
        MoveCnstrN(data_.GetAddress(), size_, new_data.GetAddress());

        std::destroy_n(data_.GetAddress(), size_);
        data_.Swap(new_data);
    }

    void Swap(Vector &other) noexcept {
        data_.Swap(other.data_);
        std::swap(size_, other.size_);
    }

    void Resize(size_t new_size) {
        if (new_size > size_) {
            Reserve(new_size);
            std::uninitialized_value_construct_n(data_.GetAddress() + size_, new_size - size_);
        } else {
            std::destroy_n(data_.GetAddress() + new_size, size_ - new_size);
        }
        size_ = new_size;
    }

    template<typename Type>
    void PushBack(Type &&value) {
        EmplaceBack(std::forward<Type>(value));
    }

    void PopBack() {
        if (size_ > 0) {
            std::destroy_at(data_.GetAddress() + size_ - 1);
            --size_;
        }
    }

    template<typename ...Args>
    T &EmplaceBack(Args &&...args) {
        T *emplace;
        if (size_ == Capacity()) {
            RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
            emplace = new(new_data + size_) T(std::forward<Args>(args)...);
            if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
                std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
            } else {
                try {
                    std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
                }
                catch (...) {
                    std::destroy_n(new_data.GetAddress() + size_, 1);
                    throw;
                }
            }
            std::destroy_n(data_.GetAddress(), size_);
            data_.Swap(new_data);
        } else {
            emplace = new(data_ + size_) T(std::forward<Args>(args)...);
        }
        size_ += 1;
        return *emplace;
    }


private:

    void MoveCnstrN(iterator from, size_t n, iterator to) {
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            std::uninitialized_move_n(from, n, to);
        } else {
            std::uninitialized_copy_n(from, n, to);
        }
    }

    RawMemory<T> data_;
    size_t size_ = 0;
};


template<typename T>
const T &Vector<T>::operator[](size_t index) const noexcept {
    return const_cast<Vector &>(*this)[index];
}

template<typename T>
T &Vector<T>::operator[](size_t index) noexcept {
    assert(index < size_);
    return data_[index];
}

template<typename T>
typename Vector<T>::iterator Vector<T>::begin() noexcept {
    return data_.GetAddress();
}

template<typename T>
typename Vector<T>::iterator Vector<T>::end() noexcept {
    return data_ + size_;
}

template<typename T>
typename Vector<T>::const_iterator Vector<T>::begin() const noexcept {
    return data_.GetAddress();
}

template<typename T>
typename Vector<T>::const_iterator Vector<T>::end() const noexcept {
    return data_ + size_;
}

template<typename T>
typename Vector<T>::const_iterator Vector<T>::cbegin() const noexcept {
    return begin();
}

template<typename T>
typename Vector<T>::const_iterator Vector<T>::cend() const noexcept {
    return end();
}

