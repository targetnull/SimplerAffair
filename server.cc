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
using chainserver::ChainServer;
using chainserver::MsgType;
using chainserver::MsgBlock;
using chainserver::ReplyType;
using chainserver::Reply;
using chainserver::Block;
using chainserver::Empty;

static Status getBlock(const std::string &msg)
{
	std::cout << "get message: " << msg << std::endl;
	return Status::OK;
}

class CNode
{
public:
	//assume ipv4 bydefault, but should check when parsing
	CNode(std::string node_ipaddr, std::string node_port) : peer_addr(node_ipaddr), port(node_port) {
		channel = grpc::CreateChannel(node_ipaddr + ":" + node_port, grpc::InsecureChannelCredentials());
		std::cout << "successfully connect back to sender " << node_ipaddr << ':' << node_port << std::endl;
	}

	std::string& get_addr()
	{
		return peer_addr;
	}

	std::string& get_port()
	{
		return port;
	}

	std::string peer_addr;
	std::string port;
	std::shared_ptr<Channel> channel; //corresponding channel
};

class ChainServerImpl final : public ChainServer::Service
{
public:
	Status Connect(ServerContext* context, const MsgBlock* msg, Empty* empty) override {
		std::string peer_addr = context->peer();
		int pos1 = peer_addr.find(':');
		int pos2 = peer_addr.find(':', pos1 + 1);
		std::cout << pos1 << ':' << pos2 << std::endl;
		std::cout << "Received connect from " << peer_addr.substr(pos1, pos2 - pos1 + 1) << std::endl;
		std::cout << "msg type is " << msg->type() << std::endl;
		//then establish a reverse connection
		CNode *node = new CNode(peer_addr.substr(pos1, pos2 - pos1 + 1), peer_addr.substr(pos2));
		cnodes.push_back(node); 
		return Status::OK;
	}

	Status SendMsg(ServerContext* context, const MsgBlock* msg, Empty* empty) override {
		Status res;
		std::string peer_addr = context->peer();
                std::cout << "Received message from " << peer_addr << std::endl;
                std::cout << "msg type is " << msg->type() << std::endl;
		switch(msg->type())
		{
		case MsgType::GET_BLOCK:
			res = getBlock(msg->msgdata());
			break;
		case MsgType::QUIT:
			//currently does nothing
		default:
			res = Status::CANCELLED;
			break;
		}
                return res;//Status::OK;
	}

	CNode *FindNode(std::string &addr)
	{
		for (std::vector<CNode *>::iterator it = cnodes.begin() ; it != cnodes.end(); ++it)
		{
			CNode *tnode = *it;
			if(0 == addr.compare(tnode->get_addr())) //FOUND!
				return tnode;
		}
		return NULL; //bad luck
	}
private:
	std::vector<CNode *> cnodes;
	std::mutex qlock;
};

int main()
{
	std::string server_address("0.0.0.0:50051");
	//RouteGuideImpl service(db_path);
	ChainServerImpl service;

	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;
	server->Wait();
	return 0;
}

