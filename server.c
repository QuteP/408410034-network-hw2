#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include <unistd.h>
#include<string.h>
#include<time.h>
int fd,portno;
struct sockaddr_in srv;
struct sockaddr_in cli,srv_addr;
int newfd;
int lobby_switch=0;
unsigned int cli_len ;
int pos[2],win=0;
char poisition[9];

char user_list[3][10]={"Player1","Player2","Player3"};
char pass_list[3][10]={"12345","22345","32345"};
char* name;


int check_win(char cc[],char x)
{
    int i,j,k;
    if(cc[0]==x && cc[1]==x && cc[2]==x)
    {
        return(1);
    }
    else if(cc[3]==x && cc[4]==x && cc[5]==x)
    {
        return(1);
    }
    else if(cc[6]==x && cc[7]==x && cc[8]==x)
    {
        return(1);
    }
    else if(cc[0]==x && cc[3]==x && cc[6]==x)
    {
        return(1);
    }
    else if(cc[1]==x && cc[4]==x && cc[7]==x)
    {
        return(1);
    }
    else if(cc[2]==x && cc[5]==x && cc[8]==x)
    {
        return(1);
    }
    else if(cc[0]==x && cc[4]==x && cc[8]==x)
    {
        return(1);
    }
    else 
    {
        return(0);
    }

}

void bind_server()
{
    bzero((char *)&srv,sizeof(srv));

    srv.sin_family=AF_INET;
    srv.sin_port=htons(portno);
    // bind a client may connect to any of server addresses

    srv.sin_addr.s_addr=INADDR_ANY;
    if(bind(fd,(struct sockaddr*) &srv,sizeof(srv))<0)
    {
        perror("Bind出錯啦\n");
        exit(1);
    }


}

int max(int a,int b){
    if(a>b)return a;
    return b;
}

int check_agree(int sockfd,char mode){
    int			maxfdp1, stdineof=0;
	fd_set		rset;
	char		buf[2048];
	int		n;
    FILE* fp=stdin;
    //struct timeval tv;
    //tv.tv_sec=15;
    if(mode=='s'){
        read(sockfd,buf,32);
        printf("用戶 %s 想加入這個房間，請問你同意嗎？（y/n）\n",buf);
        memset(buf,0,2048);
        write(sockfd,buf,32);
    }
	FD_ZERO(&rset);
	while(1) {
		if (mode=='s' && stdineof == 0)
			FD_SET(fileno(fp), &rset);
		FD_SET(sockfd, &rset);
        if(mode=='s') maxfdp1 = max(fileno(fp), sockfd) + 1;
        else maxfdp1=sockfd+1;
		select(maxfdp1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(sockfd, &rset)) {	// socket is readable 
			if ( (n = read(sockfd, buf, 2047)) == 0) {
				if (stdineof == 1)
					return 0;		// normal termination 
				else
					perror("對方離線了\n");
                    return 0;
			}
            if(mode=='c'){
                if(buf[0]=='y' || buf[0]=='Y') return 1;
                else if(buf[0]=='n' || buf[0]=='N'){
                    close(sockfd);
                    return 0;
                }
            }
		}

		if (mode=='s' && FD_ISSET(fileno(fp), &rset)) {  // input is readable 
			if ( (n = read(fileno(fp), buf, 2047)) == 0) {
				stdineof = 1;
				shutdown(sockfd, SHUT_WR);	// send FIN 
				FD_CLR(fileno(fp), &rset);
				continue;
			}
            if(buf[0]=='y' || buf[0]=='Y'){
                write(sockfd,buf,32);
                return 1;
            }
            else if(buf[0]=='n' || buf[0]=='N'){
                write(sockfd,buf,32);
                close(sockfd);
                return 0;
		    }
            memset(buf,0,2048);
        }
	}
}

void DeleteRoom(){
    FILE* fp=fopen("round.txt","r");
    char temp_b[2048]={0};
    read(fileno(fp),temp_b,2047);
    char temp_port[8]={0};
    sprintf(temp_port,"%d",portno);
    char* keyword=strstr(temp_b,temp_port);
    char* first_tail=temp_b;
    while(strstr(first_tail,"\n")<keyword)
    {
        first_tail=strstr(first_tail,"\n");
        first_tail++;
    } 
    char* second_head=strstr(keyword,"\n")+1;
    char temp_c[2048]={0};
    strncpy(temp_c,temp_b,first_tail-temp_b);
    strcat(temp_c,second_head);
    fclose(fp);
    fp=fopen("round.txt","w+");
    fwrite(temp_c,1,strlen(temp_c),fp);
    fclose(fp);
    close(fd);
    
}

void accept_server()
{
    printf("輸入「exit」可離開房間喔\n");
    cli_len=sizeof(cli);
    int agree_switch=0;
    int			maxfdp1, stdineof=0;
	fd_set		rset;
	char		buf[2048]={0};
	int		n;
    FILE* fp=stdin;
    FD_ZERO(&rset);
    do{
        
        while(1){
            printf("房間號碼 : %d\n",portno);
            printf("其他人可以透過您的帳號或房號加入對戰 :\n");
            printf("正在等待其他人加入....\n");
            FD_SET(fileno(fp), &rset);
		    FD_SET(fd, &rset);
            maxfdp1 = max(fileno(fp), fd) + 1;
		    select(maxfdp1, &rset, NULL, NULL, NULL);
		    if (FD_ISSET(fd, &rset)) {	// socket is readable 
                newfd=accept(fd,(struct sockaddr*) &cli, &cli_len);
                break;
		    }
		    if (FD_ISSET(fileno(fp), &rset)) {  // input is readable 
			    if ( (n = read(fileno(fp), buf, 2047)) == 0) {
                    stdineof = 1;
                    shutdown(fd, SHUT_WR);	// send FIN 
                    FD_CLR(fileno(fp), &rset);
                    continue;
			    }
                if(strcmp("exit\n",buf)==0){
                    printf("你確定要離開房間嗎？（y/n）\n");
                    char temp[4]={0};
                    scanf("%s",temp);
                    if(temp[0]=='y' || temp[0]=='Y'){
                        DeleteRoom();//close寫裡面
                        lobby_switch=1;
                        return;
                    }
                    else{
                        system("clear");
                    }
                }
                memset(buf,0,2048);
            }

        }
        
        if((agree_switch=check_agree(newfd,'s'))==0){
            system("clear");
            printf("您拒絕了對方的請求\n");
        }
    }while(agree_switch==0);
    if(newfd<0)
    {
        perror("accept");
        exit(1);
    }
    else
    {
        system("clear");
        printf("連線成功！\n");
    }
    
    
}
int connect_client()
{
    bzero((char *)&srv,sizeof(srv));
    srv.sin_family=AF_INET;
    srv.sin_port=htons(portno);
    //bcopy((char *)server-> h_addr ,(char *)&srv.sin_addr.s_addr,server->h_length);
    
    if(connect(fd,(struct sockaddr*) &srv,sizeof(srv))<0)
    {
        perror("connect");
        return 0;
    }
    else{
        write(fd,name,strlen(name));
        if(check_agree(fd,'c')==0) return 0;
        system("clear");
        printf("對方同意了您的請求\n");
        return 1;
    }
}

void write_read(int choice)
{
    char cc[9];
    int temp_d=0;
    if(choice==1){
        int errno=0;
        for(int i=0;i<4;i++)
        {   
            
            recv(newfd,&pos,2*sizeof(int),0);
            recv(newfd,&win,sizeof(int),0);
            poisition[pos[0]-1]='X';
            do
            {
            system("clear");
            if(errno==1){
                printf("無效的選擇，請重新選擇\n");
            }
            printf(" %c | %c | %c \n",poisition[0],poisition[1],poisition[2]);
            printf("---------------\n");
            printf(" %c | %c | %c \n",poisition[3],poisition[4],poisition[5]);
            printf("---------------\n");
            printf(" %c | %c | %c \n",poisition[6],poisition[7],poisition[8]);
            printf("\n輪到對方了 : %d\n",pos[0]);
            if(win==1)
            {
                printf("對方贏了:\n");
                break;
            }
            printf("輪到你了:");
                
            scanf("%d",&temp_d);
            
            if(poisition[temp_d-1]!=' ')
            {   
                errno=1;
            }
            else{
                pos[0]=temp_d;
            }
            }while(poisition[temp_d-1]!=' ');
            system("clear");        
            poisition[pos[0]-1]='O';
            cc[pos[0]-1]='O';
            win=check_win(cc,'O');
            send(newfd,&pos,2*sizeof(int),0);
            send(newfd,&win,sizeof(int),0);
            printf(" %c | %c | %c \n",poisition[0],poisition[1],poisition[2]);
            printf("---------------\n");
            printf(" %c | %c | %c \n",poisition[3],poisition[4],poisition[5]);
            printf("---------------\n");
            printf(" %c | %c | %c \n",poisition[6],poisition[7],poisition[8]);
            if(win)
            {
                printf("你贏了\n");
                break;
            }
        
        }
    }
    else if(choice==2){
        int errno=0;
        for(int i=0;i<5;i++)
        {
            do
            {
                system("clear");
                if(errno==1){
                    printf("無效的選擇，請重新選擇\n");
                }
                printf(" %c | %c | %c \n",poisition[0],poisition[1],poisition[2]);
                printf("---------------\n");
                printf(" %c | %c | %c \n",poisition[3],poisition[4],poisition[5]);
                printf("---------------\n");
                printf(" %c | %c | %c \n",poisition[6],poisition[7],poisition[8]);
                printf("輪到你了:");
                scanf("%d",&temp_d);
                if(poisition[temp_d-1]!=' ')
                {   
                    errno=1;
                }
                else{
                    errno=0;
                    pos[0]=temp_d;
                }
            }while(poisition[temp_d-1]!=' ');
            
            poisition[pos[0]-1]='X';
            cc[pos[0]-1]='X';
            win=check_win(cc,'X');
            send(fd,&pos,2*sizeof(int),0);
            send(fd,&win,sizeof(int),0);
            system("clear");
            printf(" %c | %c | %c \n",poisition[0],poisition[1],poisition[2]);
            printf("---------------\n");
            printf(" %c | %c | %c \n",poisition[3],poisition[4],poisition[5]);
            printf("---------------\n");
            printf(" %c | %c | %c \n",poisition[6],poisition[7],poisition[8]);
            if(win)
            {
                printf("你贏了！\n");
                break;
            }
            if(i==4) break;
            recv(fd,&pos,2*sizeof(int),0);
            recv(fd,&win,sizeof(int),0);
            system("clear");
            poisition[pos[0]-1]='O';
            printf(" %c | %c | %c \n",poisition[0],poisition[1],poisition[2]);
            printf("---------------\n");
            printf(" %c | %c | %c \n",poisition[3],poisition[4],poisition[5]);
            printf("---------------\n");
            printf(" %c | %c | %c \n",poisition[6],poisition[7],poisition[8]);
            printf("\n輪到對方了 : %d\n",pos[0]);
            if(win)
            {
                printf("對方贏了:");
                break;
            }
        }
        
    }
    if(check_win(cc,'O')==0 && check_win(cc,'X')==0){
        printf("平手！\n");
    }
    getchar();
    printf("輸入任意鍵返回大廳：");
    getchar();
    lobby_switch=1;
    system("clear");
    if(choice==1){
        DeleteRoom();
    }
}

int check_login(char* name,char* pass){
    for(int i=0;i<3;i++) if(strcmp(name,user_list[i])==0 && strcmp(pass,pass_list[i])==0) return i+1;
    return 0;
}

void ShowList(){
    FILE* roundfile = fopen("round.txt", "r");
    fseek(roundfile, SEEK_SET, 0);
    char temp_c[32]={0};
    char temp_name[32]={0};
    char temp_room[8]={0};
    while(fgets(temp_c,32,roundfile)!=NULL){
        int name_len=strstr(temp_c,":")-temp_c;
        strncpy(temp_name,temp_c,name_len);
        strncpy(temp_room,strstr(temp_c,":")+1,4);
        printf("----------------\n");
        printf("房主：%s 房號：%s\n",temp_name,temp_room);
        memset(temp_c,0,32);
    }
    printf("----------------\n");
    fclose(roundfile);
    printf("請輸入欲連線的帳號或房號：");
}
void HandleInput(char* temp_input){
    FILE* roundfile = fopen("round.txt", "r");
    fseek(roundfile, SEEK_SET, 0);
    char temp_content[2048]={0};
    fread(temp_content,2047,1,roundfile);
    if(strstr(temp_content,temp_input)==NULL || strstr(temp_content,temp_input+2)[0]=='\n'){
        system("clear");
        printf("找不到您輸入的資訊，請重新輸入一次\n");
        ShowList();
    }
    else{
        char temp_port[8]={0};
        if((strstr(temp_content,temp_input)-1)[0]==':'){
            strncpy(temp_port,temp_input,4);
        }
        else{
            strncpy(temp_port,strstr(strstr(temp_content,temp_input),":")+1,4);
        }
        portno=atoi(temp_port);
    }
    
}

int main()
{
    char temp_name[10]={0};
    char temp_pass[10]={0};
    int err_flag=-1;
    system("clear");
    login: system("clear");
    do{
    if(err_flag==0){
    memset(temp_name,0,10);
    memset(temp_pass,0,10);
    system("clear");
    printf("輸入的帳號或密碼有錯誤！\n");}
    printf("登入:\n");
    printf("帳號 :");
    scanf("%s",temp_name);
    printf("密碼 :");
    scanf("%s",temp_pass);
    }while(!(err_flag=check_login(temp_name,temp_pass)));
    name=temp_name;
    system("clear");
    lobby: system("clear");
    printf("歡迎 %s！\n",name);
    printf("1. 開房間\n");
    printf("2. 加入對戰\n");
    printf("3. 登出遊戲\n");
    printf("請選擇 :");
    int choice,gamecode;
    scanf("%d",&choice);
    system("clear");
    for(int i=0;i<9;i++)
    {
       poisition[i]=' '; 
    }

    switch(choice)
    {
//要讓大家都看到房間資訊，就只能存在伺服器文件裡，然後伺服器把它讀出來。這樣指定玩家會比較方便。
        case 1:
            srand(time(0));
            gamecode = (rand()%(9999 - 5000 + 1)) + 5000;
            FILE* roundfile = fopen("round.txt", "a+");
            char filecontent[32]={0};
            char tempcode[8]={0};
            sprintf(tempcode,"%d",gamecode);
            strcat(filecontent,temp_name);
            strcat(filecontent,":");
            strcat(filecontent,tempcode);
            strcat(filecontent,"\n");
            //printf("%ld\n",strlen(filecontent)+1);
            fwrite(filecontent,1,strlen(filecontent),roundfile);
            fclose(roundfile);
            portno = gamecode;
            //working as a server 
            //system("clear");
            if((fd=socket(AF_INET,SOCK_STREAM,0))<0)
            {
                perror("socket出錯啦");
                exit(1);
            }
            bind_server();
            if(listen(fd,5)<0)
            {
                perror("Listen出錯啦");
                exit(1);
            }
            accept_server();
            if(lobby_switch==1){
                lobby_switch=0;
                goto lobby;
            }
            //system("clear");
            
            printf(" 1 | 2 | 3 \n");
            printf("---------------\n");
            printf(" 4 | 5 | 6 \n");
            printf("---------------\n");
            printf(" 7 | 8 | 9 \n");
            write_read(choice);
            if(lobby_switch==1){
                lobby_switch=0;
                goto lobby;
            }
        case 2:
            printf("輸入「exit」可回到上一頁\n");
            ShowList();
            int loop_switch=0;
            do{
            char temp_input[32]={0};
            scanf("%s",temp_input);
            if(strcmp("exit",temp_input)==0) goto lobby;
            else HandleInput(temp_input);
            if((fd=socket(AF_INET,SOCK_STREAM,0))<0)
            {
                perror("socket出錯啦");
                exit(1);
            }
            //system("clear");
            // working as a client
            printf("正在等待對方同意....\n");
            loop_switch=connect_client();
            if(loop_switch==0){
                system("clear");
                printf("對方拒絕了您的請求\n");
                ShowList();
            } 
            }while(loop_switch==0);
            
            //system("clear");
            printf(" 1 | 2 | 3 \n");
            printf("---------------\n");
            printf(" 4 | 5 | 6 \n");
            printf("---------------\n");
            printf(" 7 | 8 | 9 \n");
            write_read(choice);
            if(lobby_switch==1){
                lobby_switch=0;
                goto lobby;
            }
        case 3:
            printf("你確定要登出遊戲嗎？（y/n）\n");
            char temp[]={0};
            scanf("%s",temp);
            if(temp[0]=='y' || temp[0]=='Y'){
                goto login;
            }
            else if(temp[0]=='n' || temp[0]=='N'){
                goto lobby;
            }
    }
}

