#include "RpcCall.h"

RpcCall::RpcCall() {
    m_GrpcContext = std::make_shared<agrpc::GrpcContext>();
}

RpcCall::~RpcCall() {
    m_GrpcContext->stop();
    m_GrpcContextThread.join();
}
