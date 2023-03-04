#ifndef __LRU_H__
#define __LRU_H__

#include <list>
#include <utility>
#include <unordered_map>

template <typename K, typename V>
class LRUCache {
public:
    LRUCache(int capacity): capacity_(capacity) {}
    
    int get(K key) {
        auto it = cache_.find(key);
        if(it == cache_.end()) return -1;
        lst_.splice(lst_.begin(), lst_, it->second);
        return it->second->second;
    }
    
    void put(K key, V value) {
        auto it = cache_.find(key);
        //Already Exists
        if(it != cache_.end()){
            it->second->second = value;
            lst_.splice(lst_.begin(), lst_, it->second);
            return;
        }
        //Not exists
        lst_.insert(lst_.begin(), std::make_pair(key, value));
        //Non-sequence container,other iterators won't be influenced by insert/delete
        cache_[key] = lst_.begin();
        if(cache_.size() > capacity_){
            //Delete according to the key
            cache_.erase(lst_.back().first); // lst_.back() is the element not iterator
            lst_.pop_back();
        }
    }
private:
    std::unordered_map<int, std::list<std::pair<K, V>>::iterator> cache_;
    std::list<std::pair<K, V>> lst_;
    int capacity_;
};

#endif