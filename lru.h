#ifndef __LRU_H__
#define __LRU_H__

#include <bits/stdc++.h>

using namespace std;

#define DEFUALT_MAXLENGTH 0

// 此处需要重载hash函数，否则会报错；当前支持的类型有：int, long, long long, string
// pair 和 tuple 需要自己重载hash函数
// 泛型编程模板
// 遗留问题：重载hash函数
template <typename K, typename V>
class LRUCache {
public:
	typedef pair<K, V> PII;
	typedef typename list<PII>::iterator IT;

	// 用于记录删除的key
	vector<K> delete_keys;

    LRUCache(int capacity = DEFUALT_MAXLENGTH): capacity(capacity) {}
    
    int get(K key) {
		if(cache.find(key) == cache.end()) return -1;
		// auto&& [_, it] = *cache.find(key); // C++17
		auto it = cache.find(key);
		lru.splice(lru.begin(), lru, it->second);
		return it->second->second;
    }
    
    void put(K key, V value) {
		if(cache.find(key) != cache.end()) {
			// auto&& [_, it] = *cache.find(key); // C++17
			auto it = cache.find(key);
			it->second->second = value;
			lru.splice(lru.begin(), lru, it->second);
			return;
		}
		// 缓存满了，删除最后一个，但是需要注意，当capacity为0时，不需要删除
		// 因为缓存满了，需要删除最后一个，所以需要记录删除的key
		if(capacity > 0 && cache.size() == capacity) {
			auto it = lru.end();
			it--;
			cache.erase(it->first);
			lru.erase(it);
			delete_keys.emplace_back(it->first);
		}
		lru.push_front(PII(key, value));
		cache.insert(make_pair(key, lru.begin()));
    }

	// 主动删除缓存的情况，不用计入delete_keys
	void remove(K key) {
		if(cache.find(key) == cache.end()) return;
		// auto&& [_, it] = *cache.find(key); // C++17
		auto it = cache.find(key);
		lru.erase(it->second);
		cache.erase(key);
	}

	int set_capacity(int capacity) {
		if(capacity < 0) return -1;

		this->capacity = capacity;
		// 缓存满了，删除最后一个，但是需要注意，当capacity为0时，不需要删除
		while(capacity > 0 && cache.size() > capacity) {
			auto it = lru.end();
			it--;
			cache.erase(it->first);
			lru.erase(it);
			delete_keys.emplace_back(it->first);
		}
		return 0;
	}
private:
	int capacity;

	// 重载hash函数的方式 C++17
	// static constexpr auto tri_hash = [fn = hash<int>()](const tuple<int, int, int>& o) -> size_t {
	// 	auto&& [x, y, z] = o;
	// 	return (fn(x) << 24) ^ (fn(y) << 8) ^ fn(z);
	// };

	// unordered_map<tuple<int, int, int>, pair<TreeNode*, int>, decltype(tri_hash)> cache{0, tri_hash};

	unordered_map<K, IT> cache;
	list<PII> lru;
};

#endif