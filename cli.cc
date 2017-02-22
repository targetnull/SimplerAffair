#if 0
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unistd.h> //to parse command-line arguments

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
#endif
#include "cli.h"

int main(int argc, char **argv)
{
	std::string server_port("50051"); //
	char opt;
	while ((opt = getopt(argc, argv, "p:")) != -1) {
		switch (opt)
		{
		case 'p':
			server_port = argv[1]; //only change port
			break;
		default: /* '?' */
			fprintf(stderr, "Usage: %s [-p port]\n",
				argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	ClientContext context;
	//cli can only change port, but not ip, ip should always be localhost(No remote access allowed at present)
        std::unique_ptr<ChainServer::Stub> stub_ = ChainServer::NewStub(grpc::CreateChannel("localhost:"+server_port, grpc::InsecureChannelCredentials()));

	//since it is a local command, we don't need the server to connect back.
	//directly send command
	MsgBlock msg;
	Empty empty;
	msg.set_type(MsgType::GET_CMD);
	msg.set_data("This is my command, do it now!\n");	 
	stub_->SendMsg(&context, msg, &empty);
	//let's try
	context.TryCancel();

	return 0;
}

