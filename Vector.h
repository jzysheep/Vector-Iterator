//
// Created by Ziyang Jiang on 1/31/16.
//
// Vector.h -- header file for Vector data structure project

#pragma once

#ifndef _Vector_h
#define _Vector_h

#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace epl {
    
    template <typename T>
    class vector {
    private:
        
        T* cap_start;
        T* cap_finish;
        T* data_start;
        T* data_end;
        
        const uint64_t minimum_capacity = 8;
    public:
        //        using value_type=T;
        
        vector(void) {
            uint64_t capacity = minimum_capacity;
            cap_start = static_cast<T*>(operator new(capacity * sizeof(T)));
            cap_finish = cap_start + capacity;
            data_start = data_end = cap_start;
        }
        
        explicit vector(uint64_t n) {
            uint64_t capacity = n;
            if (n == 0) { capacity = minimum_capacity; }
            cap_start = static_cast<T*>(operator new(capacity * sizeof(T)));
            cap_finish = cap_start + capacity;
            data_start = data_end = cap_start;
            for (uint64_t k = 0; k < n; k += 1) {
                new (data_end) T();
                ++data_end;
            }
            
        }
        
        vector(const vector<T>& that) {
            copy(that);
            
        }
        
        vector(vector<T>&& that) {
            move(std::move(that));
            
        }
        
        template <typename AltType>
        vector(const vector<AltType>& that) {
            uint64_t capacity = that.size();
            if (capacity == 0) { capacity = minimum_capacity; }
            cap_start = static_cast<T*>(operator new(capacity * sizeof(T)));
            cap_finish = cap_start + capacity;
            data_start = data_end = cap_start;
            for (uint64_t k = 0; k < capacity; k += 1) {
                new (data_end) T(that[k]);
                ++data_end;
            }
            
        }
        
        template <typename Iterator>
        vector(Iterator start, Iterator finish) {
            constructFromIterator(start, finish, typename std::iterator_traits<Iterator>::iterator_category());
            
        }
        
        vector(std::initializer_list<T> il) :
        vector(il.begin(), il.end()) {
        }
        
        ~vector(void) {
            destroy();
        }
        
        vector<T>& operator=(const vector<T>& rhs) {
            if (this != &rhs) {
                destroy();
                copy(rhs);
            }
            return *this;
        }
        
        vector<T>& operator=(vector<T>&& rhs) {
            destroy();
            move(std::move(rhs));
            return *this;
        }
        
        uint64_t size(void) const {
            return data_end - data_start;
        }
        
        T& operator[](uint64_t k) {
            T* p = data_start + k;
            if (p >= data_end) { throw std::out_of_range("index out of range"); }
            return *p;
        }
        
        const T& operator[](uint64_t k) const {
            const T* p = data_start + k;
            if (p >= data_end) { throw std::out_of_range("index out of range"); }
            return *p;
        }
        
        void push_back(const T& that) {
            T temp(that);
            check_back(1);
            new (data_end) T(std::move(temp));
            ++data_end;
        }
        
        void push_back(T&& that) {
            //            T temp(std::move(that));
            check_back(1);
            //            new (data_end) T(std::move(temp));
            new (data_end) T(std::move(that));
            ++data_end;
        }
        
        template <typename... Args>
        void emplace_back(Args... args) {
            check_back(1);
            new(data_end) T(args...);
        }
        
        void push_front(const T& that) {
            check_front(1);
            --data_start;
            new (data_start) T(that);
        }
        
        void push_front(T&& that) {
            check_front(1);
            --data_start;
            new (data_start) T(std::move(that));
        }
        
        template <typename... Args>
        void emplace_front(Args... args) {
            check_front(1);
            --data_start;
            new (data_start) T(args...);
        }
        
        void pop_back(void) {
            if (data_start == data_end) { throw std::out_of_range("empty vector, nothing to pop back"); }
            --data_end;
            data_end->~T();
        }
        
        void pop_front(void) {
            if (data_start == data_end) { throw std::out_of_range("empty vector, nothing to pop front"); }
            data_start->~T();
            ++data_start;
        }
        
        T& front(void) {
            if (data_start == data_end) { throw std::out_of_range("empty Vector"); }
            return *data_start;
        }
        
        const T& front(void) const {
            if (data_start == data_end) { throw std::out_of_range("empty Vector"); }
            return *data_start;
        }
        
        T& back(void) {
            if (data_start == data_end) { throw std::out_of_range("back called on empty Vector"); }
            return *(data_end - 1);
        }
        
        const T& back(void) const {
            if (data_start == data_end) { throw std::out_of_range("back called on empty Vector"); }
            return *(data_end - 1);
        }
        
        class iterator;
        class const_iterator : public std::iterator<std::random_access_iterator_tag, T> {
            const vector<T>* parent;
            uint64_t index;
            const T* ptr;
            
        public:
            const_iterator(void) {
                index = 0;
                ptr = nullptr;
                parent = nullptr;
            }
            
            const T& operator*(void) const {
                return *ptr;
            }
            
            const_iterator& operator++(void) {
                ++ptr;
                ++index;
                return *this;
            }
            
            const_iterator operator++(int) {
                const_iterator t(*this);
                this->operator++();
                return t;
            }
            
            const_iterator& operator--(void) {
                --ptr;
                --index;
                return *this;
            }
            
            
            const_iterator operator--(int) {
                const_iterator t(*this);
                this->operator--();
                return t;
            }
            
            int64_t operator-(const const_iterator that) const {
                return this->ptr - that.ptr;
            }
            
            const_iterator operator+(int64_t k) {
                return iterator(parent, ptr + k);
            }
            
            bool operator==(const const_iterator& rhs) const {
                return this->parent == rhs.parent && this->ptr == rhs.ptr;
            }
            
            bool operator!=(const const_iterator& that) const {
                return ! (*this == that);
            }
            
            friend vector<T>;
            friend vector<T>::iterator;
            
        private:
            const_iterator(const vector<T>* parent, const T* ptr) {
                this->parent = parent;
                this->ptr = ptr;
                this->index = ptr - parent->data_start;
            }
            
        };
        
        class iterator : public const_iterator {
            //            using const_iterator = iterator;
            //            using Base = const_iterator;
        public:
            iterator(void) {}
            
            T& operator*(void) const {
                return const_cast<T&>(const_iterator::operator*());
            }
            
            iterator operator+(int64_t k) {
                //                const_iterator::operator+(k); //calls checkRevision for us
                return iterator(const_iterator::parent, const_iterator::ptr + k);
            }
            const_iterator& operator++(void) { const_iterator::operator++(); return *this; }
            const_iterator operator++(int) { const_iterator t(*this); operator++(); return t; }
            const_iterator& operator--(void) { const_iterator::operator--(); return *this; }
            const_iterator operator--(int) { const_iterator t(*this); operator--(); return t; }
        private:
            friend vector<T>;
            iterator(const vector<T>* parent, const T* ptr) : const_iterator(parent, ptr) { }
        };
        
        const_iterator begin(void) const { return const_iterator(this, data_start); }
        iterator begin(void) { return iterator(this, data_start); }
        
        const_iterator end(void) const { return const_iterator(this, data_end); }
        iterator end(void) { return iterator(this, data_end); }
        
    private:
        void destroy(void) {
            if (cap_start != nullptr) {
                while (data_start != data_end) {
                    data_start->~T();
                    ++data_start;
                }
                operator delete(cap_start);
            }
        }
        
        void copy(const vector<T>& that) {
            uint64_t capacity = that.size();
            if (capacity < minimum_capacity) { capacity = minimum_capacity; }
            cap_start = static_cast<T*>(operator new(capacity * sizeof(T)));
            data_start = data_end = cap_start;
            for (uint64_t i = 0; i < that.size(); i += 1) {
                new (data_end) T(that[i]);
                ++data_end;
            }
        }
        
        void move(vector<T>&& that) {
            cap_start = that.cap_start;
            cap_finish = that.cap_finish;
            data_start = that.data_start;
            data_end = that.data_end;
            that.cap_start = that.cap_finish = that.data_start = that.data_end = nullptr;
        }
        
        template <typename Iterator>
        void constructFromIterator(Iterator start, Iterator finish, std::random_access_iterator_tag) {
            uint64_t capacity = (uint64_t) (finish - start);
            if (capacity < minimum_capacity) { capacity = minimum_capacity; }
            cap_start = static_cast<T*>(operator new(capacity * sizeof(T)));
            data_start = data_end = cap_start;
            while (start != finish) {
                new (data_end) T(*start);
                ++data_end;
                ++start;
            }
        }
        
        template <typename Iterator>
        void constructFromIterator(Iterator start, Iterator finish, std::forward_iterator_tag) {
            uint64_t capacity = minimum_capacity;
            cap_start = static_cast<T*>(operator new(capacity * sizeof(T)));
            data_start = data_end = cap_start;
            while (start != finish) {
                push_back(*start);
                ++start;
            }
        }
        
        void check_back(uint64_t back_capacity) {
            if (back_capacity <= (uint64_t) (cap_finish - data_end)) {
                return;
            }
            
            uint64_t capacity = 2 * (cap_finish - cap_start);
            
            while (capacity < back_capacity) {
                capacity *= 2;
            }
            
            uint64_t excess_capacity = capacity - size();
            if (back_capacity < excess_capacity / 2) { back_capacity = excess_capacity / 2; }
            
            T* new_address = static_cast<T*>(operator new(sizeof(T) * capacity));
            T* new_data = new_address + capacity - back_capacity - size();
            T* new_data_end = new_data;
            
            while (data_start != data_end) {
                new (new_data_end) T(std::move(*data_start));
                data_start->~T();
                ++data_start;
                ++new_data_end;
            }
            operator delete(cap_start);
            
            cap_start = new_address;
            cap_finish = cap_start + capacity;
            data_start = new_data;
            data_end = new_data_end;
        }
        
        void check_front(uint64_t front_capacity) {
            if (front_capacity <= (uint64_t) (data_start - cap_start)) {
                return;
            }
            
            uint64_t capacity = 2 * (cap_finish - cap_start);
            
            while (capacity < front_capacity) {
                capacity *= 2;
            }
            
            uint64_t excess_capacity = capacity - size();
            if (front_capacity < excess_capacity / 2) { front_capacity = excess_capacity / 2; }
            
            T* new_address = static_cast<T*>(operator new(sizeof(T) * capacity));
            T* new_data = new_address + front_capacity;
            T* new_data_end = new_data;
            
            while (data_start != data_end) {
                new (new_data_end) T(std::move(*data_start));
                data_start->~T();
                ++data_start;
                ++new_data_end;
            }
            operator delete(cap_start);
            
            cap_start = new_address;
            cap_finish = cap_start + capacity;
            data_start = new_data;
            data_end = new_data_end;
        }
        
    };
    
}

#endif
