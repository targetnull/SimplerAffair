#include "server.h"
#include "deserialize.h"
#include "block.h"
#include "transaction.h"

#define __CMD_HANDLER   //just helper macro

static Status __CMD_HANDLER sendBlock(const std::string &data)
{
	std::cout << "going to send message: " << data << std::endl;
	//TODO.
	MsgBlock msg;

	return Status::OK;
}

static Status __CMD_HANDLER getBlock(lBlock &block)
{
	std::cout << "get block!!\n";
	if(!VerifyBlock(&block)){
		std::cerr << "block modified or damaged.\n";
		return Status::CANCELLED;
	}

	//then deal with each transaction
	ProcessTxns(&block);

	return Status::OK;
}

//received command from local cli
static Status __CMD_HANDLER cliCmd(const std::string &msg)
{
	std::cout << " this is a command from local cli: " << msg << std::endl;
	return Status::OK;
}

//addr, the incoming addr, parse and store it in `ip` and `port`
static void parseAddr(const std::string &addr, std::string &ip, std::string &port)
{
	//TODO. should check if ipv4 or ipv6, assume ipv4 at present
	int pos1 = addr.find(':');
    int pos2 = addr.find(':', pos1 + 1);
	ip = addr.substr(pos1 + 1, pos2 - pos1 - 1);
	port = addr.substr(pos2 + 1);
}

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

Status ChainServerImpl::Connect(ServerContext* context, const MsgBlock* msg, Empty* empty)
{
	std::string peer_ip;
	std::string peer_port;
	parseAddr(context->peer(), peer_ip, peer_port);
	std::cout << "Receive connection from " << peer_ip << ' ' << peer_port << std::endl;
	//then establish a reverse connection
	CNode *node = new CNode(peer_ip, peer_port);
	cnodes.push_back(node);
	return Status::OK;
}

Status ChainServerImpl::SendMsg(ServerContext* context, const MsgBlock* msg, Empty* empty)
{
	Status res;
	std::string peer_addr = context->peer();
#ifndef NO_PARSE
	std::string ip;
	std::string port;
	parseAddr(peer_addr, ip, port);
	std::cout << "Received message from " << ip << ' ' << port << std::endl;
#else
	std::cout << "Received message from " << peer_addr << std::endl;
#endif

	std::cout << "message type [before] is " << msg->type() << std::endl;

	//deserialize MsgBlock to local class object to simplify the process.
	lMsgBlock *newmsg = new lMsgBlock;
	deserial_msgblock(msg, newmsg);
	inmsg.bq.push(*newmsg);

	return Status::OK;
}

void ChainServerImpl::eventHandler(InboundMsg &inmsg)
{
	lMsgBlock msg;
	while(true)
	{
		inmsg.bq.wait_and_pop(msg); //wait until there is an element
		std::cout << "msg type is " << msg.type << std::endl;
		switch(msg.type)
		{
			case MsgType::GET_BLOCK:
				getBlock(msg.block);
				break;
			case MsgType::SEND_BLOCK:
				sendBlock(msg.data);
				break;
			case MsgType::GET_CMD: //local command
				cliCmd(msg.data);
				break;
			case MsgType::QUIT:
				//TODO. should check if it is from the server or the peer
				//currently lets take it as from server.
				return; //quit loop
			default:
				//res = Status::CANCELLED;
				break;
		}
	}
}

//to simplify the method
void ChainServerImpl::listenEvent()
{
	thread_id = std::thread(ChainServerImpl::eventHandler, std::ref(inmsg));
}

int main(int argc, char** argv)
{
	std::string server_address("0.0.0.0:50051");
	ChainServerImpl service;
	CNode *seed = NULL;
	if(argc > 1 && argc == 3)
	{ //must both provide ip and port, otherwise only serve as server
		seed = connect_seed(argv[1], argv[2]);
		if(NULL == seed) //could not connect seed
			return 1;
		service.PushNode(seed);
	}

	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;

	//create a thread that constantly check the event queue
	service.listenEvent();

	server->Wait();
	delete seed;
	return 0;
}

