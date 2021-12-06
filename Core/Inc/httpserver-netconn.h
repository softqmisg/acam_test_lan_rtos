#ifndef __HTTPSERVER_NETCONN_H__
#define __HTTPSERVER_NETCONN_H__

#include "lwip/api.h"

void http_server_netconn_init(void);
void DynWebPage(struct netconn *conn);
 void http_server_netconn_thread(void *arg);
#endif /* __HTTPSERVER_NETCONN_H__ */
