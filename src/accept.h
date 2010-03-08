typedef int	ListenAddress;

extern ListenAddress NetServerInitSocket();
extern PortDescriptor *NetServerAccept();
extern int NetRead();
extern int NetServerWrite();
extern int NetCloseConnection();
extern int NetIsThereInput();

