#ifndef HV_PROTO_RPC_ROUTER_H_
#define HV_PROTO_RPC_ROUTER_H_

#include "generated/base.pb.h"
#include "generated/zadd.pb.h"
#include "generated/zrem.pb.h"
#include "generated/zrank.pb.h"
#include "generated/zscore.pb.h"

typedef void (*protorpc_handler)(const protorpc::Request& req, protorpc::Response* res);

typedef struct {
    const char*      method;
    protorpc_handler handler;
} protorpc_router;

void error_response(protorpc::Response* res, int code, const std::string& message);
void not_found(const protorpc::Request& req, protorpc::Response* res);
void bad_request(const protorpc::Request& req, protorpc::Response* res);

void zadd(const protorpc::Request& req, protorpc::Response* res);
void zrem(const protorpc::Request& req, protorpc::Response* res);
void zrank(const protorpc::Request& req, protorpc::Response* res);
void zscore(const protorpc::Request& req, protorpc::Response* res);


#endif // HV_PROTO_RPC_ROUTER_H_
