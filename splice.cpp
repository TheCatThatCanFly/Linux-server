#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>


/*
    sendfile在两个文件描述符之间直接传递数据，完全在内核中进行操作，从而避免了内核缓冲区和用户缓冲区之间的数据拷贝，
    称为零拷贝，效率很高。
    in_fd必须指向真实的文件，不能是套接字和管道

    splice用于在两个文件描述符之间传递数据，至少有一个管道

    tee函数用于在两个管道文件之间复制数据，不消耗数据
*/
void error_handling(char *message);

int main(int argc, char *argv[]) {
    if(argc <= 2) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t sz_clnt_addr;
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1) {
        error_handling("invalid serv_sock");
    }

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = argv[2];

    if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("bind error");
    }

    listen(serv_sock, 5);

    sz_clnt_addr = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &sz_clnt_addr);
    if(clnt_sock == -1) {
        error_handling("invalid clnt_sock");
    }

    int pipefd[2];
    pipe(pipefd);

    int ret = splice(clnt_sock, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE|SPLICE_F);
    ret = splice(pipefd[0], NULL, clnt_sock, NULL, 32768, SPLICE_F_MORE|SPLICE_F);
    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputs("\n", stderr);
    exit(1);
}