#ifndef __SKIPLIST_H__
#define __SKIPLIST_H__

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <mutex>
#include <algorithm>

#define SKIPLIST_P 0.5
#define SKIPLIST_MAXLEVEL 32
#define STORE_FILE "store/dump_file"

#define __output(...) \
	printf(__VA_ARGS__);

#define __format(__fmt__) "%s(%d)-<%s>: " __fmt__ "\n"

#define TRACE_CMH(__fmt__, ...) \
	__output(__format(__fmt__), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);

std::mutex mtx;
std::string delimiter = ":";

class SkipListNode {
public:
	SkipListNode() {}
	SkipListNode(std::string ele, double socre, int level);
	~SkipListNode();

	int level_;
	SkipListNode* backward_;
	// 采用Redis原结构，使用不定长数组 TODO: 此处是否会存在问题？更改为二重指针
	SkipListNode** forward_;
	unsigned int* span_;

	std::string ele_; // Key
	double score_; // Value
};

SkipListNode::SkipListNode(std::string ele, double score, int level) {
	this->ele_      = ele;
	this->score_    = score;
	this->backward_ = NULL;
	this->level_    = level;

	this->forward_  = new SkipListNode*[level];
	this->span_     = new unsigned int[level];
	memset(this->forward_, 0, sizeof(SkipListNode*) * level);
	memset(this->span_, 0, sizeof(unsigned int) * level);
}

SkipListNode::~SkipListNode() {
	delete []forward_;
	delete []span_;
}

class SkipList {
public:
	SkipList(int max_level);
	~SkipList();
	int InsertElement(std::string ele, double score);
	void DeleteElement(std::string ele, double score);
	bool SearchElement(std::string ele, double score);
	void DisplayList();
	void DumpFile();
	void LoadFile();
	int Length();
private:
	// 表头节点和表尾节点
	SkipListNode* header_, * tail_;
	// 表中节点的数量
	unsigned int length_;
	// 表中层数最大的节点的层数
	int level_;
	// 表中允许的最大节点层数
	int max_level_;

	// file operator
    std::ofstream file_writer_;
    std::ifstream file_reader_;

	bool IsValidString(const std::string& str);
	void GetObjectFromString(const std::string& str, std::string* key, std::string* value);

	int GetRandomLevel();
};

/* 构造函数 */
SkipList::SkipList(int max_level) {
	max_level_ = max_level;
	level_ = 0;
	length_ = 0;

	std::string ele;
	double score;
	header_ = new SkipListNode(ele, score, max_level);
	header_->backward_ = NULL;
	for(int i = 0; i < max_level; ++i){
		header_->forward_[i] = NULL;
		header_->span_[i]    = 0;
	}
	tail_ = NULL;
}

SkipList::~SkipList() {
	if (file_writer_.is_open()) {
        file_writer_.close();
    }
    if (file_reader_.is_open()) {
        file_reader_.close();
    }

    // 清理仍然处于跳表中的节点
	while(tail_ != NULL) {
		SkipListNode* curr = tail_;
		tail_ = tail_->backward_;
		delete curr;
	}

    delete header_;
}

int SkipList::InsertElement(std::string ele, double score) {
	mtx.lock();
	// update用于记录待插入位置的前一个节点
	SkipListNode* update[max_level_];
	memset(update, 0, sizeof(SkipListNode*) * max_level_);
	// rank用于记录从header节点到update[i]节点所经历的步长
	unsigned int rank[max_level_];
	memset(rank, 0, sizeof(unsigned int) * max_level_);

	SkipListNode* curr = header_, * next;
	// 当前最大深度level_和新创建节点的最大深度level的较大值cur_level，如果当前最大深度大，表示从level_开始寻找；如果新创建节点的最大深度大，表示从level开始创建节点
	int cur_level = max_level_;
	
	while(--cur_level >= 0) {
		// 当前最大深度大于新创建节点的最大深度，应该从cur_level开始寻找，直到level再插入
		// 当前层数大于新建node的层数，但是当前层有元素
		// 1.forword不为空；2.score小于forward->score 或者 score等于forward->score但是ele的字典序小于forward->ele
		// 此处的比较函数是否需要独立函数?是的
		while(curr->forward_[cur_level] != NULL 
		&& (curr->forward_[cur_level]->score_ < score 
		|| (score == curr->forward_[cur_level]->score_ && curr->forward_[cur_level]->ele_ < ele))) {
			rank[cur_level] += curr->span_[cur_level];
			curr = curr->forward_[cur_level];
		}

		update[cur_level] = curr;

		if(curr->forward_[cur_level] != NULL && curr->forward_[cur_level]->ele_ == ele && curr->forward_[cur_level]->score_ == score) {
			// std::cout << "key: " << ele << ", score: "<< score << ", exists" << std::endl;
			mtx.unlock();
			return 1;
		}
	}

	// 新创建节点的最大深度level
	int level = GetRandomLevel();
	SkipListNode* node = new SkipListNode(ele, score, level);
	// TRACE_CMH("node: [%p], ele: [%s], score: [%lf], level: [%d]\n", node, ele.c_str(), score, level);
	node->ele_   = ele;
	node->score_ = score;

	// 当cur_level < level时，插入节点node
	for(int i = 0; i < level; ++i) {
		next = update[i]->forward_[i];
		update[i]->forward_[i] = node;
		node->forward_[i] = next;
	}

	// 最底层为所有依照score大小排列的双向链表：当next为空时，表示该节点为尾节点；当next不为空时，使用backward进行反向连接
	if(next != NULL) next->backward_ = node;
	else tail_ = node;
	if(curr != header_) node->backward_ = curr;
	else node->backward_ = NULL;

	// 设置span
	for(int i = 0; i < level; ++i) {
		update[i]->forward_[i]->span_[i] = update[i]->span_[i] - (rank[0] - rank[i]);
		update[i]->span_[i] = rank[0] - rank[i] + 1;
	}

	level_ = std::max(level, level_);
	++length_;

	// std::cout << "Successfully inserted key: " << ele << ", value:" << score << std::endl;
	mtx.unlock();
	return 0;
}

void SkipList::DeleteElement(std::string ele, double score) {
	mtx.lock();
	SkipListNode* curr = header_, * node;
	int cur_level = level_, level = max_level_;
	SkipListNode* update[max_level_];
	memset(update, 0, sizeof(SkipListNode*) * max_level_);
	while(--cur_level >= 0) {
		while(curr->forward_[cur_level] != NULL 
			&& (curr->forward_[cur_level]->score_ < score || score == curr->forward_[cur_level]->score_ && curr->forward_[cur_level]->ele_ < ele))
			curr = curr->forward_[cur_level];

		update[cur_level] = curr;

		node = curr->forward_[cur_level];
		if(node != NULL && node->ele_ == ele && node->score_ == score && level == max_level_)
			level = cur_level;
	}

	// 将level作为一个标志，此处表示没有找到相应元素
	if(level == max_level_) {
		std::cout << "key: " << ele << ", score: "<< score << ", does not exist" << std::endl;
		mtx.unlock();
		return;
	}

	// TRACE_CMH("node_level: [%d], list_level: [%d]\n", level + 1, level_);
	// 以下的逻辑表示从0～level层有该元素，需要逐层处理
	for(int i = 0; i <= level; ++i) {
		if(node->forward_[i] == NULL) {
			// node是尾节点
			update[i]->span_[i] = 0;
			update[i]->forward_[i] = NULL;
			if(i == 0) tail_ = update[i];
		}
		else {
			// node不是尾节点
			update[i]->span_[i] += node->span_[i] - 1;
			update[i]->forward_[i] = node->forward_[i];
			if(i == 0) node->forward_[i]->backward_ = update[i];
		}
	}

	// 释放节点内存
	delete node;

	--length_;

	mtx.unlock();
}

bool SkipList::SearchElement(std::string ele, double score) { 
	SkipListNode* curr = header_, * node;
	int cur_level = level_;
	while(--cur_level >= 0) {
		while(curr->forward_[cur_level] != NULL 
			&& (curr->forward_[cur_level]->score_ < score || score == curr->forward_[cur_level]->score_ && curr->forward_[cur_level]->ele_ < ele))
			curr = curr->forward_[cur_level];

		node = curr->forward_[cur_level];
		if(node != NULL && node->ele_ == ele && node->score_ == score) {
			// std::cout << "Found Key: " << ele << ", Value: " << score << std::endl;
			return true;
		}
	}

	// std::cout << "NotFound Key: " << ele << ", Value: " << score << std::endl;
	return false;
}

void SkipList::DisplayList() {
	std::cout << "\n************************* Skip List *************************" << std::endl;
    for (int i = 0; i < max_level_; i++) {
        SkipListNode *node = header_->forward_[i]; 
        std::cout << "Level " << i << ": ";
        while (node != NULL) {
            std::cout << node->ele_ << ":" << node->score_ << ";";
            node = node->forward_[i];
        }
        std::cout << std::endl;
    }
	std::cout << std::endl;
}

void SkipList::DumpFile() {
    std::cout << "\n************************* Dump File *************************" << std::endl;
    file_writer_.open(STORE_FILE);
    SkipListNode *node = header_->forward_[0];

    while (node != NULL) {
        file_writer_ << node->ele_ << ":" << node->score_ << "\n";
        std::cout << node->ele_ << ":" << node->score_ << ";\n";
        node = node->forward_[0];
    }
	std::cout << std::endl;

    file_writer_.flush();
    file_writer_.close();
}

void SkipList::LoadFile() {
    file_reader_.open(STORE_FILE);
    std::cout << "\n************************* Load File *************************" << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* val = new std::string();
    while (getline(file_reader_, line)) {
        GetObjectFromString(line, key, val);
        if (key->empty() || val->empty()) {
            continue;
        }
		
		double value;
		try {
			value = std::stod(*val);
		}
		catch(const std::exception& e) {
			std::cerr << e.what() << '\n';
			continue;
		}
		
        InsertElement(*key, value);
        std::cout << "key: " << *key << "value: " << value << std::endl;
    }
    file_reader_.close();

	delete key;
	delete val;
}

bool SkipList::IsValidString(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}

void SkipList::GetObjectFromString(const std::string& str, std::string* key, std::string* value) {
    if(!IsValidString(str)) {
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

// 获取表中节点的数量
int SkipList::Length() {
	return length_;
}

/* 返回插入跳表的层数 */
// 此处与Redis跳表稍有不同，最底层的双向链表为level0
int SkipList::GetRandomLevel() {
	int level = 1;
	while((random() & 0xFFFF) < (SKIPLIST_P * 0xFFFF))
		level += 1;
	return (level < max_level_) ? level : max_level_;
}

#endif