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
	Status Connect(ServerContext* context, const MsgBlock* msg, Empty* empty) override {
		std::string peer_addr = context->peer();
		int pos1 = peer_addr.find(':');
		int pos2 = peer_addr.find(':', pos1 + 1);
		std::cout << "Received connect from " << peer_addr.substr(pos1 + 1, pos2 - pos1 - 1) << " " << peer_addr.substr(pos2 + 1) << std::endl;
		//std::cout << "msg type is " << msg->type() << std::endl;
		std::string peer_ip = peer_addr.substr(pos1 + 1, pos2 - pos1 - 1);
		std::string peer_port = peer_addr.substr(pos2 + 1);
		//then establish a reverse connection
		CNode *node = new CNode(peer_ip, peer_port);
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
//private: //not sure whether should set it private or not
	std::vector<CNode *> cnodes;
	//std::unique_ptr<ChainServer::Stub> stub_;
	//std::mutex qlock;
};

static CNode *connect_seed(const char *ip, const char *port)
{
	CNode *seed = new CNode(ip, port);
	if(!seed)
		return NULL;

	ClientContext context;
	MsgBlock msg;
	Empty empty;
	msg.set_type(MsgType::NEW_PEER);
	seed->stub_->Connect(&context, msg, &empty);
	std::cout << "connect to seed " << seed->peer_ip << ':' << seed->port << std::endl;
	return seed;
}

int main(int argc, char** argv)
{
	std::string server_address("0.0.0.0:50051");
	ChainServerImpl service;
	CNode *seed = NULL;
	if(argc > 1 && argc == 3)
	{//must both provide ip and port, otherwise only serve as server
		seed = connect_seed(argv[1], argv[2]);
		if(NULL == seed) //could not connect seed
			return 1;
		service.cnodes.push_back(seed);
	}

	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;
	server->Wait();
	delete seed;
	return 0;
}

