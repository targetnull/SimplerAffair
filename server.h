#ifndef _SERVER_H_
#define _SERVER_H_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <grpc++/grpc++.h>

#include "chainserver.grpc.pb.h"

using grpc::Channel;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ClientContext;
using chainserver::ChainServer;
using chainserver::MsgType;
using chainserver::MsgBlock;
using chainserver::Block;
using chainserver::Empty;

class CNode
{
public:
        CNode(const char *node_ip, const char *node_port) : peer_ip(node_ip), port(node_port) {
                stub_ = ChainServer::NewStub(grpc::CreateChannel(peer_ip + ':' + port, grpc::InsecureChannelCredentials()));
        }

        //assume ipv4 bydefault, but should check when parsing
        CNode(std::string &node_ip, std::string &node_port) : peer_ip(node_ip), port(node_port) {
                stub_ = ChainServer::NewStub(grpc::CreateChannel(peer_ip + ":" + port, grpc::InsecureChannelCredentials()));
        }

        //this way is strongly NOT suggested as it hides the ip and port of the connection
        CNode(std::shared_ptr<Channel> &chan): channel(chan) {
                stub_ = ChainServer::NewStub(channel);
        }

        std::string& get_ip()
        {
                return peer_ip;
        }

        std::string& get_port()
        {
                return port;
        }

        std::string peer_ip;
        std::string port;
        std::unique_ptr<ChainServer::Stub> stub_;
        std::shared_ptr<Channel> channel; //corresponding channel
};

class ChainServerImpl final : public ChainServer::Service
{
public:
        Status Connect(ServerContext* context, const MsgBlock* msg, Empty* empty) override;

        Status SendMsg(ServerContext* context, const MsgBlock* msg, Empty* empty) override;

        CNode *FindNode(std::string &addr)
        {
                for (std::vector<CNode *>::iterator it = cnodes.begin() ; it != cnodes.end(); ++it)
                {
                        CNode *tnode = *it;
                        if(0 == addr.compare(tnode->get_ip())) //FOUND!
                                return tnode;
                }
                return NULL; //bad luck
        }

        void PushNode(CNode *node)
        {
                if(NULL == node)
                        return;
                cnodes.push_back(node);
        }

	//currently connected to how many nodes
        int peerCount()
        {
                return cnodes.size();
        }

//private: //not sure whether should set it private or not
        std::vector<CNode *> cnodes;
        //std::mutex qlock;
};//currently connected to how many nodes

#endif

