#ifndef HASHMAP_HASHMAP_H
#define HASHMAP_HASHMAP_H

#include <functional>
#include <memory>
#include <vector>
#include <utility>
#include <string>
#include <map>
#include <tuple>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
/*using KeyType = std::string;
using ValueType = int;
using Hash = std::hash<std::string>;*/


class HashMap {
    using PairType = std::pair<KeyType, ValueType>;
    using MapType = std::pair<const KeyType, ValueType>;


    float load_factor = 0.77;
    struct Bucket {
        PairType elem;
        int dist;
    };


    std::unique_ptr<Bucket[]> data;
    const int sz_coef = 3;
    size_t sz = 0;
    size_t elem_cnt = 0;
    Hash hash_f;

public:

    class iterator {
        HashMap *map;
        size_t index = 0;
    public:

        iterator() = default;

        iterator(HashMap &map, size_t ind) : map(&map), index(ind) {
            go_to_nearest();
        }

        iterator(const iterator &other) {
            map = other.map;
            index = other.index;
        }


        iterator &operator=(const iterator &other) = default;

        void go_to_nearest() {
            if (index >= map->sz)
                index = map->sz;
            while (index < map->sz && map->data[index].dist == 0)
                ++index;
        }


        iterator &operator++() {
            ++index;
            go_to_nearest();
            return *this;
        }

        iterator operator++(int) {
            auto old = *this;
            ++index;
            go_to_nearest();
            return old;
        }

        MapType &operator*() const {
            return *operator->();
        }

        MapType *operator->() const {
            return (MapType *) (&map->data[index]);
        }

        bool operator==(const iterator &other) const {
            return other.index == index;
        }

        bool operator!=(const iterator &other) const {
            return other.index != index;
        }

    };

    class const_iterator {

        const HashMap *map;
        size_t index = 0;
    public:
        const_iterator() = default;

        const_iterator(const HashMap &map, size_t ind) : map(&map), index(ind) {
            go_to_nearest();
        }

        const_iterator(const const_iterator &other) {
            map = other.map;
            index = other.index;
        }


        const_iterator &operator=(const const_iterator &other) = default;

        void go_to_nearest() {
            if (index >= map->sz) index = map->sz;
            while (index < map->sz && map->data[index].dist == 0) ++index;
        }

        const_iterator &operator++() {
            ++index;
            go_to_nearest();
            return *this;
        }

        const_iterator operator++(int) {
            auto old = *this;
            ++index;
            go_to_nearest();
            return old;
        }

        const MapType &operator*() const {
            return *operator->();
        }

        MapType *operator->() const {
            return (MapType *const) (&map->data[index]);
        }

        bool operator==(const const_iterator &other) const {
            return other.index == index;
        }

        bool operator!=(const const_iterator &other) const {
            return other.index != index;
        }

    };


    void clear() {
        elem_cnt = 0;
        sz = 10;
        data = std::make_unique<Bucket[]>(sz);
    }


    HashMap() {
        clear();
    }

    explicit HashMap(Hash hash_f) : hash_f(hash_f) {
        clear();
    }

    HashMap(std::initializer_list<PairType> init_list) {
        clear();
        for (const auto &elem: init_list) {
            insert(elem);
        }
    }

    HashMap(std::initializer_list<PairType> init_list, Hash hash_f) : hash_f(hash_f) {
        clear();
        for (const auto &elem: init_list) {
            insert(elem);
        }
    }

    template<class InputIterator>
    HashMap(InputIterator beg, InputIterator end) {
        clear();
        for (; beg != end; ++beg) {
            insert(*beg);
        }
    }

    template<class InputIterator>
    HashMap(InputIterator beg, InputIterator end, Hash hash_f) : hash_f(hash_f) {
        clear();
        for (; beg != end; ++beg) {
            insert(*beg);
        }
    }

    void deep_copy(const HashMap &other) {
        load_factor = other.load_factor;
        sz = other.sz;
        elem_cnt = other.elem_cnt;
        hash_f = other.hash_f;
        data = std::make_unique<Bucket[]>(sz);
        for (size_t i = 0; i < sz; ++i) {
            data[i] = other.data[i];
        }
    }

    HashMap(const HashMap &other) {
        deep_copy(other);
    }

    HashMap &operator=(const HashMap &other) {
        if (&other == this)
            return *this;
        deep_copy(other);
        return *this;
    }

    size_t size() const {
        return elem_cnt;
    }

    bool empty() const {
        return elem_cnt == 0;
    }

    Hash hash_function() const {
        return hash_f;
    }

    size_t eval_hash(const KeyType &elem) const {
        return hash_f(elem) % sz;
    }

    void resize(size_t new_size) {
        size_t old_sz = sz;
        sz = new_size;
        auto tmp = std::make_unique<Bucket[]>(sz);
        std::swap(tmp, data);
        elem_cnt = 0;
        for (size_t i = 0; i < old_sz; ++i) {
            if (tmp[i].dist != 0) {
                insert(tmp[i].elem);
            }
        }
    }

    inline void expand() {
        resize(sz * sz_coef);
    }

    inline void shrink() {
        resize(sz / sz_coef);
    }

    void insert(PairType elem) {
        if (elem_cnt >= load_factor * sz) {
            expand();
        }
        size_t h = eval_hash(elem.first);
        int cur_dist = 1;
        while (true) {
            if (data[h].dist == 0) {
                data[h].dist = cur_dist;
                std::swap(data[h].elem, elem);
                ++elem_cnt;
                break;
            }
            if (data[h].dist < cur_dist) {
                std::swap(data[h].dist, cur_dist);
                std::swap(data[h].elem, elem);
            }
            if (data[h].dist == cur_dist && elem.first == data[h].elem.first) {
                return;
            }
            ++cur_dist, ++h;
            if (h == sz) h -= sz;
        }
    }

    bool erase(const KeyType &key) {
        if (elem_cnt > 0 && elem_cnt * sz_coef * sz_coef + 1 < sz) {
            shrink();
        }
        size_t h = eval_hash(key);
        int cur_dist = 1;
        while (true) {
            if (data[h].dist < cur_dist)
                return false;
            if (data[h].elem.first == key) {
                data[h].dist = 0;
                while (true) {
                    size_t nxt = h + 1;
                    if (nxt == sz) nxt -= sz;
                    if (data[nxt].dist <= 1) {
                        data[h].dist = 0;
                        --elem_cnt;
                        return true;
                    }
                    std::swap(data[nxt], data[h]);
                    --data[h].dist;
                    ++h;
                    if (h == sz) h -= sz;
                }
            }
            ++cur_dist, ++h;
            if (h == sz) h -= sz;
        }
    }

    size_t find_pos(const KeyType &key) const {
        size_t h = eval_hash(key);
        int cur_dist = 1;
        while (true) {
            if (data[h].dist < cur_dist)
                return sz;
            if (data[h].elem.first == key) {
                return h;
            }
            ++cur_dist, ++h;
            if (h == sz) h -= sz;
        }
    }

    iterator find(const KeyType &key) {
        return iterator(*this, find_pos(key));
    }

    const_iterator find(const KeyType &key) const {
        return const_iterator(*this, find_pos(key));
    }

    ValueType &operator[](const KeyType &key) {
        size_t pos = find_pos(key);
        if (pos == sz) {
            insert({key, ValueType()});
        }
        return data[find_pos(key)].elem.second;
    }

    const ValueType &at(const KeyType &key) const {
        size_t pos = find_pos(key);
        if (pos == sz) {
            throw std::out_of_range("you're out of range!");
        }
        return data[pos].elem.second;
    }


    iterator begin() {
        return iterator(*this, 0);
    }

    iterator end() {
        return iterator(*this, sz);
    }

    const_iterator begin() const {
        return const_iterator(*this, 0);
    }

    const_iterator end() const {
        return const_iterator(*this, sz);
    }

};


#endif
