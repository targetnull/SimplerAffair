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
	CNode(const char *node_ip, const char *node_port) : peer_ip(node_ip), port(node_port) {
		stub_ = ChainServer::NewStub(grpc::CreateChannel(peer_ip + ':' + port, grpc::InsecureChannelCredentials()));
                std::cout << "successfully connect back to sender " << peer_ip << ':' << port << std::endl;
	}

	//assume ipv4 bydefault, but should check when parsing
	CNode(std::string &node_ip, std::string &node_port) : peer_ip(node_ip), port(node_port) {
		stub_ = ChainServer::NewStub(grpc::CreateChannel(peer_ip + ":" + port, grpc::InsecureChannelCredentials()));
		std::cout << "successfully connect back to sender " << peer_ip << ':' << port << std::endl;
	}

	//this way is strongly NOT suggested as it hides the ip and port of the connection
	CNode(std::shared_ptr<Channel> &chan): channel(chan)
        {
                stub_ = ChainServer::NewStub(channel);
                std::cout << "A channel is directly provided :( ";
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
		std::cout << "Received connect from " << peer_addr.substr(pos1, pos2 - pos1 + 1) << std::endl;
		std::cout << "msg type is " << msg->type() << std::endl;
		std::string peer_ip = peer_addr.substr(pos1, pos2 - pos1 + 1);
		std::string peer_port = peer_addr.substr(pos2);
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
private:
	std::vector<CNode *> cnodes;
	//std::unique_ptr<ChainServer::Stub> stub_;
	//std::mutex qlock;
};

int main(int argc, char** argv)
{
	std::string server_address("0.0.0.0:50051");
	CNode *seed = NULL;
	if(argc > 1 && argc == 3)
	{//must both provide ip and port, otherwise only serve as server
		seed = new CNode(argv[1], argv[2]);
		std::cout << "connect to seed " << seed->peer_ip << ':' << seed->port << std::endl;
	}
	ChainServerImpl service;

	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;
	server->Wait();
	delete seed;
	return 0;
}

