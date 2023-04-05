#ifndef __LRU_H__
#define __LRU_H__

#include <bits/stdc++.h>

using namespace std;

// 此处需要重载hash函数，否则会报错；当前支持的类型有：int, long, long long, string
// pair 和 tuple 需要自己重载hash函数
// 泛型编程模板
// 遗留问题：重载hash函数
template <typename K, typename V>
class LRUCache {
public:
	typedef pair<K, V> PII;
	typedef list<PII>::iterator IT;

    LRUCache(int capacity): capacity(capacity) {}
    
    int get(K key) {
		if(cache.find(key) == cache.end()) return -1;
		auto&& [_, it] = *cache.find(key); // C++17
		lru.splice(lru.begin(), lru, it);
		return it->second;
    }
    
    void put(K key, V value) {
		if(cache.find(key) != cache.end()) {
			auto&& [_, it] = *cache.find(key); // C++17
			it->second = value;
			lru.splice(lru.begin(), lru, it);
			return;
		}
		if(cache.size() == capacity) {
			IT it = lru.end();
			it--;
			cache.erase(it->first);
			lru.erase(it);
		}
		lru.push_front(PII(key, value));
		cache.insert(pair(key, lru.begin()));
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