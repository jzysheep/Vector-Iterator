#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <cstdint>
#include <stdexcept>
#include <utility>

//Utility gives std::rel_ops which will fill in relational
//iterator operations so long as you provide the
//operators discussed in class.  In any case, ensure that
//all operations listed in this website are legal for your
//iterators:
//http://www.cplusplus.com/reference/iterator/RandomAccessIterator/
using namespace std::rel_ops;

namespace epl{
    
    class invalid_iterator {
    public:
        enum SeverityLevel {SEVERE,MODERATE,MILD,WARNING};
        SeverityLevel level;
        
        invalid_iterator(SeverityLevel level = SEVERE){ this->level = level; }
        virtual const char* what() const {
            switch(level){
                case WARNING:   return "Warning"; // not used in Spring 2015
                case MILD:      return "Mild";
                case MODERATE:  return "Moderate";
                case SEVERE:    return "Severe";
                default:        return "ERROR"; // should not be used
            }
        }
    };
    
    template <typename T>
    class vector {
    private:
        
        T* cap_start;
        T* cap_finish;
        T* data_start;
        T* data_end;
        
        size_t version = 0;
        size_t resize_version = 0;
        
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
                ++resize_version;
                ++version;
            }
            return *this;
        }
        
        vector<T>& operator=(vector<T>&& rhs) {
            destroy();
            move(std::move(rhs));
            ++version;
            ++resize_version;
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
            ++version;
            
        }
        
        void push_back(T&& that) {
            T temp(std::move(that));
            check_back(1);
            new (data_end) T(std::move(temp));
            //            new (data_end) T(std::move(that));
            ++data_end;
            ++version;
        }
        
        template <typename... Args>
        void emplace_back(Args&&... args) {
            check_back(1);
            new(data_end) T(std::forward<Args>(args)...);
            ++version;
        }
        
        void push_front(const T& that) {
            T temp(that);
            check_front(1);
            --data_start;
            new (data_start) T(std::move(temp));
            ++version;
        }
        
        void push_front(T&& that) {
            T temp(std::move(that));
            check_front(1);
            --data_start;
            new (data_start) T(std::move(temp));
            ++version;
        }
        
        template <typename... Args>
        void emplace_front(Args&&... args) {
            check_front(1);
            --data_start;
            new (data_start) T(std::forward<Args>(args)...);
            ++version;
        }
        
        void pop_back(void) {
            ++version;
            if (data_start == data_end) { throw std::out_of_range("empty vector, nothing to pop back"); }
            --data_end;
            data_end->~T();
        }
        
        void pop_front(void) {
            ++version;
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
            const vector<T>* obj;
            
            uint64_t version;
            uint64_t resize_version;
            uint64_t index; // change from pointer to index for the convinience of comparison
            bool valid;
            
            //        using Same = const_iterator;
            
        public:
            const_iterator(void) { // no use
                obj = nullptr;
                version = 0;
                resize_version = 0;
                index = 0;
                valid = true;
            }
            
            const_iterator(const vector<T>* obj, size_t version, size_t resize_version, uint64_t index) {
                this->obj = obj;
                this->version = version;
                this->resize_version = resize_version;
                this->index = index;
                this->valid = (index >=0 && index < obj->size());
            }
            
            const_iterator(const const_iterator& that) : obj(that.obj), version(that.version), resize_version(that.resize_version), index(that.index), valid(that.valid) {};
            
            const_iterator operator=(const const_iterator& that) {
                obj = that.obj;
                version = that.version;
                resize_version = that.resize_version;
                index = that.index;
                valid = that.valid;
                return *this;
            }
            
            
            
            const T& operator*(void) const {
                validate(this);
                return *(obj->data_start + index);
            }
            
            const_iterator& operator++(void) {
                validate(this);
                ++index;
                valid = (index >= 0 && index < obj->size());
                return *this;
            }
            
            const_iterator operator++(int) {
                const_iterator tmp(*this);
                this->operator++();
                return tmp;
            }
            
            const_iterator& operator--(void) {
                validate(this);
                --index;
                valid = (index >= 0 && index < obj->size());
                return *this;
            }
            
            
            const_iterator operator--(int) {
                const_iterator tmp(*this);
                this->operator--();
                return tmp;
            }
            
            int64_t operator-(const const_iterator& that) const {
                validate(this);
                validate(&that);
                return this->index - that.index;
            }
            
            const_iterator operator+(int64_t k) {
                validate(this);
                index += k;
                valid = (index >= 0 && index < obj->size());
                return *this;
            }
            
            
            T& operator[](uint64_t k) { return *(obj->data_start + index + k); }
            
            
            
            bool operator==(const const_iterator& that) const {
                validate(this);
                validate(&that);
                return this->index == that.index;
            }
            
            bool operator!=(const const_iterator& that) const {
                return ! (*this == that);
            }
            
            friend vector<T>;
            friend vector<T>::iterator;
            
            void validate(const const_iterator* iter) const {
                if ((iter->version != iter->obj->version) || (iter->resize_version != iter->obj->resize_version)) {
                    if (iter->valid && (iter->index < 0 || iter->index >= iter->obj->size()))
                    {throw epl::invalid_iterator{ epl::invalid_iterator::SEVERE };}
                    else if (iter->valid && (iter->resize_version != iter->obj->resize_version))
                    {throw epl::invalid_iterator{ epl::invalid_iterator::MODERATE };}
                    else
                    { throw epl::invalid_iterator{ epl::invalid_iterator::MILD }; }
                }
            }
            
            
        };
        
        class iterator : public const_iterator {
            
        public:
            iterator(void) {}
            
            T& operator*(void) const {
                return const_cast<T&>(const_iterator::operator*());
            }
            
            iterator operator+(int64_t k) {
                const_iterator::operator+(k); //calls checkRevision for us
                return iterator(const_iterator::obj, const_iterator::version, const_iterator::resize_version, const_iterator::index);
            }
            iterator& operator++(void) { const_iterator::operator++(); return *this; }
            iterator operator++(int) { iterator tmp(*this); operator++(); return tmp; }
            iterator& operator--(void) { const_iterator::operator--(); return *this; }
            iterator operator--(int) { iterator tmp(*this); operator--(); return tmp; }
        private:
            friend vector<T>;
            iterator(const vector<T>* obj, size_t version, size_t resize_version, uint64_t index) : const_iterator(obj, version, resize_version, index) { }
        };
        
        const_iterator begin(void) const { return const_iterator(this, version, resize_version, 0); }
        iterator begin(void) { return iterator(this, version, resize_version, 0); }
        
        const_iterator end(void) const { return const_iterator(this, version, resize_version, size()); }
        iterator end(void) { return iterator(this, version, resize_version, size()); }
        
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
            
            ++resize_version;
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
            
            ++resize_version;
        }
        
    };
    
    
} //namespace epl

#endif
