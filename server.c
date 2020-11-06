
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <assert.h>
#include "ticker_prot.h"
#include "minirpc.h"
#include <rpc/svc.h>
#include <sys/queue.h>

void ticker_prog_1 (struct svc_req *rqstp, SVCXPRT *transp);

char *progname;
u_int32_t my_id;		/* This server's unique ID */
int nothers;			/* Number of other servers */
struct sockaddr_in **others;	/* Addresses of the other servers,
				 * followed by a NULL pointer */
int isFirstTime = 1;

int timestamp = 0;
struct trade
{
  char* tradeInfo;
  int svc_id;
  int timestamp;
};

long int latestRecieved = -1;

int listSize = 0;
struct trade tradeList[10000];
struct trade sortedList[10000];
int sortedNum = 0;;
//want_timer = 1;

FILE *fp;

long getCurrentTime()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*1000+tv.tv_usec/1000;
}


submit_result *
ticker_xaction_1_svc(xaction_args *arguments,struct svc_req *rqstp)
{
  static submit_result broadcast_result;
  //set the return value 
  broadcast_result.ok = TRUE;
  
  //  printf("the timestamp recieved is: %d\n",arguments->ts);
  //printf("The sender id is: %d\n",arguments->my_id);
  //  printf("%s\n",arguments->msg);
  int tsRecieved = arguments->ts;
    
  //set the timestamp into the larger value + 1
  if(timestamp <= tsRecieved){
    timestamp = tsRecieved + 1;
  }else{
    timestamp = timestamp + 1;
  }
  if(want_timer == 0 ){
    isFirstTime = 1;
    want_timer = 1;
  }else if(want_timer != 0){
    isFirstTime = 0;
  }
  
  // want_timer = 1;
  
  //stored the trade into tradeList
  struct trade tradeRecieved = {0};
  //tradeRecieved.tradeInfo = arguments->msg;
  char* p = malloc(sizeof(char)*200);
  strncpy(p,arguments->msg,sizeof(char)*200);
  tradeRecieved.tradeInfo = p;
  //strcpy(&tradeRecieved.tradeInfo,&(arguments->msg));
  //printf("tradeinfo is: %s(during recieved)\n",tradeRecieved.tradeInfo);
  tradeRecieved.svc_id = arguments->my_id;
  tradeRecieved.timestamp = tsRecieved;
  //printf("listSize is :%d\n",listSize);
  tradeList[listSize] = tradeRecieved;
  //printf("does the trade recieved--? %d\n",tradeList[0].timestamp);
  listSize++;
  latestRecieved = elapsed;
  return (&broadcast_result);
}




submit_result *
ticker_submit_1_svc (submit_args *argp, struct svc_req *rqstp)
{
  static submit_result result;
  /*
   * XXX - You must write this code
   */
  if(want_timer == 0 ){
    isFirstTime = 1;
    want_timer = 1;
  }else if(want_timer != 0){
    isFirstTime = 0;
  }
  //want_timer = 1;
  static xaction_args test;
  test.my_id = my_id;
  timestamp = timestamp + 1;
  test.ts = timestamp;
  test.msg = argp->msg;
   rpc_broadcast(others,TICKER_PROG,TICKER_VERS,TICKER_XACTION,(xdrproc_t)xdr_xaction_args,&test);

   //stored the trade which was sended to others
   struct trade tradeSended = {0};
   //tradeSended.tradeInfo = argp->msg;
   char* p = malloc(sizeof(char)*200);
   strncpy(p,argp->msg,sizeof(char)*200);
   tradeSended.tradeInfo = p;
   tradeSended.svc_id = my_id;
   tradeSended.timestamp = timestamp;
   tradeList[listSize] = tradeSended;
   //printf("tradeList sended value is %d\n",tradeList[0].timestamp);
  result.ok = TRUE;
  listSize++;
  latestRecieved = elapsed;
  
  return (&result);
}


void sort(void){
  fprintf(fp,"----------------------------------------------\n");
  for(int x = 0; x < listSize; x ++){
    //fprintf(fp,"trade %d -- ts = %d -- tI = %s\n",x,tradeList[x].timestamp, tradeList[x].tradeInfo);
  }
  
  //printf("listSize in sort is %d\n",listSize);
  //firstly order the whole list
  int distinct = 0;
  int tmp[1000];
  memset(tmp,0,1000*sizeof(int));
  int diff;
  //get the distinct number of value
  for(int i= 0; i < listSize ; i++){
    diff = 1;
    //fprintf(fp,"In loop: trade %d -- ts %d\n",i,tradeList[i].timestamp);
    for(int j = 0 ;j < listSize; j++ ){
      if(tradeList[i].timestamp == tmp[j]){
	diff = 0;
	break;
      }
    }
    if(diff) tmp[distinct++] = tradeList[i].timestamp; 
  }
  
  
  //fprintf(fp,"distinct number is %d\n",distinct);
  //struct trade sortedList[10000];
  struct trade tsList[1000];
  int count,min;
  int totalCount=0;
  sortedNum = 0;
  int lastMin = 0;
  if(listSize != 0){
    for(int a = 0; a < distinct ; a++){
      min = 1000;
      //find the minimum number in the list
      for(int i = 0; i < listSize; i++){
	if(tradeList[i].timestamp <= min && tradeList[i].timestamp > lastMin) min = tradeList[i].timestamp;      
      }
      lastMin = min;
      //printf("min is: %ds\n",min);
      //count the number of trade whose timestamp equals to min 
      count = 0;
      //find trade whose timestamp equals to a
      for(int m = 0; m < listSize; m++){
	if(tradeList[m].timestamp == min){
	  struct trade tmp ;
	  tmp.tradeInfo = tradeList[m].tradeInfo;
	  tmp.timestamp = tradeList[m].timestamp;
	  tmp.svc_id = tradeList[m].svc_id;
	  tsList[count] = tmp;
	  //set the timestamp to 10000
	  //tradeList[m].timestamp = 10000;
	  count = count + 1;
	  }
	}
      totalCount = totalCount + count;
      /*
      printf("---------------------------------------\n");
      for(int a = 0; a < count; a++){
	printf("tslist %d - %d - %s\n",a,tsList[a].timestamp,tsList[a].tradeInfo);
      }
      printf("----------------------------------------\n");
      */
      if(count == 1){
	sortedList[sortedNum] = tsList[count-1];
	sortedNum++;
      }
      if(count > 1){
	int i = 0;
	int j = 0;
	struct trade tmpp;
	for(i = 0; i < count - 1; i++){
	  for(j = 0; j < count-i-1; j++){
	    if(tsList[j].svc_id > tsList[j+1].svc_id){
	      tmpp = tsList[j+1];
	      tsList[j+1] = tsList[j];
	      tsList[j] = tmpp;
	    }
	  }
	}
	for(i = 0; i < count; i++){
	  sortedList[sortedNum] = tsList[i];
	  sortedNum++;
	}
      }
    }
    //fprintf(fp,"total count is: %d\n",totalCount);
  }
  //printf("sortedNum is : %d\n",sortedNum);
  
  //printf("sortedNum is %d\n",sortedNum);
}


/*
 * This function gets called once per second, if you set the global
 * variable want_timer to a positive value.
 *
 * You should use this to print out trades in order.  (You have to
 * delay printing a trade until you know you have heard of and printed
 * any trades that happened before that trade.)
 */
//FILE *fp;
void
timer (void)
{
  /*
   * XXX - You must write this code
   */
  /*
  for(int x = 0; x < listSize; x ++){
    printf("trade %d -- ts = %d -- tI = %s\n",x,tradeList[x].timestamp, tradeList[x].tradeInfo);
  }
  
  //printf("listSize is %d\n",listSize);
  //firstly order the whole list
  int distinct = 1;
  int tmp[listSize];
  int diff;
  //get the distinct number of value
  int i,j;
  if(listSize > 1){
    for(i= 0; i < listSize ; i++){
      diff = 1;
      for(j = 0 ;j < listSize; j++ ){
	if(tradeList[i].timestamp == tmp[j]){
	  diff = 0;
	  break;
	}
      }
      if(diff) tmp[distinct++] = tradeList[i].timestamp; 
    }
  }else{
    distinct = 1;
  }
  
  
  printf("distinct number is %d\n",distinct);
  struct trade sortedList[10000];
  struct trade tsList[1000];
  int count,min,sortedNum;
  sortedNum = 0;
  if(listSize != 0){
    for(int a = 0; a < distinct ; a++){
      min = 1000;
      //find the minimum number in the list
      for(int i = 0; i < listSize; i++){
	if(tradeList[i].timestamp <= min) min = tradeList[i].timestamp;      
      }
      //printf("min is: %ds\n",min);
      //count the number of trade whose timestamp equals to min 
      count = 0;
      //find trade whose timestamp equals to a
      for(int m = 0; m < listSize; m++){
	if(tradeList[m].timestamp == min){
	  struct trade tmp = {0};
	  tmp.tradeInfo = tradeList[m].tradeInfo;
	  tmp.timestamp = tradeList[m].timestamp;
	  tmp.svc_id = tradeList[m].svc_id;
	  tsList[count] = tmp;
	  //set the timestamp to 10000
	  tradeList[m].timestamp = 10000;
	  count = count + 1;
	  }
	}
      
      printf("---------------------------------------\n");
      for(int a = 0; a < count; a++){
	printf("tslist %d - %d - %s\n",a,tsList[a].timestamp,tsList[a].tradeInfo);
      }
      printf("----------------------------------------\n");
      
      if(count == 1){
	sortedList[sortedNum] = tsList[count-1];
	sortedNum++;
      }
      if(count > 1){
	int i = 0;
	int j = 0;
	struct trade tmpp;
	for(i = 0; i < count - 1; i++){
	  for(j = 0; j < count-i-1; j++){
	    if(tsList[j].svc_id > tsList[j+1].svc_id){
	      tmpp = tsList[j+1];
	      tsList[j+1] = tsList[j];
	      tsList[j] = tmpp;
	    }
	  }
	}
	for(i = 0; i < count; i++){
	  sortedList[sortedNum] = tsList[i];
	  sortedNum++;
	}
      }
    }
      
  }
  printf("sortedNum is : %d\n",sortedNum);
  for(int s = 0; s < sortedNum; s++){
    printf("trade sorted %d ts is %d, Info is %s\n",s,sortedList[s].timestamp,sortedList[s].tradeInfo);
  }
  //printf("sortedNum is %d\n",sortedNum);
  */
  if(elapsed - latestRecieved <= 1  && listSize >= 20){
    //fp = fopen("log.txt","w+");
    //printf("listSize before sorted: %d\n",listSize);
    sort();
    //printf("listSize after sorted: %d\n",listSize);
    //printf("listSize is: %d\n",listSize);
    struct trade printList[1000];
    //printf("in first if\n");
    //printf("sortedNum/2 : %d\n",sortedNum/2);
    for(int i = 0; i < (sortedNum/2); i++){
      printf("%s\n",sortedList[i].tradeInfo);
      //fprintf(fp,"%s\n",sortedList[i].tradeInfo);
    }
    for(int i = (sortedNum/2); i < sortedNum; i ++){
      //tradeList[i - (sortedNum/2)] = sortedList[i];
      //int timestamp = sortedList[i].timestamp;
      struct trade tmppp;
      //tmppp = malloc(sizeof(sortedList[i]));
      tmppp.timestamp = sortedList[i].timestamp;
      tmppp.svc_id = sortedList[i].svc_id;
      tmppp.tradeInfo = sortedList[i].tradeInfo;
      tradeList[i-(sortedNum/2)] = tmppp;
    }
    listSize = sortedNum - (sortedNum/2);
    //printf("-----------------------------\n");
    //printf("listSize after: %d\n",listSize);
  }
  if(elapsed - latestRecieved > 1){
    sort();
    //printf("sortedNum is %d\n",sortedNum);
    //printf("in second if! listSize is - %d\n",listSize);
    for(int i =0; i < listSize; i++){
      //printf("in loop\n");
      printf("%s\n",sortedList[i].tradeInfo);
    }
    want_timer = 0;
    //printf("==============================\n");
    listSize = 0;
    
  }
  //printf("=============================\n");

  /*  
  
  //printf("size of tradeList: %d\n",sizeof(tradeList)/sizeof(struct trade));
  if(!(elapsed - latestRecieved <= 1 && isFirstTime == 1)){
    //printf("trade number is: %d\n",listSize);
    //printf("lateset Recieved time is: %d\n",latestRecieved);
    for(int n = 0; n < listSize; n++){
    //msg_t str = tradeList[b].tradeInfo;
      //printf("trade %d -- timestamp is %d --  id is %d -- tradeInfo is %s\n",n,tradeList[n].timestamp,tradeList[n].svc_id,tradeList[n].tradeInfo);
    }
  

    int distinct = 0;
    int tmp[listSize];
    int diff;
    //get the distinct number of value
    int i,j;
    for(i= 0; i < listSize ; i++){
      diff = 1;
      for(j = 0 ;j < listSize; j++ ){
	if(tradeList[i].timestamp == tmp[j]){
	  diff = 0;
	  break;
	}
      }
      if(diff) tmp[distinct++] = tradeList[i].timestamp; 
    }
    //printf("distinct number is %d\n",distinct);
    struct trade printList[10000];
    int count,min;
    if(listSize != 0){
      for(int a = 0; a < distinct ; a++){
	min = 10000;
	//find the minimum number in the list
	for(int i = 0; i < listSize; i++){
	  if(tradeList[i].timestamp <= min) min = tradeList[i].timestamp;	
	}
	//printf("minimum number found is : %d\n",min);
	
	//count the number of trade whose timestamp equals to min 
	count = 0;
      //find trade whose timestamp equals to a
	for(int m = 0; m < listSize; m++){
	  if(tradeList[m].timestamp == min){
	    //printf("trade's timestamp is: %d\n",tradeList[m].timestamp);
	    struct trade tmp = {0};
	    //char tmpInfo[79] = tradeList[m].tradeInfo;
	    tmp.tradeInfo = tradeList[m].tradeInfo;
	    tmp.timestamp = tradeList[m].timestamp;
	    tmp.svc_id = tradeList[m].svc_id;
	    printList[count] = tmp;
	    //printf("printlist trade %s sended by server %d",printList[count].tradeInfo,printList[count].svc_id);
	    tradeList[m].timestamp = 10000;
	    count = count + 1;
	    //printf("value in the printList in the loop is %d\n",tmp.timestamp);
	  }
	}
	//printf("corrent a value is - %d == min value is - %d == count value is - %d\n",a,min,count);
	//printf("tradeList 1 information is: %s\n",printList[0].tradeInfo);
	if(count == 1){
	  printf("%s\n",printList[0].tradeInfo);
	  //printf("tradeInfo is: %s\n",printList[0].tradeInfo);
	}
	if(count > 1){
	  int i = 0;
	  int j = 0;
	  struct trade tmpp;
	  for(i = 0; i < count - 1; i++){
	    for(j = 0; j < count-i-1; j++){
	      if(printList[j].svc_id > printList[j+1].svc_id){
		tmpp = printList[j+1];
		printList[j+1] = printList[j];
		printList[j] = tmpp;
	      }
	      
	    }
	  }
	  for(i = 0; i < count; i++){
	    printf("%s\n",printList[i].tradeInfo);
	  }
	}
      }
      
    }
    
    listSize = 0;
    //printf("=================================\n");
    //want_timer = 0;
  }
  else if(elapsed - latestRecieved > 1 ){
    //printf("%s\n",tradeList[listSize].tradeInfo);
    //listSize = 0;
    want_timer = 0;
  }
  */
  
}

/*
 *  You don't have to change anything below here
 */

static void
init_others (int no, char **av)
{
  int i;
  struct hostent *hp;
  int failed = 0;

  nothers = no;
  others = malloc ((nothers + 1) * sizeof (others[0]));
  if (!others) {
    fprintf (stderr, "out of memory\n");
    exit (1);
  }
  others[nothers] = NULL;	/* So end of list is NULL */

  for (i = 0; i < nothers; i++) {
    others[i] = malloc (sizeof (*others[i]));
    if (!others[i]) {
      fprintf (stderr, "out of memory");
      exit (1);
    }
    bzero (others[i], sizeof (*others[i]));
    others[i]->sin_family = AF_INET;
    hp = gethostbyname (*av++);
    if (!hp) {
      fprintf (stderr, "%s: could not find address of host %s\n",
	       progname, av[-1]);
      failed = 1;
    }
    others[i]->sin_addr = *(struct in_addr *) hp->h_addr;
    others[i]->sin_port = htons (atoi (*av++));
    //printf("this is portnumber %d\n",others[i]->sin_port);
  }

  if (failed)
    exit (1);
}

static void usage (void) __attribute__ ((noreturn));
static void
usage (void)
{
  fprintf (stderr, "usage: %s my-id my-port host1 port1 [host2 port2 ...]\n",
	   progname);
  exit (1);
}

int
main (int argc, char **argv)
{
  int my_sock;
  SVCXPRT *transp;
  fp = fopen("log.txt","w+");
  if ((progname = strrchr (argv[0], '/')))
    progname++;
  else
    progname = argv[0];

  if (argc < 3 || (argc - 3) % 2)
    usage ();

  my_id = atoi (argv[1]);
  my_sock = mkudpsock (atoi (argv[2]));
  init_others ((argc - 3) / 2, argv + 3);
  //printf("my_id is %d\n",&my_id);
  if (!(transp = svcudp_create (my_sock))) {
    fprintf (stderr, "cannot create UDP service\n");
    exit (1);
  }
  if (!svc_register (transp, TICKER_PROG, TICKER_VERS, ticker_prog_1, 0)) {
    fprintf (stderr, "failed to register RPC program\n");
    exit (1);
  }
  //printf("test timestamp: %d\n",getCurrentTime());
  setvbuf (stdout, NULL, _IOLBF, 0);
  //  printf("finish called rpc_run in the end\n");
  rpc_run ();
}
