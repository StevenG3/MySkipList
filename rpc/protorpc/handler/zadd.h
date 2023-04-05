#ifndef __ZADD_H__
#define __ZADD_H__

#include "../router.h"

#include "../generated/zadd.pb.h"

void zadd(const protorpc::ZaddRequest& req, protorpc::ZaddResponse* res) {
	// 实现插入跳表的逻辑
	if(req.items_size() == 0) {
		return bad_request(req, res);
	}

	protorpc::ZaddRequestItem param;
	if(!param.ParseFromString(req.params(0))) {
		return bad_request(req, res);
	}

	protorpc::ZaddResponse result;
	for(int i = 0; i < param.key_size(); i++) {
		// 插入跳表
		// result.add_result(1);
	}

	res->set_result(result.SerializeAsString());
}

#endif // __ZADD_H__