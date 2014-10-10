/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the 
 *     GET method to serve static and dynamic content.
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp, int *cl);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void serve_head(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
void *thread_worker(void *arg);

int main(int argc, char **argv) 
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    /* Check command line args */
    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }
    port = atoi(argv[1]);

    listenfd = Open_listenfd(port);

    while (1) {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept

		Set_nonblocking(connfd);

		pthread_t thid;
		pthread_create(&thid, NULL, thread_worker, (void*)connfd);
		pthread_detach(thid);

//		doit(connfd);                                             //line:netp:tiny:doit
//		Close(connfd);                                            //line:netp:tiny:close
    }
}
/* $end tinymain */

/*
 * doit - handle one HTTP request/response transaction
 */
/* $begin doit */
void doit(int fd) 
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);                   //line:netp:doit:readrequest
	printf("%s\n", buf);
    sscanf(buf, "%s %s %s", method, uri, version);       //line:netp:doit:parserequest
    if (!strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
	    read_requesthdrs(&rio, NULL);                              //line:netp:doit:readrequesthdrs

	    /* Parse URI from GET request */
	    is_static = parse_uri(uri, filename, cgiargs);       //line:netp:doit:staticcheck
	    if (stat(filename, &sbuf) < 0) {                     //line:netp:doit:beginnotfound
			clienterror(fd, filename, "404", "Not found",
			    "Tiny couldn't find this file");
			return;
	    }                                                    //line:netp:doit:endnotfound

	    if (is_static) { /* Serve static content */
			if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) { //line:netp:doit:readable
			    clienterror(fd, filename, "403", "Forbidden",
					"Tiny couldn't read the file");
			    return;
			}
			serve_static(fd, filename, sbuf.st_size);        //line:netp:doit:servestatic
	    }
	    else { /* Serve dynamic content */
			if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) { //line:netp:doit:executable
			    clienterror(fd, filename, "403", "Forbidden",
					"Tiny couldn't run the CGI program");
			    return;
			}
			serve_dynamic(fd, filename, cgiargs);            //line:netp:doit:servedynamic
	    }
	} else if (!strcasecmp(method, "HEAD")) {
		parse_uri(uri, filename, cgiargs);
		if (stat(filename, &sbuf) < 0) {
			clienterror(fd, filename, "404", "Not found",
				"Tiny couln't find this file");
			return;
		}

		serve_head(fd, filename, sbuf.st_size);
	} else if (!strcasecmp(method, "POST")) {	
		int cl = 0;
	    read_requesthdrs(&rio, &cl);                              //line:netp:doit:readrequesthdrs

		is_static = parse_uri(uri, filename, cgiargs);
		if (!strstr(filename, "update.html")) {
			clienterror(fd, filename, "403", "Forbidden",
				"Tiny does not support operation");
		}

		serve_static2(fd, cl);
	} else {
		clienterror(fd, method, "501", "Not Implemented",
                "Tiny does not implement this method");
        return;
    }                                                    //line:netp:doit:endrequesterr
}
/* $end doit */

/*
 * read_requesthdrs - read and parse HTTP request headers
 */
/* $begin read_requesthdrs */
void read_requesthdrs(rio_t *rp, int *cl) 
{
    char buf[MAXLINE];

	while (1) {
		Rio_readlineb(rp, buf, MAXLINE);
		printf("%s", buf);
		if (cl) {
				char *p1, *p2;
				
				p1 = strstr(buf, "Content-Length:");
				if (p1) {
					p1 += strlen("Content-Length:");
					while (*p1 == ' ') 
						p1++;
					p2 = p1;
					while (*p2 >= '0' && *p2 <= '9')
						p2++;
					if (p2 - p1 > 0) {
						*p2 = '\x0';
						*cl = atoi(p1);
					}
				}
			}

		if (!strcmp(buf, "\r\n"))
			break;
	}

    return;
}
/* $end read_requesthdrs */

/*
 * parse_uri - parse URI into filename and CGI args
 *             return 0 if dynamic content, 1 if static
 */
/* $begin parse_uri */
int parse_uri(char *uri, char *filename, char *cgiargs) 
{
    char *ptr;

    if (!strstr(uri, "cgi-bin")) {  /* Static content */ //line:netp:parseuri:isstatic
		strcpy(cgiargs, "");                             //line:netp:parseuri:clearcgi
		strcpy(filename, ".");                           //line:netp:parseuri:beginconvert1
		strcat(filename, uri);                           //line:netp:parseuri:endconvert1
		if (uri[strlen(uri)-1] == '/')                   //line:netp:parseuri:slashcheck
		    strcat(filename, "home.html");               //line:netp:parseuri:appenddefault
		return 1;
    }
    else {  /* Dynamic content */                        //line:netp:parseuri:isdynamic
		ptr = index(uri, '?');                           //line:netp:parseuri:beginextract
		if (ptr) {
		    strcpy(cgiargs, ptr+1);
		    *ptr = '\0';
		}
		else 
		    strcpy(cgiargs, "");                         //line:netp:parseuri:endextract
		strcpy(filename, ".");                           //line:netp:parseuri:beginconvert2
		strcat(filename, uri);                           //line:netp:parseuri:endconvert2
		return 0;
    }
}
/* $end parse_uri */

/*
 * serve_static - copy a file back to the client 
 */
/* $begin serve_static */
void serve_static(int fd, char *filename, int filesize) 
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
 
    /* Send response headers to client */
    get_filetype(filename, filetype);       //line:netp:servestatic:getfiletype
    sprintf(buf, "HTTP/1.0 200 OK\r\n");    //line:netp:servestatic:beginserve
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_write(fd, buf, strlen(buf));       //line:netp:servestatic:endserve
	//printf(" %s\n", buf);

    /* Send response body to client */
    srcfd = Open(filename, O_RDONLY, 0);    //line:netp:servestatic:open
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);//line:netp:servestatic:mmap
    Close(srcfd);                           //line:netp:servestatic:close
    Rio_write(fd, srcp, filesize);         //line:netp:servestatic:write
    Munmap(srcp, filesize);                 //line:netp:servestatic:munmap
}

void serve_static2(int fd, int cl)
{
	ssize_t n, tor;
	char buf[MAXBUF];

	tor = cl > MAXBUF ? MAXBUF : cl;
	while (cl > 0 && (n = Rio_read(fd, buf, tor)) > 0) {
		printf("%s", buf);

		cl -= n;
		tor = cl > MAXBUF ? MAXBUF : cl;

		printf("\n\n\n cl = %d, tor = %d\n\n\n", cl, tor);
	}

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n"); 
    Rio_write(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_write(fd, buf, strlen(buf));

	Close(fd);
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype) 
{
    if (strstr(filename, ".html"))
		strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
    else if (strstr(filename, ".jpg"))
		strcpy(filetype, "image/jpeg");
	else if (strstr(filename, ".mpg"))
		strcpy(filetype, "video/mpeg");
	else if (strstr(filename, ".m3u8"))
		strcpy(filetype, "video/x-mpegurl");
	else if (strstr(filename, ".ts"))
		strcpy(filetype, "video/ts");
    else
		strcpy(filetype, "text/plain");
}  
/* $end serve_static */

/*
 * serve_dynamic - run a CGI program on behalf of the client
 */
/* $begin serve_dynamic */
void serve_dynamic(int fd, char *filename, char *cgiargs) 
{
    char buf[MAXLINE], *emptylist[] = { NULL };

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n"); 
    Rio_write(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_write(fd, buf, strlen(buf));
  
    if (Fork() == 0) { /* child */ //line:netp:servedynamic:fork
		/* Real server would set all CGI vars here */
		setenv("QUERY_STRING", cgiargs, 1); //line:netp:servedynamic:setenv
		Dup2(fd, STDOUT_FILENO);         /* Redirect stdout to client */ //line:netp:servedynamic:dup2
		Execve(filename, emptylist, environ); /* Run CGI program */ //line:netp:servedynamic:execve
    }
    Wait(NULL); /* Parent waits for and reaps child */ //line:netp:servedynamic:wait
}
/* $end serve_dynamic */

/*
 * serve_static - copy a file back to the client 
 */
/* $begin serve_static */
void serve_head(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);       //line:netp:servestatic:getfiletype
    sprintf(buf, "HTTP/1.0 200 OK\r\n");    //line:netp:servestatic:beginserve
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    sprintf(buf, "%sLast-modified: Tue,17Apr200106:46:28GMT\r\n", buf);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_write(fd, buf, strlen(buf));       //line:netp:servestatic:endserve
}

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_write(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_write(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_write(fd, buf, strlen(buf));
    Rio_write(fd, body, strlen(body));
}
/* $end clienterror */

void *thread_worker(void *arg)
{
	int connfd = (int)arg;

	doit(connfd);
	Close(connfd);

	return NULL;
}

