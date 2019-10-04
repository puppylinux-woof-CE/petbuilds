//
// MPSCAN GPL2
// Last update 01 Mar 2003 
//

//======================================================================
// scan.h
#define S_RAW 0
#define S_CON 1

int verbose_level(void);
int scan(int s_port,int e_port,char **ip,int len,int typ,int verbose);


// net.h
#include <unistd.h>
#define ST_CONNECT   0
#define ST_ERROR     1
#define ST_WAIT      2
#define ST_NOCONNECT 3
#define ST_FAIL      4
#define ST_NORESOURCE 5

#define close_socket(x) close(x)

int connect_to(long int ip, int port, int *status);
void set_verbose(int i);
char *inet_htoa(long int ip);
char *net_portname(int port);
char *net_portprota(int port);

void reset_list(void);
void add_to_list(int sock);
int is_in_list(int sock);
int listen_list(void);
void get_con_status(int sock,int *status);
int get_timeout(void);
void set_timeout(int sec);

// lists.h

#define H_TEST    0
#define H_TESTRUN 1
#define H_RUN     2
#define H_DONE    3

#define P_TEST    0
#define P_WAIT    1
#define P_FAIL    2
#define P_OK      3

/* ports list */
typedef struct{
	int port;
	int status;
	int socket;
} PElement;

typedef struct pnode *plist;
struct pnode{
	PElement info;
	plist next;
};

#define NULL_plist ((plist)0)
#define mkPEmpty() (NULL_plist)
#define isPEmpty(l) (l==mkPEmpty())

plist addp_one(PElement info,plist l);
void free_plist(plist l);

/* ip list */
typedef struct{
	long int ip;
	int status;
	plist ports;
} Element;

typedef struct Node* list;
struct Node{
	Element info;
	list next;
};

#define NULL_LIST ((list)0)
#define mkEmpty() (NULL_LIST)
#define isEmpty(l) (l==mkEmpty())

list add_one(Element info,list l);
void free_list(list l);
void print_ip(list l);
void print_ports(list l);

void free_ports(list l);
int init_ports(int s,int e,list l);

int is_open_connection(list l);
int connections(list l);
int listen_results(list l);

//======================================================================
//                            lists.c
//======================================================================

#include <stdio.h>	      
#include <stdlib.h>

static void printh_stat(int s){
	switch(s){
	case H_TEST:
		printf("Host test");
		return;
	case H_TESTRUN:
		printf("Host testrun");
		return;
	case H_RUN:
		printf("Host scan");
		return;
	case H_DONE:
		printf("Host done");
	}	
}

static void printp_stat(int s){
	switch(s){
	case P_TEST:
		printf("port test");
		return;
	case P_WAIT:
		printf("port wait");
		return;
	case P_FAIL:
		printf("port fail");
		return;
	case P_OK:
		printf("port OK");
	}	
}

static void print_succese(long int ip,int port){
	printf("ip: %s %d OK",inet_htoa(ip),port);
	if(net_portname(port)!=NULL)
		printf(" service:%s",net_portname(port));
	if(net_portprota(port)!=NULL)
		printf(" protocol:%s",net_portprota(port));
	printf("\n");
}


static void print_fail(long int ip,int port){
	if(verbose_level()==0)
		return;
	printf("ip: %s %d FAIL",inet_htoa(ip),port);
	if(net_portname(port)!=NULL)
		printf(" service:%s",net_portname(port));
	if(net_portprota(port)!=NULL)
		printf(" protocol:%s",net_portprota(port));
	printf("\n");
}

/* post list */
plist addp_one(PElement info,plist l){
	plist res=malloc(sizeof(*res));
	if(res==NULL)
		return mkPEmpty();
	res->next=l;
	res->info=info;
	return res;
}

void free_plist(plist l){
	if(isPEmpty(l))
		return;
	free_plist(l->next);
	free(l);
}

void print_port(plist l){
	if(isPEmpty(l))
		return;
	print_port(l->next);
	printf("Port %d ",l->info.port);
	printp_stat(l->info.status);
	if(net_portname(l->info.port)!=NULL)
		printf(" service:%10s",net_portname(l->info.port));
	if(net_portprota(l->info.port)!=NULL)
		printf(", protocol:%4s",net_portprota(l->info.port));
	printf("\n");
}

/* ip list */
list add_one(Element info,list l){
	list res=malloc(sizeof(*res));
	if(res==NULL)
		return mkEmpty();
	res->next=l;
	res->info=info;
	return res;
}

void free_list(list l){
	if(isEmpty(l))
		return;
	free_list(l->next);
	free(l);
}

void print_ip(list l){
	if(isEmpty(l))
		return;
	print_ip(l->next);	
	printf("ip:\t%s\t",inet_htoa(l->info.ip));
	printh_stat(l->info.status);
	printf("\n");
}

void print_ports(list l){
	if(isEmpty(l))
		return; 
	print_ports(l->next);
	printf("ip:\t%s\t",inet_htoa(l->info.ip));
	printh_stat(l->info.status);
	printf("\n");
	print_port(l->info.ports);
}


void free_ports(list l){
	if(isEmpty(l))
		return;
	free_plist(l->info.ports);
	free_ports(l->next);
}

static plist init_port(int s,int e){
	int i;
	plist res=mkPEmpty();
	for(i=s;i<=e;i++){
		PElement in;
		in.port=i;
		in.status=P_TEST;
		res=addp_one(in,res);
		if(isPEmpty(res))
			return mkPEmpty();
	}
	return res;
}

int init_ports(int s,int e,list l){
	if(isEmpty(l))
		return 0;
	l->info.ports=init_port(s,e);
	if(isPEmpty(l->info.ports))
		return -1;
	return init_ports(s,e,l->next);	
}

int is_open_connection(list l){
	if(isEmpty(l))
		return 0;
	if(l->info.status == H_TEST 
	|| l->info.status == H_TESTRUN
	|| l->info.status == H_RUN )
		return 1;
	return is_open_connection(l->next);
}

static plist last_p(plist p){
	if(isPEmpty(p->next))
		return p;
	return last_p(p->next);	
}

static int open_one_port(list l){
	int status;
	plist p=last_p(l->info.ports);
	p->info.socket=connect_to(l->info.ip,p->info.port,&status);
	switch(status){
	case ST_CONNECT:
		l->info.status=H_RUN;
		p->info.status=P_OK;
		print_succese(l->info.ip,p->info.port);
		return 0;
	case ST_ERROR:
		return -1;
	case ST_WAIT:
		l->info.status=H_TESTRUN;
		p->info.status=P_WAIT;
		return 0;
	case ST_NOCONNECT:
		l->info.status=H_RUN;
		p->info.status=P_FAIL;
		print_fail(l->info.ip,p->info.port);
		return 0;
	case ST_FAIL:	
		l->info.status=H_DONE;
		p->info.status=P_FAIL;
		print_fail(l->info.ip,p->info.port);
		return 0;
	case ST_NORESOURCE:
		return 1;
	}
	return 0;
}

static int open_all_ports_1(list l,plist p){
	if(isPEmpty(p))
		return 0;
	if(p->info.status==P_TEST){
		int status;
		p->info.socket=connect_to(l->info.ip,p->info.port,&status);
	
		switch(status){
		case ST_CONNECT:
			p->info.status=P_OK;
			print_succese(l->info.ip,p->info.port);
			break;
		case ST_ERROR:
			return -1;
		case ST_WAIT:
			p->info.status=P_WAIT;
			break;
		case ST_NOCONNECT:
			p->info.status=P_FAIL;
			print_fail(l->info.ip,p->info.port);
			break;
		case ST_FAIL:	
			p->info.status=P_FAIL;
			print_fail(l->info.ip,p->info.port);
			break;
		case ST_NORESOURCE:
			return 1;
		}
	}
	return open_all_ports_1(l,p->next);
}

static int open_all_ports(list l){
	return open_all_ports_1(l,l->info.ports);
}

int connections(list l){
	int k=0;
	if(isEmpty(l))
		return 0;
	switch(l->info.status){
	case H_TEST:
		k=open_one_port(l);
		break;
	case H_RUN:
		k=open_all_ports(l);
		break;
	}	
	if(k==-1)
		return -1;
	else if(k==1)
		return 0;
	return connections(l->next);	
}

static void set_listen_to_ports(plist p){
	if(isPEmpty(p))
		return;
	if(p->info.status==P_WAIT)
		add_to_list(p->info.socket);
	set_listen_to_ports(p->next);	
}

static void set_listen_ports(list l){
	if(isEmpty(l))
		return;
	if(l->info.status==H_TESTRUN || l->info.status==H_RUN)
		set_listen_to_ports(l->info.ports);	
	set_listen_ports(l->next);		
}

static void check_ports_reult(list l,plist p){
	if(isPEmpty(p))
		return;
	if(p->info.status==P_WAIT){
		if(is_in_list(p->info.socket)){
			int status;
			get_con_status(p->info.socket,&status);
			
			switch(status){
			case ST_CONNECT:
				p->info.status=P_OK;
				print_succese(l->info.ip,p->info.port);
			break;
			/*case ST_ERROR:
				return -1;*/
			case ST_NOCONNECT:
				p->info.status=P_FAIL;
				print_fail(l->info.ip,p->info.port);
				break;
			case ST_FAIL:	
				p->info.status=P_FAIL;
				print_fail(l->info.ip,p->info.port);
				break;
			/*case ST_NORESOURCE:
				return 1;*/
			}
		}
	}
	check_ports_reult(l,p->next);
}

static int check_if_done(plist p){
	if(isPEmpty(p))
		return 1;
	if(p->info.status==P_TEST
	|| p->info.status==P_WAIT)
		return 0;
	return check_if_done(p->next);	
}


static int check_ports_one_reult(list l){
	int status;
	plist p=last_p(l->info.ports);
	if(!is_in_list(p->info.socket))
		return 0;
	get_con_status(p->info.socket,&status);
	switch(status){
	case ST_CONNECT:
		l->info.status=H_RUN;
		p->info.status=P_OK;
		print_succese(l->info.ip,p->info.port);
		return 0;
	case ST_ERROR:
		return -1;
	case ST_WAIT:
		l->info.status=H_TESTRUN;
		p->info.status=P_WAIT;
		return 0;
	case ST_NOCONNECT:
		l->info.status=H_RUN;
		p->info.status=P_FAIL;
		print_fail(l->info.ip,p->info.port);
		return 0;
	case ST_FAIL:	
		l->info.status=H_DONE;
		p->info.status=P_FAIL;
		print_fail(l->info.ip,p->info.port);
		return 0;
	case ST_NORESOURCE:
		return 1;
	}
	return 0;
}

static void check_result(list l){
	if(isEmpty(l))
		return;
	if(l->info.status==H_TESTRUN)
		check_ports_one_reult(l);
	if(l->info.status==H_RUN){
		check_ports_reult(l,l->info.ports);
		if(check_if_done(l->info.ports))
			l->info.status=H_DONE;
	}
	check_result(l->next);	
}

int listen_results(list l){
	reset_list();
	set_listen_ports(l);
	switch(listen_list()){
	case -1: return -1;
	case 2: return 1;
	}
	check_result(l);
	return 0;
}


//======================================================================
//                            net.c
//======================================================================

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/select.h>
#include <stdlib.h>
#include <sys/time.h> 
extern int errno;
static fd_set wfd;
static int maxfd=0;
static int timeout_sec=2;

char *inet_htoa(long int ip){
        struct in_addr adr;
        adr.s_addr=(unsigned long int)htonl(ip);
        return inet_ntoa(adr);
}

static char *_net_namesp(int port,int c){
	struct servent *info = getservbyport(htons(port), NULL);
	if(info==NULL)
		return NULL;
	if(c==0)
		return info->s_name;
	else 
		return info->s_proto;
}

char *net_portname(int port){
	return _net_namesp(port,0);
}

char *net_portprota(int port){
	return _net_namesp(port,1);	
}

void reset_list(void){
	maxfd=0;
	FD_ZERO(&wfd);
}

void add_to_list(int sock){
	FD_SET(sock,&wfd);
	maxfd=(maxfd>sock?maxfd:sock);
}

int is_in_list(int sock){
	return FD_ISSET(sock,&wfd);
}

int get_timeout(void){
	return timeout_sec;
}

void set_timeout(int sec){
	timeout_sec=sec;
}

#define MAXOUT 0
int listen_list(void){
	int ret;
	static int outs=0;
	struct timeval timeout;
	timeout.tv_sec=get_timeout();
	timeout.tv_usec=0;
	ret=select(maxfd + 1, NULL, &wfd, NULL,&timeout);
	if(ret==0){
		outs++;
		fprintf(stderr,"Timeout: %d\n",outs);
		if(outs>MAXOUT){
			fprintf(stderr,"Reach max number of timeouts\n");
			return 2;
		}	
	} else 
		outs=0;
	if(ret==-1){
		switch(errno){
		default:
		case ENOMEM:
		case EINVAL:
		case EBADF:
			perror("select:");
			return -1;
		case EINTR:
			return 1;
		}
	}	
	return 0;
}

static void connetc_error(int er,int *status){
	errno=er;
	switch(er){
	case ECONNREFUSED:
		*status=ST_NOCONNECT;
		if(verbose_level()>3)
			perror("connect_to:");
		break;
	case EINPROGRESS:
	case EALREADY:
		*status=ST_WAIT;
		if(verbose_level()>=3)
			perror("connect_to:");
		break;
	case ETIMEDOUT:
	case ENETUNREACH:
	case EHOSTUNREACH:
	case EHOSTDOWN:
		*status=ST_FAIL;
		if(verbose_level()>=3)
			perror("connect_to:");
		break;
	case EAGAIN:
	case EMFILE:
	case ENFILE:
		*status=ST_NORESOURCE;
		if(verbose_level()>=3)
			perror("connect_to:");
		break;
	case EBADF:
	case EFAULT:
	case ENOTSOCK:
	case EISCONN:
	case EADDRINUSE:
	case EAFNOSUPPORT:
	case EACCES:
	case EPERM:
		*status=ST_ERROR;
		perror("connect_to");
		break;
	default:
		perror("connect_to");
		exit(-1);
	}	
}

void get_con_status(int sock,int *status){
	int rg;
	socklen_t len=sizeof(rg);
	if(getsockopt(sock,SOL_SOCKET, SO_ERROR,&rg, &len)==-1){
		*status=ST_ERROR;
		return;
	}
	if(rg==0){
		*status=ST_CONNECT;
	} else 
		connetc_error(rg,status);
	if(*status!=ST_WAIT)
		close_socket(sock);
}	

int connect_to(long int ip, int port, int *status){
	struct sockaddr_in serv_addr;
	int res;

	memset(&serv_addr,0, sizeof(struct sockaddr_in));
	res=socket(PF_INET, SOCK_STREAM, 0);
	if(res==-1){
		switch(errno){
		case ENOBUFS:
		case ENOMEM:
		case ENFILE:
		case EMFILE:
			*status=ST_NORESOURCE;
			if(verbose_level()>=2)
				perror("connect_to:");
			break;
		default :
		case EINVAL:
		case EACCES:
		case EPROTONOSUPPORT:
		case EAFNOSUPPORT:
			*status=ST_ERROR;
			perror("connect_to:");
		}
		return -1;
	}
	if(fcntl(res,F_SETFL,O_NONBLOCK)==-1){
		close_socket(res);
		*status=ST_ERROR;
		perror("connect_to (fcntl): ");
		return -1;
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr= htonl(ip);
	serv_addr.sin_port = htons(port);
	if(verbose_level()>=3) 
		printf("connect_to: ip %s, port %d\n",inet_htoa(ip),port);
	if(connect(res,(struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in))==-1){
		connetc_error(errno,status);
		if(*status==ST_WAIT)
			return res;
	} else 
		*status=ST_CONNECT;
	close_socket(res);
	return 0;
}

//======================================================================
//                            scan.c
//======================================================================

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>	      
#include <stdlib.h>

static list hlist;
static int verb;

int verbose_level(void){
	return verb;
}

/* ip host byte order */
static long int next_ip_(char **ip,int len){
	static int p=0;
	static int num=0;
	
	if(p>=len)
		return -2;
	if(*ip[p]=='-'){
		long int max;
		long int base;
		if(p+1>=len)
			return -2;
		max=inet_network(ip[p+1]);
		if(max==-1){
			p+=2;
			return -1;
		}
		num++;
		base=inet_network(ip[p-1]);
		if(base+num<max && base != -1)
			return inet_network(ip[p-1])+num;
		p++;	
	}  
	num=0;
	return inet_network(ip[p++]);
}

static long int next_ip(char **ip,int len){
	long int res;
	while((res=next_ip_(ip,len))==-1);
	return res;
}


static int init_host_ip(char **ip,int len){
	long int hip;
	hlist=mkEmpty();
	while((hip=next_ip(ip,len))!=-2){
		Element in;
		in.ip=hip;
		in.status=H_TEST;
		in.ports=mkPEmpty();
		hlist=add_one(in,hlist);
		if(isEmpty(hlist))
			return -1;
	}

	return 0;
}

static void close_hosts(void){
	free_ports(hlist);
	free_list(hlist);
}


int scan(int s_port,int e_port,char **ip,int len,int typ,int verbose){
	verb=verbose;
	if(init_host_ip(ip,len)!=0)
		return -1;
	if(init_ports(s_port,e_port,hlist)!=0)
		return -1;
	if(verbose==2)
		print_ip(hlist);
	else if(verbose>=3)
		print_ports(hlist);
	if(connections(hlist)==-1)
		exit(-1);
	while(is_open_connection(hlist)){
		switch(listen_results(hlist)){
		case -1: exit(-1);
		case 1:  exit(0);
		}
		if(connections(hlist)==-1)
			exit(-1);
	}
	close_hosts();
	return 0;
}

//======================================================================
//                            MAIN
//======================================================================

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

static int stealth=0;
static int verbose=0;
static int s_port=1;
static int e_port=1;

static void version(void){
	printf("mpscan 0.1.0 \n");
}

static void usage(char *name){
	version();
	printf("Suggestions, comments, and bugs to: Markus Fraczek (marekf@gmx.net)\n\n");
	printf("usage: %s [-V] | [-h] | [-v] [-t sec] [-p N] [-e N] IP\n",name);
	printf("\n" 
	       "-V\t\tPrints Version\n"
	       "-h\t\tshow this usage message\n"
	       "-v\t\tverbose, -vvv more verbose\n"
	       "-t\t\ttimeout sec\n"
	       "-p\t\tfirst port\n"
	       "-e\t\tlast port\n"
	       "IP:\t\tlist or range\n"
	       "\nexample:\t%s -p 22 192.168.1.1 - 192.168.1.10\n"
	         "        \t%s -p 53 -e 101 192.168.1.1\n",name,name);
}

static int init(int argc, char **argv){
	int c;	
	while((c=getopt(argc,argv,"hVvp:e:st:"))!=-1){
		switch(c){
		case 's':
		        /* TODO: stealth scanner */		
			stealth=1;	
			break;
		case 'V':
			version();
			exit(0);
		case 'v': 
			verbose++;
			break;
		case 'p': 
			if(optarg){
				s_port=atoi(optarg);	
				if(e_port < s_port)
					e_port=s_port;
			}
			break;
		case 'e':
			if(optarg)
				e_port=atoi(optarg);
			if(e_port < s_port){
				usage(argv[0]);
				return -1;
			}	
			break;	
		case 't':
		        if(optarg){
				int timeout=atoi(optarg);
				set_timeout(timeout);
			}
			break;
		case 'h':
			usage(argv[0]);
			exit(0);
		case '?':
			break;
		}	
	}
	if (optind == argc) {
		usage(argv[0]);
		return -1;
	}	
	return optind;	
}

int main(int argc, char **argv)
{
	if(init(argc,argv)==-1)
		return -1;
	if(scan(s_port,e_port,argv+optind,argc-optind,S_CON,verbose)!=0)
		return -1;
	return 0;
}
