#include<cstdio>
#include<sys/types.h>
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

using namespace std;
int main(int argc,char **argv){
    int sockfd;
    socklen_t len;
    struct sockaddr_in servaddr,tempaddr;
    fd_set allset,rset;
    char buffer[1000];
    char data[1000];
    char succeeded[]="succeeded\n";
    char failed[]="failed\n";
    int n;
    string str;
    //FD_SET CLEAR
    FD_ZERO(&rset);
    FD_ZERO(&allset);

    //SOCKET

    if(argc!=3){
        cout<<"argument error"<<endl;
        exit(0);
    }
    int port=atoi(argv[2]);
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0) perror("socket error from client");
    cout<<"socket success"<<endl;
    //SETSOCKET

    bzero((char*)&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    inet_pton(AF_INET,argv[1],&servaddr.sin_addr.s_addr);
    servaddr.sin_port=htons(port);

    //CONNECT
    if(connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
        perror("connect error");
    }

    cout<<"connect success"<<endl;
    FD_SET(0,&allset);
    FD_SET(sockfd,&allset);
    for(;;){
        rset=allset;
        bzero(data,1000);
        bzero(buffer,1000);
        if(select(4,&rset,NULL,NULL,NULL)<0){
            perror("select error from client");
        }
        if(FD_ISSET(0,&rset)){
            cin.getline(buffer,1000);
            string s(buffer);
            vector<string> v;
            stringstream stream;
            stream<<s;
            string input;
            while(stream>>input){
                v.push_back(input);
            }
            if(v.size()==3&&v[0]=="PUT"){
                if(write(sockfd,buffer,sizeof(buffer))<0) perror("write error");
                FILE* fp;
                fp=fopen(v[1].c_str(),"rb");
                if(fp==NULL){
                    char sourceerror[]="source error\n";
                    cout<<sourceerror;
                    if(write(sockfd,sourceerror,sizeof(sourceerror))<0) perror("write error");
                    continue;
                }
                char sourcesuccess[]="source success\n";
                //cout<<sourcesuccess;
                if(write(sockfd,sourcesuccess,sizeof(sourcesuccess))<0) perror("write error");
                char buffer1[256];
                bzero(buffer1,256);
                if(read(sockfd,&buffer1,256)<0) perror("read error");
                int tempport;
                tempport=atoi(buffer1);
                //cout<<"tempport"<<tempport<<endl;
                int tempfd;
                /////////////////create socket to transfer
                if((tempfd=socket(AF_INET,SOCK_STREAM,0))<0) perror("socket error from client");
                //cout<<"socket success"<<endl;
                //SETSOCKET
                bzero((char*)&tempaddr,sizeof(tempaddr));
                tempaddr.sin_family=AF_INET;
                inet_pton(AF_INET,argv[1],&tempaddr.sin_addr.s_addr);
                tempaddr.sin_port=htons(tempport);

                //CONNECT
                if(connect(tempfd,(struct sockaddr*)&tempaddr,sizeof(tempaddr))<0){
                    perror("connect error");
                }
                //cout<<"connect success"<<endl;
                char data1[256];
                bzero(data1,sizeof(data1));
				int n;
				while((n=read(fileno(fp),&data1,256))){
                    if(write(tempfd,data1,n)<0) perror("write error");
                }
                close(tempfd);
                //////////////
                fseek(fp,0,SEEK_END);
                long filelen=ftell(fp);
				char length[256];
				bzero(length,256);
                sprintf(length,"%ld\n",filelen);
           		fclose(fp);
           		if(write(sockfd,length,256)<0) perror("write error");
            }
            else if(v.size()==3&&v[0]=="GET"){
                if(write(sockfd,buffer,sizeof(buffer))<0) perror("write error");
                char buffer1[256];
                bzero(buffer1,256);
                if(read(sockfd,&buffer1,256)<0) perror("read error");
                char errorff[]="source error\n";
                if(!strcmp(errorff,buffer1)){
                    cout<<buffer1;
                    continue;
                }
                int tempport;
                tempport=atoi(buffer1);
                //cout<<"tempport"<<tempport<<endl;
                int tempfd;
                /////////////////create socket to transfer
                if((tempfd=socket(AF_INET,SOCK_STREAM,0))<0) perror("socket error from client");
                //cout<<"socket success"<<endl;
                //SETSOCKET
                bzero((char*)&tempaddr,sizeof(tempaddr));
                tempaddr.sin_family=AF_INET;
                inet_pton(AF_INET,argv[1],&tempaddr.sin_addr.s_addr);
                tempaddr.sin_port=htons(tempport);
                //CONNECT
                if(connect(tempfd,(struct sockaddr*)&tempaddr,sizeof(tempaddr))<0){
                    perror("connect error");
                }
                //cout<<"connect success"<<endl;
                char length[256];
                bzero(length,256);
                FILE* fp;
                fp=fopen(v[2].c_str(),"wb");
                if(fp==NULL){
                    perror("destination error");
                    exit(1);
                }
                char data1[256];
                bzero(data1,sizeof(data1));
                while((n=read(tempfd,&data1,256))){
                    //cout<<"n:"<<n<<endl;
                    if(write(fileno(fp),data1,n)<0) perror("write error");
                }
                fseek(fp,0,SEEK_END);
                long filelen=ftell(fp);
                //cout<<"filelen:"<<filelen<<endl;
                rewind(fp);
                close(tempfd);
                fclose(fp);
                if(read(sockfd,&length,256)<0) perror("read error");
                //cout<<"length:"<<length<<endl;
                long datalength=atol(length);
                if((filelen==datalength)&&datalength>0){
                    char successmsg[256];
                    sprintf(successmsg,"%s %s",buffer,succeeded);
                    cout<<successmsg;
                }
                else if((filelen!=datalength)&&datalength>0){
                    char errormsg[256];
                    sprintf(errormsg,"%s %s",buffer,failed);
                    cout<<errormsg;
                }
            }
			else if(v.size()==1&&v[0]=="EXIT"){
				break;	
			}
			else{
				if(write(sockfd,buffer,sizeof(buffer))<0) perror("write error");
			}
        }
        int num;
        if(FD_ISSET(3,&rset)){
            if((num=read(sockfd,&data,1000))<0){
                perror("read error from client");
            }
            else{
                cout<<data;
            }
        }
    }
    close(sockfd);
    return 0;

}
