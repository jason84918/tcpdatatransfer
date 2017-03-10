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
    fd_set allset,rset;
    int maxfd;
    int client[FD_SETSIZE];
	vector<string> idlist;
    int nready,count,maxi;
    char buffer[1000];
    char succeeded[]="succeeded\n";
    char failed[]="failed\n";
    char li[]="LIST succeeded\n";
    int nbytes,n;
    int tempport=PORT+1;
    vector<vector<string>> book;
	struct stat st={0};
	for(int i=0;i<FD_SETSIZE;i++){
		string temp;
		idlist.push_back(temp);
	}

    //FD_SET CLEAR
    FD_ZERO(&rset);
    FD_ZERO(&allset);

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
    maxfd=listenfd;
    maxi=-1;
    for(int i=0;i<FD_SETSIZE;i++){
        client[i]=-1;
    }
    FD_SET(listenfd,&allset);

    for(;;){
        rset=allset;
        nready=select(maxfd+1,&rset,NULL,NULL,NULL);
        if(nready<0){
            perror("select error from server");
        }
        if(FD_ISSET(listenfd,&rset)){
            len=sizeof(cliaddr);
            connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&len);
            if(connfd==-1){
                perror("accept error form server");
            }
            else{
//                char address1[INET_ADDRSTRLEN];
//                inet_ntop(AF_INET,&(cliaddr.sin_addr),address1,INET_ADDRSTRLEN);
                for(count=0;count<FD_SETSIZE;count++){
                    if(client[count]<0){
                        client[count]=connfd;
                        break;
                    }
                }
                if(count==FD_SETSIZE){
                    perror("too many clients");
                }

            }
            cout<<"someone is linking"<<endl;
            FD_SET(connfd,&allset);
            if(connfd>maxfd) maxfd=connfd;
            if(count>maxi) maxi=count;
	    }
        //cout<<maxi<<endl;
        for(int i=0;i<=maxi;i++){
			bzero(buffer,1000);
            if((sockfd=client[i])<0) continue;
            if(FD_ISSET(sockfd,&rset)&&sockfd!=listenfd){
                if((nbytes=read(sockfd,&buffer,1000))<=0){
                    if(nbytes==0){
                        cout<<"leaving"<<endl;
                        close(sockfd);
                        FD_CLR(sockfd,&allset);
                        client[i]=-1;
                    }
                    else{
                        perror("closed incorrectly from client");
                    }
                }
                else{
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
                        bzero(temp,256);
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
						sprintf(dir,"%s/",idlist[sockfd].c_str());
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
                           // cout<<"n:"<<n<<endl;
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
						sprintf(dir,"%s/",idlist[sockfd].c_str());
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
						sprintf(dir,"%s/",idlist[sockfd].c_str());
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
					else if(v.size()==2&&v[0]=="CLIENTID:"){
						idlist[sockfd]=v[1];
						if(stat(v[1].c_str(),&st)==-1){
							mkdir(v[1].c_str(),0700);
						}

					}
                    else{
                        char errmsg[]="error command\n";
						if(write(sockfd,errmsg,strlen(errmsg))<0) perror("write error");
                    }
                }
            if(--nready<=0) break;
            }
        }
    }
}
