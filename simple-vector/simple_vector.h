#pragma once

#include <cassert>
#include <initializer_list>
#include "array_ptr.h"
#include <algorithm>
#include <stdexcept>
#include <iterator>

struct ReserveProxyObj {

    ReserveProxyObj(size_t capacity) : capacity_(capacity) {};
    size_t capacity_;

};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:

    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : size_(size),
        capacity_(size),
        vect_(size)
    {
        std::generate(vect_.Get(), vect_.Get() + size, []() {return Type{}; });
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        :size_(size),
        capacity_(size),
        vect_(size) {
        std::generate(vect_.Get(), vect_.Get() + size, [value]() {return Type{ value }; });
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
        
        :size_(init.size()),
        capacity_(init.size()), 
        vect_(init.size()) {

        std::copy(init.begin(), init.end(), vect_.Get());

    }
    SimpleVector(const SimpleVector& other)
    {
        SimpleVector<Type>tmp(other.size_);
        std::copy(other.begin(), other.end(), tmp.vect_.Get());
        swap(tmp);
    }

    SimpleVector(SimpleVector&& other)

    {
        size_ = other.size_;
        capacity_ = other.capacity_;
        vect_ = (std::move(other.vect_));
        other.size_ = 0;

    }
    SimpleVector(size_t size, Type&& value) :
        SimpleVector(size) {
        for (auto i = vect_.Get(); i < vect_.Get() + size; ++i) {
            *i = std::move(value);
        }

    }
    SimpleVector operator=(SimpleVector&& rhs)
    {
        if (*this != rhs)//проверка что не присваиваем себя себе
        {
            SimpleVector temp(rhs);
            swap(temp);
        }

        return *this;
    }
    SimpleVector(ReserveProxyObj proxy) {
        capacity_ = proxy.capacity_;
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
    Type& operator[](size_t index) noexcept {
        assert(index >= 0 && index < size_);
        return vect_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index >= 0 && index < size_);
        if (index < size_) {
            return vect_[index];
        }
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Error: index out_of_range ");
        }
        else{
            return vect_[index]; 
            }
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Error: index out_of_range ");
        }
        else {
            return vect_[index];
        }
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
        }
        else if (new_size <= capacity_) {

            std::generate(vect_.Get() + size_, vect_.Get() + capacity_, []() {return Type{}; });
            size_ = new_size;
        }
        else {
            Reloc(new_size);
        }
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            auto tmp_ = size_;
            Reloc(new_capacity);
            size_ = tmp_;
        }
    }


    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return vect_.Get();
        // Напишите тело самостоятельно
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return vect_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return vect_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return vect_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return vect_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return vect_.Get() + size_;
    }


    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this->begin() == rhs.begin()) {
            return *this;
        }
        SimpleVector tmp(rhs);
        swap(tmp);
        return *this;
    }
    

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора

    void PushBack(const Type& item) {
        Insert(end(), item);
    }
    void PushBack(Type&& item) {
        Insert(end(), std::move(item));
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= cbegin() && pos <= cend());
        auto index_pos = pos - this->begin();
        if (capacity_ == 0) {
            Reloc(1);
            vect_[0] = value;
            return Iterator{ this->begin() + index_pos };
        }

        if (size_ < capacity_) {
            ++size_;
            std::copy_backward(this->begin() + index_pos, this->end(), this->end());
            vect_[index_pos] = value;

        }
        else {
            auto tmp_ = ++size_;
            Reloc(2 * capacity_);
            size_ = tmp_;
            std::copy_backward(begin() + index_pos, this->end(), this->end() + 1);
            vect_[index_pos] = value;

        }
        return Iterator{ this->begin() + index_pos };
    }
    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= cbegin() && pos<= cend());
        auto index_pos = pos - this->begin();
        if (capacity_ == 0) {
            Reloc(1);
            vect_[0] = std::move(value);
            return Iterator{ this->begin() + index_pos };
        }

        if (size_ < capacity_) {
            ++size_;
            std::copy_backward(std::move_iterator(this->begin() + index_pos), std::move_iterator(this->end()), this->end());
            vect_[index_pos] = std::move(value);

        }
        else {
            auto tmp_ = ++size_;
            Reloc(2 * capacity_);
            size_ = tmp_;
            std::copy_backward(std::move_iterator(this->begin() + index_pos), std::move_iterator(this->end()), this->end() + 1);
            vect_[index_pos] = std::move(value);

        }
        return Iterator{ this->begin() + index_pos };
    }
    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(size_ > 0);
        
            --size_;
        
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= cbegin() && pos <= cend());;
        assert(size_ > 0);
        size_t position = pos - begin();
        std::copy(std::move_iterator(begin() + position + 1), std::move_iterator(end()), begin() + position);
        --size_;
        return vect_.Get() + position;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        other.vect_.swap(this->vect_);
        std::swap(this->size_, other.size_);
        std::swap(this->capacity_, other.capacity_);
    }

   

private:
    size_t  size_ = 0;
    size_t  capacity_ = size_;
    ArrayPtr<Type> vect_;
    void Reloc(size_t new_size) {

        ArrayPtr<Type> tmp_(new_size);
        std::copy(std::move_iterator(vect_.Get()), std::move_iterator(vect_.Get() + size_), tmp_.Get());
        std::generate(tmp_.Get() + size_, tmp_.Get() + new_size, []() {return Type{}; });
        vect_.swap(tmp_);
       
        size_ = capacity_ = new_size;

    }

};


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (&lhs == &rhs)  // Оптимизация сравнения списка с собой
        || (lhs.GetSize() == rhs.GetSize()
            && std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()));  // может бросить исключение
}


template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);  // может бросить исключение
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());  // может бросить исключение
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);  // Может бросить исключение
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (rhs < lhs);  // Может бросить исключение
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);  // Может бросить исключение
}
