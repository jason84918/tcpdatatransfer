#include<cstdio>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/errno.h>
#include<iostream>
#include<fcntl.h>
#include<netdb.h>
#include<cstring>
#include<arpa/inet.h>
#include<unistd.h>
#include<cstdlib>
#include<sstream>
#include<string>
#include<vector>
#include<fstream>
#include<dirent.h>
#define PORT 5859
using namespace std;
int main(void){
    int sockfd,connfd,listenfd;
    socklen_t len;
    struct sockaddr_in servaddr,cliaddr,tempaddr,tempcliaddr;
    int nready;
    char buffer[1000];
    char succeeded[]="succeeded\n";
    char failed[]="failed\n";
    char li[]="LIST succeeded\n";
    int nbytes,n;
    pid_t childpid;
    int tempport=PORT+1;
	struct stat st={0};
	
    //SOCKET

    if((listenfd=socket(AF_INET,SOCK_STREAM,0))<0) perror("socket error from server");

    cout<<"socket success"<<endl;
    //SETSOCKET

    bzero((char*)&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);

    //BIND

    if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0) perror("bind error from server");
    cout<<"bind success"<<endl;
    //LISTEN

    if(listen(listenfd,40)<0) perror("listen error from server");
    cout<<"server running ... waiting for connections"<<endl;

    for(;;){
        len=sizeof(cliaddr);
        connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&len);
        if(connfd==-1){
            perror("accept error form server");
        }
        else{
            if((childpid=fork())==0){//if it��s 0, it��s child process
	            printf ("%s\n","Child created for dealing with client requests");
	              //close listening socket
	            close (listenfd);
				char dir[256];
				sprintf(dir,"%d",connfd);
				if(stat(dir,&st)==-1){
					mkdir(dir,0700);
				}
				bzero(buffer,1000);
				sockfd=connfd;
	            while((nbytes=read(sockfd,&buffer,1000))>0){
	                vector<string> v;
	                string s(buffer);
	                cout<<"sockfd:"<<sockfd<<endl;
					cout<<"buffer:"<<buffer<<endl;
	                stringstream stream;
	                stream<<s;
	                string input;
	                while(stream>>input){
	                    v.push_back(input);
	                }
	                if(v.size()==0) continue;
	                if(v.size()==3&&v[0]=="PUT"){
	                    char errorff[]="source error\n";
	                 	char temp1[256];
	                 	bzero(temp1,256);
	                 	if((read(sockfd,&temp1,256))<0) perror("read error");
	                 	if(!strcmp(temp1,errorff)){
	                        cout<<"source error"<<endl;
	                        continue;
	                 	}
	                 	cout<<temp1<<endl;
						////////create new line for transformation
						int tempfd;
	                    tempport++;
	                    char temp[256];
	                    sprintf(temp,"%d\n",tempport);
	                    if(write(sockfd,temp,256)<0) perror("write error");
	                    if((tempfd=socket(AF_INET,SOCK_STREAM,0))<0) perror("socket error from server");
	                    cout<<"socket success"<<endl;
	                    //SETSOCKET
	                    bzero((char*)&tempaddr,sizeof(tempaddr));
	                    tempaddr.sin_family=AF_INET;
	                    tempaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	                    tempaddr.sin_port=htons(tempport);
	                    //BIND
	                    if(bind(tempfd,(struct sockaddr*)&tempaddr,sizeof(tempaddr))<0) perror("bind error from server");
	                    cout<<"bind success"<<endl;
	                    if(listen(tempfd,40)<0) perror("listen error from server");
	                    cout<<"running ... waiting for data transformation"<<endl;
	                    int fd;
	                    socklen_t templen=sizeof(tempcliaddr);
	                    fd=accept(tempfd,(struct sockaddr*)&tempcliaddr,&templen);
	                    if(fd==-1){
	                        perror("accept error form server");
	                    }
	                    cout<<"accept success"<<endl;
	                    /////////////////////
	                    char data[256];
	                    bzero(data,256);
	                    char length[256];
	                    bzero(length,256);
	                    FILE* fp;
						char dir[256];
						bzero(dir,256);
						sprintf(dir,"%d/",sockfd);
	                    char newsol[256];
	                    bzero(newsol,256);
						sprintf(newsol,"%s%s",dir,v[2].c_str());
						cout<<"newsol:"<<newsol<<endl;
						fp=fopen(newsol,"wb");
	                    if(fp==NULL){
	                        perror("destination error");
	                        exit(1);
	                    }
	                    while((n=read(fd,&data,256))){
							if(write(fileno(fp),data,n)<0) perror("write error");
	                    }
	                    fseek(fp,0,SEEK_END);
	                    long filelen=ftell(fp);
	                    //cout<<"filelen:"<<filelen<<endl;
						rewind(fp);
	                    close(fd);
	                    fclose(fp);
	                    if(read(sockfd,&length,256)<0) perror("read error");
						//cout<<"length:"<<length<<endl;
	                    long datalength=atol(length);
	                    if((filelen==datalength)&&datalength>0){
	                        char successmsg[256];
	                        sprintf(successmsg,"%s %s",buffer,succeeded);
	                        if(write(sockfd,successmsg,strlen(successmsg))<0) perror("write error");
	                    }
	                    else if((filelen!=datalength)&&datalength>0){
	                        char errormsg[256];
	                        sprintf(errormsg,"%s %s",buffer,failed);
	                        if(write(sockfd,errormsg,strlen(errormsg))<0) perror("write error");
	                    }
	                }
	                else if(v[0]=="GET"&&v.size()==3){
	                    FILE* fp;
	                    char dir[256];
						sprintf(dir,"%d/",sockfd);
	                    char newsol[256];
						sprintf(newsol,"%s%s",dir,v[1].c_str());
						cout<<"newsol:"<<newsol<<endl;
						fp=fopen(newsol,"rb");
	                    if(fp==NULL){
	                        char sourceerror[]="source error\n";
	                        cout<<sourceerror;
	                        if(write(sockfd,sourceerror,sizeof(sourceerror))<0) perror("write error");
	                        continue;
	                    }
						////////create new line for transformation
						int tempfd;
	                    tempport++;
	                    char temp[256];
	                    sprintf(temp,"%d\n",tempport);
	                    if(write(sockfd,temp,256)<0) perror("write error");
	                    if((tempfd=socket(AF_INET,SOCK_STREAM,0))<0) perror("socket error from server");
	                    cout<<"socket success"<<endl;
	                    //SETSOCKET
	                    bzero((char*)&tempaddr,sizeof(tempaddr));
	                    tempaddr.sin_family=AF_INET;
	                    tempaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	                    tempaddr.sin_port=htons(tempport);
	                    //BIND
	                    if(bind(tempfd,(struct sockaddr*)&tempaddr,sizeof(tempaddr))<0) perror("bind error from server");
	                    cout<<"bind success"<<endl;
	                    if(listen(tempfd,40)<0) perror("listen error from server");
	                    cout<<"running ... waiting for data transformation"<<endl;
	                    int fd;
	                    socklen_t templen=sizeof(tempcliaddr);
	                    fd=accept(tempfd,(struct sockaddr*)&tempcliaddr,&templen);
	                    if(fd==-1){
	                        perror("accept error form server");
	                    }
	                    cout<<"accept success"<<endl;
	                    /////////////////////
	                    char data1[256];
	                    bzero(data1,sizeof(data1));
	                    int n;
	                    while((n=read(fileno(fp),&data1,256))){
	                        //cout<<"n:"<<n<<endl;
	                        if(write(fd,data1,n)<0) perror("write error");
	                    }
	                    close(fd);
	                    fseek(fp,0,SEEK_END);
	                    long filelen=ftell(fp);
	                    //cout<<"filelen:"<<filelen<<endl;
	                    char length[256];
	                    sprintf(length,"%ld\n",filelen);
	                    cout<<"length:"<<length<<endl;
	                    fclose(fp);
	                    if(write(sockfd,length,256)<0) perror("write error");
	                }
	                else if(v.size()==1&&v[0]=="LIST"){
	                	char dir[256];
						sprintf(dir,"%d/",sockfd);
						DIR *theFolder =opendir(dir);
						struct dirent *next_file;
						while((next_file=readdir(theFolder))!=NULL){
							char* c=next_file->d_name;
							if(strcmp(c,".")&&strcmp(c,"..")){
								char filename[256];
								sprintf(filename,"%s\n",c);
								cout<<filename;
								if(write(sockfd,filename,strlen(filename))<0) perror("write error");
							}
						}
						closedir(theFolder);
						char listsuccessmsg[]="LIST succeeded\n";
						if(write(sockfd,listsuccessmsg,strlen(listsuccessmsg))<0) perror("write error");
						
	                }
	                else{
	                    char errmsg[]="error command\n";
						if(write(sockfd,errmsg,strlen(errmsg))<0) perror("write error");
	                }
	            }
                if(nbytes==0){
                    cout<<"leaving"<<endl;
                    close(sockfd);
					char sol[256];
					sprintf(sol,"rm -r %d/",sockfd);
					system(sol);			
                }
                else{
                    perror("closed incorrectly from client");
                }
                exit(0);
    		}
    	}
    }
}
