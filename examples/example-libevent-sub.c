/*
 * @Author: Ron
 * @LastEditors: Ron
 * @FilePath: \hiredis\examples\example-libevent-sub.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <hiredis.h>
#include <async.h>
#include <adapters/libevent.h>

#define CMD_SIZE 128
 
void subCallback(redisAsyncContext *c, void *r, void *priv) {
    redisReply *reply = r;
    if (reply == NULL) return;
    if ( reply->type == REDIS_REPLY_ARRAY && reply->elements == 3 ) {
        if ( strcmp( reply->element[0]->str, "subscribe" ) != 0 ) {
            printf( "Received[%s] channel %s: %s\n",
                    (char*)priv,
                    reply->element[1]->str,
                    reply->element[2]->str );
        }
    }
}
 
void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");
}
 
void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Disconnected...\n");
}
 
int main (int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);

    if (argc <= 1)
    {
        printf("usage: proc key1 key2 ...\n");
        return 1;
    }

    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
    if (c->err) {
        /* Let *c leak for now... */
        printf("Error: %s\n", c->errstr);
        return 1;
    }

    printf("main Connect...\n");
    struct event_base *base = event_base_new();
    redisLibeventAttach(c,base);
    redisAsyncSetConnectCallback(c,connectCallback);
    redisAsyncSetDisconnectCallback(c,disconnectCallback);

    char cmd[CMD_SIZE] = {0};
    int count = argc;
    while(count-->1)
    {
        snprintf(cmd, CMD_SIZE, "SUBSCRIBE %s", argv[argc-count]);
        redisAsyncCommand(c, subCallback, (char *)"sub", cmd);
        printf("%s\n", cmd);
    }
 
    event_base_dispatch(base);
    printf("HELLO WORLD\n");
    return 0;
}