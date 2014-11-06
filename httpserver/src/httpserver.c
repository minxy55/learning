/////
// Project: TCS
////
// File: httpserver.c
/////

#define HTTP_GET  0
#define HTTP_POST 1

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <poll.h>
#include <sys/stat.h>

#include "http.h"

#include "debug.h"
#include "tools.h"
#include "threads.h"
#include "sockets.h"

#include "httpserver.h"
#include "httpbuffer.c"
#include "dyn_buffer.c"

#include "images.c"

#define config_file "tcs.conf"


char *src2string(int srctype, int srcid, char *prestr, char *ret);

typedef struct
{
	char name[256];
	char value[512];
} http_get;

typedef struct 
{
	struct dyn_buffer dbf;
	int type;//= (HTTP_GET/HTTP_POST)
	char path[512];
	char file[512];
	int http_version;//(0:1.0,1:1.1)
	char Host[100];//(localhost:9999)
	int Connection;//(1:keep-alive, 0:close);
	http_get getlist[20];
	int getcount;
	http_get postlist[20];
	int postcount;
	http_get headers[20];
	int hdrcount;

} http_request;

void buf2str( char *dest, char *start, char *end)
{
  while (*start==' ') start++;
  while (start<=end)
  {
	*dest=*start;
	start++;
	dest++;
  }
  *dest='\0';
}

///////////////////////////////////////////////////////////////////////////////
char *isset_get(http_request *req, char *name)
{
  int i;
  char *n,*v;
  for(i=0; i<req->getcount; i++) {
    n = req->getlist[i].name;
    v = req->getlist[i].value;
    if (!strcmp(name, n)) {
		//printf("[$_GET] Name: '%s'    Value :'%s'\n", n,v);
		return v;
	}
  }
  return NULL;
}


///////////////////////////////////////////////////////////////////////////////
char *isset_header(http_request *req, char *name)
{
  int i;
  char *n,*v;
  //printf("Searching '%s'\n", name);
  for(i=0; i<req->hdrcount; i++) {
	//printf("[HEADER] Name: '%s'    Value :'%s'\n", req->headers[i].name, req->headers[i].value);
    n = req->headers[i].name;
    v = req->headers[i].value;
    if (!strcmp(name, n)) return v;
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
void explode_get(http_request *req, char *get) // Get Variables
{
  char *end,*a;
  int i;
  i=0;
  //debugf("explode_get()\n");
  while ( (end=strchr(get, '&')) ) 
  {
	*end = '\0';
        if (i>8) break; //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	if ( (a=strchr(get, '=')) ) {
	  *a='\0';
	  strncpy(req->getlist[i].name,get,255); 
	  strncpy(req->getlist[i].value,a+1,255);
	  //printf("$_GET['%s'] = '%s'\n",req->getlist[i].name,req->getlist[i].value);
	  i++;
	}
	get=end+1;
  }
  if ( (a=strchr(get, '=')) ) {
    *a='\0';
    strncpy(req->getlist[i].name,get,255);
    strncpy(req->getlist[i].value,a+1,255);
    //printf("$_GET['%s'] = '%s'\n",req->getlist[i].name,req->getlist[i].value);
    i++;
  }
  req->getcount=i;
}

///////////////////////////////////////////////////////////////////////////////
void explode_post(http_request *req, char *post)
{
  char *end,*a;
  int i;
  i=0;
  while ( (end=strchr(post, '&')) ) 
  {
	*end = '\0';
	if ( (a=strchr(post, '=')) ) {
	  *a='\0';
	  strncpy(req->postlist[i].name,post,255); 
	  strncpy(req->postlist[i].value,a+1,255);
	  //printf("$_POST['%s'] = '%s'\n",req->postlist[i].name,req->postlist[i].value);
	  i++;
	}
	post=end+1;
  }
  if ( (a=strchr(post, '=')) ) {
    *a='\0';
    strncpy(req->postlist[i].name,post,255);
    strncpy(req->postlist[i].value,a+1,255);
    i++;
  }
  req->postcount=i;
}

//////////////////////////////////////////////////////////////////////////////

int extractreq(http_request *req, char *buffer, int len )
{
	char *path_start, *path_end;
	char *rnrn, *slash;

	//printf("buffer size %d\n",len);
	//#Check Header
	if (!(rnrn=strstr( buffer, "\r\n\r\n"))) return -1;
	int reqsize = (rnrn-buffer)+4;
	//#Get Path
	path_start = buffer+4;
	while (*path_start==' ') path_start++;
	path_end=path_start;
	while (*path_end!=' ') path_end++;
	buf2str( req->path, path_start, path_end-1);
	//printf("Path = '%s'\n", req->path);
	//#extract filename, and path
	slash = path_start = (char*)&req->path;
	while (*path_start) {
		if (*path_start=='/') slash=path_start;
		else if (*path_start=='?') {
			explode_get(req,path_start+1);
			*path_start='\0';
			break;
		}
		path_start++;
	}
	slash++;
	strncpy( req->file, slash, 100);
	//#Extract headers
	path_start = buffer+4;
	while ( (*path_start!='\r')&&(*path_start!='\n') ) path_start++;
	if (*path_start=='\r') path_start++;
	if (*path_start=='\n') path_start++;
	while ( path_start<rnrn ) {
		// start = path_start
		//get end of line
		path_end = path_start;
		slash = NULL;
		while ( (*path_end!='\r')&&(*path_end!='\n')&&(*path_end!=0) ) {
			if (*path_end==':') if (!slash) slash = path_end;
			path_end++;
		}
		if (path_end==path_start) break; // end
		char tmp = *path_end;
		*path_end = 0;
		if (slash) {
			// Extract header name: value
			buf2str( req->headers[req->hdrcount].name , path_start, slash-1);
			buf2str( req->headers[req->hdrcount].value, slash+1, path_end-1);
			//printf(">> %s\n", path_start);
			//printf("[HEADER] Name: '%s'    Value :'%s'\n", req->headers[req->hdrcount].name, req->headers[req->hdrcount].value);
			//if ( !strcmp(req->headers[req->hdrcount].name,"Authorization") ) 
			req->hdrcount++;
		}
		*path_end = tmp;
		path_start = path_end;
		while ( (*path_start=='\r')||(*path_start=='\n') ) path_start++;
	}

	if ( !memcmp(buffer,"GET ",4) ) {
		//printf("requesttype = GET\n");
		req->type = HTTP_GET;
	}
	else if ( !memcmp(buffer,"POST",4) ) {
		//printf("requesttype = POST\n");
		req->type = HTTP_POST;
		int i;
		for(i=0; i<req->hdrcount; i++) {
			if ( !strcmp(req->headers[i].name,"Content-Length") ) {
				reqsize += atoi(req->headers[i].value);
				//printf("req size %d\n", reqsize);
				return reqsize;
			}
		}
	}
	return 0;
}




int parse_http_request(int sock, http_request *req )
{
	unsigned char buffer[2048]; // HTTP Header cant be greater than 1k
	int size;
	int totalsize = 0;
	memset(buffer,0,sizeof(buffer));
	memset(req,0, sizeof(http_request));
	size = recv( sock, buffer, sizeof(buffer), MSG_NOSIGNAL);
	if (size<10) return 0;
	totalsize += size;
	//printf("** Receiving %d bytes\n%s\n",size,buffer );
	if ( !memcmp(buffer,"GET ",4) || !memcmp(buffer,"POST",4) ) {
		buffer[size] = '\0';

		// Get Header
		while ( !strstr((char*)buffer, "\r\n\r\n") ) {
			struct pollfd pfd;
			pfd.fd = sock;
			pfd.events = POLLIN | POLLPRI;
			int retval = poll(&pfd, 1, 5000);
			if ( retval>0 )	{
				if ( pfd.revents & (POLLHUP|POLLNVAL) ) return 0; // Disconnect
				else if ( pfd.revents & (POLLIN|POLLPRI) ) {
					int len = recv(sock, (buffer+size), sizeof(buffer)-size, MSG_NOSIGNAL);
					//printf("** Receiving %d bytes\n",len );
					if (len<=0) return 0;
					size+=len;
					buffer[size]=0;
					totalsize += len;
				}
			}
			else if (retval==0) break;
			else return 0;
		}
		// Received Header
		//debugf(" Received Header >>>\n%s\n<<<\n", buffer);
		int ret = extractreq(req,(char*)buffer,size);
		if (ret==-1) return 0;
		//Get Data
		if (req->type==HTTP_POST) {
			while (ret>totalsize) {
				//printf("Waiting....\n");
				struct pollfd pfd;
				pfd.fd = sock;
				pfd.events = POLLIN | POLLPRI;
				int retval = poll(&pfd, 1, 5000);
				if ( retval>0 )	{
					if ( pfd.revents & (POLLHUP|POLLNVAL) ) return 0; // Disconnect
					else if ( pfd.revents & (POLLIN|POLLPRI) ) {
						if (size>=sizeof(buffer)) {
							dynbuf_write( &req->dbf, buffer, size);
							size = 0;
						}
						int len = recv(sock, (buffer+size), sizeof(buffer)-size, MSG_NOSIGNAL);
						//printf("** Receiving %d bytes\n",len );
						if (len<=0) return 0;
						size+=len;
						totalsize += len;
					}
				}
				else return 0;
			}
			if (size) dynbuf_write( &req->dbf, buffer, size);
		}
	}
	else return 0;
	return 1;
}


int total_profiles()
{
	return 0;
}	

int total_servers()
{
	return 0;
}

int connected_servers()
{
	return 0;
}


int totalcachepeers()
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//<LINK href=\"./style.css\" rel=\"stylesheet\" type=\"text/css\">


//color: #000000; background-color: #FFFFFF;
char http_replyok[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";

char http_html[] = "<HTML>\n";
char http_html_[] = "</HTML>\n";

char http_head[] = "<HEAD>\n";
char http_head_[] = "</HEAD>\n";

char http_body[] = "<BODY>\n";
char http_body_[] = "</BODY>\n";

char http_title[] = "<title>TCS - %s</title>\n";

char http_link[] = "<link rel=\"icon\" type=\"image/png\" href=\"/favicon.png\">\n";

char http_style[] = "<STYLE TYPE=\"text/css\" MEDIA=screen><!--\n\
body {font-family: Tahoma; font-size: 14; }\
a { font-family: Tahoma; font-size: 12; }\
a:link { text-decoration: none; color: #3300FF; }\
a:visited { text-decoration: none; color: #3300FF; }\
a:hover { text-decoration: none; color: #FF0000; }\
a:active { text-decoration: none; color: #3300FF; }\
td { border-width: 0px; font-family: Tahoma; font-size: 11; padding: 2px 7px 2px 7px; }\
.header { font-family: Arial, sans-serif; font-size: 12; font-weight: bold; background-color: #441144; color: #ffffff; text-align: center; }\
.busy { background-color: #99bb00; text-align: center; }\
.online { background-color: #66F822; text-align: center; }\
.offline { background-color: #EE3333; text-align: center; }\
.left { text-align: left; }\
.center { text-align: center; }\
.right { text-align: right; }\
.alt1 { background-color: #FFFFFF; }\
.alt2 { background-color: #FAF8FA; }\
.alt3 { background-color: #E4EAE4; }\
.small { font-size: 10; }\
.infomenu { border-width: 0px; background-color: whitesmoke; font-family: Tahoma; font-size: 14; }\
.infomenu td { border-width: 0px; font-family: Tahoma; font-size: 14; padding: 0px 0px; }\
.button { float:left; height:auto; font: 11px Lucida Grande, Geneva, Verdana, Arial, Helvetica, sans-serif; width:100px; text-align:center; white-space:nowrap; }\
.button a:link, .button a:visited {	background-color: #e8e8e8;color: #666; font-size: 11px; font-weight:bolder; text-decoration: none; border:1px solid #000; margin: 0.2em; padding:0.2em; display:block; }\
.button a:hover { background-color: #f8f8f8; color:#555; border-top:0.1em solid #777; border-left:0.1em solid #777; border-bottom:0.1em solid #aaa; border-right:0.1em solid #aaa; padding:0.2em; margin: 0.2em; }\
div.outer { position: relative; width: 100%; } div.right { width: 100%; margin-left: 50%; } div.top,div.bottom { position: absolute; width: 50%; } div.top { top: 0; } div.bottom { bottom: 0; }\
table.option th, table.option td { font-family : Arial,sans-serif; font-size: 10; background : #efe none; color : #630; }\
\n\
.yellow { font: 11px Verdana, Arial, Helvetica, sans-serif; border-collapse: collapse; }\
.yellow th { padding: 3px; text-align: left; border-top: 1px solid #FB7A31; border-bottom: 1px solid #FB7A31; background: #FFC; }\
.yellow tr:hover { background-color: #ebf2fb; }\
.yellow td { border-bottom: 1px solid #CCC; padding: 3px; }\
.yellow td+td, .yellow th+th { border-left: 1px solid #CCC; }\
.yellow .alt1 { background-color: #FFFFFF; }\
.yellow .alt2 { background-color: #FAF8FA; }\
.yellow .alt3 { background-color: #E4EAE4; }\
.yellow .success { color: #347C17; }\
.yellow .failed { color: #C11B17; }\
\n\
textarea { border:1px solid #999999; width:100%; height:100%; margin:5px 0; padding:3px; }\
\n--></STYLE>\n";

char http_menu[] = "<span class='button'><a href='/'>Home</a></span>"
"<span class='button'><a href='/profiles'>Profiles</a></span>"
"<span class='button'><a href='/editor'>Editor</a></span>"
"<span class='button'><a href='/upg'>Update</a></span>"
"<span class='button'><a href='/restart'>Restart</a></span>"
"<span style='float:right'><b>TCS</b></span>"
"<br><br>\n";


static int check_progress = 0;
static int update_progress = 0;

static void dump_http_request(http_request *req)
{
	int j;
	
	printf("req.path: %s\n", req->path);
	printf("req.file: %s\n", req->file);
	for (j = 0; j < req->getcount; j++) {
		printf("get %d: %s -> %s\n", j, req->getlist[j].name, req->getlist[j].value);
	}
	for (j = 0; j < req->postcount; j++) {
		printf("post %d: %s -> %s\n", j, req->postlist[j].name, req->postlist[j].value);
	}
	for (j = 0; j < req->hdrcount; j++) {
		printf("header %d: %s -> %s\n", j, req->headers[j].name, req->headers[j].value);
	}
}

void http_send_updating(int sock, http_request *req)
{
	struct tcp_buffer_data tcpbuf;
	unsigned char http_buf[1024];
	unsigned char progress_buf[8];

	snprintf(progress_buf, sizeof(progress_buf), "%d", update_progress);

	tcp_init(&tcpbuf);
	snprintf(http_buf, sizeof(http_buf), 
		"HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-type: text/plain\r\n\r\n%s", 
		strlen(progress_buf), progress_buf);
	tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf));
	tcp_flush(&tcpbuf, sock);

	if (update_progress < 100)
		update_progress++;
}


void http_send_checking(int sock, http_request *req)
{
	struct tcp_buffer_data tcpbuf;
	unsigned char http_buf[1024];
	unsigned char progress_buf[8];

	snprintf(progress_buf, sizeof(progress_buf), "%d", check_progress);

	tcp_init(&tcpbuf);
	snprintf(http_buf, sizeof(http_buf), 
		"HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-type: text/plain\r\n\r\n%s", 
		strlen(progress_buf), progress_buf);
	tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf));
	tcp_flush(&tcpbuf, sock);

	if (check_progress < 100)
		check_progress++;
}

void http_send_html(int sock, http_request *req, char *filename)
{
	char http_buf[1024];
	struct tcp_buffer_data tcpbuf;
	struct stat sb;
	int fp = -1, n = 0;

	check_progress = 0;
	update_progress = 0;

	tcp_init(&tcpbuf);

	fp = open(filename, O_RDONLY);
	if (fp > 0) {
		stat(filename, &sb);

		snprintf(http_buf, sizeof(http_buf),
			"HTTP/1.1 200 OK\r\n"
			"Content-Length: %ld\r\n"
			"Content-Type: text/html\r\n\r\n", 
			sb.st_size);
		tcp_write(&tcpbuf, sock, (char*)http_buf, strlen(http_buf));
		//printf("http_buf = %s\n", http_buf);

		while ((n = read(fp, http_buf, sizeof(http_buf))) > 0) {
			//printf("http_buf = %s\n", http_buf);
			tcp_write(&tcpbuf, sock, (char*)http_buf, n);
			memset(http_buf, 0, sizeof(http_buf));
		}
		close(fp);
	} else {
		snprintf(http_buf, sizeof(http_buf), "HTTP/1.1 404 Not found\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<HTML><HEAD><TITLE>Multi Cardserver</TITLE></HEAD><BODY><H2>No page required</H2></BODY></HTML>");
		tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf));
	}

	tcp_flush(&tcpbuf, sock);
}

void http_send_image(int sock, http_request *req, unsigned char *buf, int size, char *type)
{
	char http_buf[1024];
	struct tcp_buffer_data tcpbuf;
	tcp_init(&tcpbuf);
	sprintf( http_buf, "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nCache-Control: private, max-age=86400\r\nContent-Length: %d\r\nConnection: close\r\nContent-Type: image/%s\r\n\r\n", size, type);
	tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
	tcp_write(&tcpbuf, sock, (char*)buf, size );
	tcp_flush(&tcpbuf, sock);
}


void http_send_xml(int sock, http_request *req, char *buf, int size)
{
	char http_buf[1024];
	struct tcp_buffer_data tcpbuf;
	tcp_init(&tcpbuf);
//	sprintf( http_buf, "HTTP/1.1 200 OK\r\nDate: Tue, 03 Apr 2012 08:10:57 GMT\r\nServer: Apache/2.2.14 (Ubuntu)\r\nLast-Modified: Tue, 03 Apr 2012 08:08:36 GMT\r\nETag: \"1c08f-b7-4bcc1d0490900\"\r\nAccept-Ranges: bytes\r\nContent-Length: %d\r\nKeep-Alive: timeout=15, max=100\r\nConnection: Keep-Alive\r\nContent-Type: application/xml\r\n\r\n" , size );
	sprintf( http_buf, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nAccept-Ranges: bytes\r\nConnection: close\r\nContent-Type: application/xml\r\n\r\n", size);
	tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
	tcp_write(&tcpbuf, sock, (char*)buf, size );
	tcp_flush(&tcpbuf, sock);
}

void http_get_bin(int sock, http_request *req, char *filename)
{	
	unsigned char http_buf[2048];
	struct tcp_buffer_data tcpbuf;

	unsigned char *c1 = req->dbf.data;
	unsigned char *c2 = strstr(c1, "\r\n\r\n");
	if (c2) {
		int fp = -1;
		
		c2 += 4;

		fp = open(filename, O_RDWR | O_CREAT);

		if (fp > 0) {
			write(fp, c2, req->dbf.datasize - (c2 - c1));
			close(fp);
		}
	}

	tcp_init(&tcpbuf);
	snprintf(http_buf, sizeof(http_buf), "HTTP/1.1 200 OK\r\n\r\n");
	tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf));
	tcp_flush(&tcpbuf, sock);
}
/*

char file[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n<html>\n\n  <head>\n    <script type=\"text/javascript\" language=\"javascript\">\n\n      //=====================================================\n\n      function makeHttpRequest(url, callFunction, xml)\n      {\n\n        //===============================\n        // Define http_request\n        \n        var httpRequest;\n        try\n        {\n          httpRequest = new XMLHttpRequest();  // Mozilla, Safari, etc\n        }\n        catch(trymicrosoft)\n        {\n          try\n          {\n            httpRequest = new ActiveXObject(\"Msxml2.XMLHTTP\");\n          }\n          catch(oldermicrosoft)\n          {\n            try\n            {\n              httpRequest = new ActiveXObject(\"Microsoft.XMLHTTP\");\n            }\n            catch(failed)\n            {\n              httpRequest = false;\n            }\n          }\n        }\n        if(!httpRequest)\n        {\n          alert('Your browser does not support Ajax.');\n          return false;\n        }\n\n        //===============================\n        // Action http_request\n\n        httpRequest.onreadystatechange = function()\n        {\n          if(httpRequest.readyState == 4)\n            if(httpRequest.status == 200)\n            {\n              if(xml)\n                eval(callFunction+'(httpRequest.responseXML)');\n              else\n                eval(callFunction+'(httpRequest.responseText)');\n            }\n            else\n              alert('Request Error: '+httpRequest.status);\n        }\n        httpRequest.open('GET',url,true);\n        httpRequest.send(null);\n      \n      }\n\n      //=====================================================\n\n      function buildTable2D(tabName,xmlDoc)\n      {\n        var htmDiv = document.getElementById('div'+tabName);\n        htmDiv.innerHTML = '<table border=1 id=\"'+tabName+'\"></table>';\n        var htmTab = document.getElementById(tabName);\n        var xmlTag = xmlDoc.getElementsByTagName(xmlDoc.documentElement.tagName).item(0);\n        for (var row=0; row < xmlTag.childNodes.length; row++)\n        {\n          var xmlRow = xmlTag.childNodes.item(row);\n          if(xmlRow.childNodes.length > 0)\n          {\n            var htmRow = htmTab.insertRow(parseInt(row/2));\n            for (var col=0; col < xmlRow.childNodes.length; col++)\n            {\n              var xmlCell = xmlRow.childNodes.item(col);\n              if(xmlCell.childNodes.length > 0)\n              {\n                var htmCell = htmRow.insertCell(parseInt(col/2));\n                htmCell.innerHTML = xmlCell.childNodes.item(0).data;\n              }\n            }\n          }\n        }\n      }\n\n      //=====================================================\n\n      function buildTable(xmlDoc)\n      {\n        buildTable2D('xmltable',xmlDoc);\n      }\n\n      //=====================================================\n\n    </script>\n  </head>\n\n  <body>\n    <input type=\"button\" value=\"My First Ajax XML\" onclick=\"makeHttpRequest('MyFirstAjaxXML.xml','buildTable',true)\"/>\n    <div id=\"divxmltable\">\n    </div>\n  </body>\n  \n</html>\n";

void http_send_file(int sock, http_request *req)
{
	tcp_init(&tcpbuf);
	tcp_write(&tcpbuf, sock, file, strlen(file) );
	tcp_flush(&tcpbuf, sock);
}

*/

void http_send_index(int sock, http_request *req)
{
	char http_buf[1024];
	struct tcp_buffer_data tcpbuf;

	tcp_init(&tcpbuf);
	tcp_write(&tcpbuf, sock, http_replyok, strlen(http_replyok) );
	tcp_write(&tcpbuf, sock, http_html, strlen(http_html) );
	tcp_write(&tcpbuf, sock, http_head, strlen(http_head) );
	sprintf(http_buf, http_title, "Home"); tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
	tcp_write(&tcpbuf, sock, http_link, strlen(http_link) );
	tcp_write(&tcpbuf, sock, http_style, strlen(http_style) );
	tcp_write(&tcpbuf, sock, http_head_, strlen(http_head_) );
	tcp_write(&tcpbuf, sock, http_body, strlen(http_body) );
	tcp_write(&tcpbuf, sock, http_menu, strlen(http_menu) );

	unsigned int d= GetTickCount()/1000;
	sprintf( http_buf,"<br>Uptime: %02dd %02d:%02d:%02d", d/(3600*24), (d/3600)%24, (d/60)%60, d%60); tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
	sprintf( http_buf, "<br>Total Profiles: %d", total_profiles() ); tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
	sprintf( http_buf, "<br>Connected Servers: %d / %d", connected_servers(), total_servers() ); tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
//	sprintf( http_buf, "<br>Total Cache Peers: %d", totalcachepeers() ); tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
	sprintf( http_buf, "<br><pre style=\"font-size:10; color:#004455;\">"); tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
	int i=idbgline;
	do {
		sprintf( http_buf, "%s", dbgline[i] ); tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
		i++;
		if (i>=MAX_DBGLINES) i=0;
	} while (i!=idbgline);
	sprintf( http_buf, "</pre>"); tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );

	tcp_write(&tcpbuf, sock, http_body_, strlen(http_body_) );
	tcp_write(&tcpbuf, sock, http_html_, strlen(http_html_) );
	tcp_flush(&tcpbuf, sock);
}

void http_send_restart(int sock, http_request *req)
{
	char http_buf[1024];
	struct tcp_buffer_data tcpbuf;
	tcp_init(&tcpbuf);
	tcp_write(&tcpbuf, sock, http_replyok, strlen(http_replyok) );
	tcp_write(&tcpbuf, sock, http_html, strlen(http_html) );
	tcp_write(&tcpbuf, sock, http_head, strlen(http_head) );
	sprintf( http_buf, http_title, "Restarting"); tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
	tcp_write(&tcpbuf, sock, http_link, strlen(http_link) );
	tcp_write(&tcpbuf, sock, http_style, strlen(http_style) );
	tcp_write(&tcpbuf, sock, http_head_, strlen(http_head_) );
	tcp_write(&tcpbuf, sock, http_body, strlen(http_body) );
	tcp_write(&tcpbuf, sock, http_menu, strlen(http_menu) );

	tcp_writestr(&tcpbuf, sock, "\n<script type=\"text/JavaScript\"><!--\nsetTimeout(\"location.href = '/';\",5000);\n--></script>\n<h3>Restarting TCS<br>Plesase Wait...</h3>");

	tcp_flush(&tcpbuf, sock);
	prg.restart = 1;
}

char *encryptxml( char *src)
{
	char exml[1024];
	char *dest = exml;
	while (*src) {
		switch (*src) {
			case '<':
				memcpy(dest,"&lt;", 4);
				dest +=4;
				break;				
			default:
				*dest = *src;
				dest++;
		}
		src++;
	}
	*dest = 0;
	strcpy( src, exml);
	return src;
}

void http_send_profiles(int sock, http_request *req)
{
	char http_buf[1024];
	struct tcp_buffer_data tcpbuf;

	char cell[11][512];

	int i;
	char *id = isset_get( req, "id");
	// Get Peer ID
	if (id)	{
		memset(cell, 0, sizeof(cell));
		i = atoi(id);
		//look for server
		printf("id = %d\n", i);
		/*
		struct cardserver_data *cs = cfg.cardserver;
		while (cs) {
			if (cs->id==(uint32)i) break;
			cs = cs->next;
		}
		if (!cs) return;
		// Send XML CELLS
		getprofilecells(cs,cell);
		*/
		char buf[5000] = "";
		sprintf( buf, "<profile>\n<c0>%s</c0>\n<c1_c>%s</c1_c>\n<c1>%s</c1>\n<c2>%s</c2>\n<c3>%s</c3>\n<c4>%s</c4>\n<c5>%s</c5>\n<c6>%s</c6>\n<c7>%s</c7>\n<c8>%s</c8>\n<c9>%s</c9>\n</profile>\n",encryptxml(cell[0]),encryptxml(cell[10]),encryptxml(cell[1]),encryptxml(cell[2]),encryptxml(cell[3]),encryptxml(cell[4]),encryptxml(cell[5]),encryptxml(cell[6]),encryptxml(cell[7]),encryptxml(cell[8]),encryptxml(cell[9]) );
		http_send_xml( sock, req, buf, strlen(buf));
		return;
	}

	tcp_init(&tcpbuf);
	tcp_write(&tcpbuf, sock, http_replyok, strlen(http_replyok) );
	tcp_write(&tcpbuf, sock, http_html, strlen(http_html) );
	tcp_write(&tcpbuf, sock, http_head, strlen(http_head) );
	sprintf( http_buf, http_title, "Profiles"); tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
	tcp_write(&tcpbuf, sock, http_link, strlen(http_link) );
	tcp_write(&tcpbuf, sock, http_style, strlen(http_style) );
	tcp_writestr(&tcpbuf, sock, "\n<script type='text/javascript'>\n\nvar requestError = 0;\n\nfunction makeHttpRequest( url, xml, callFunction, param )\n{\n	var httpRequest;\n	try {\n		httpRequest = new XMLHttpRequest();  // Mozilla, Safari, etc\n	}\n	catch(trymicrosoft) {\n		try {\n			httpRequest = new ActiveXObject('Msxml2.XMLHTTP');\n		}\n		catch(oldermicrosoft) {\n			try {\n				httpRequest = new ActiveXObject('Microsoft.XMLHTTP');\n			}\n			catch(failed) {\n				httpRequest = false;\n			}\n		}\n	}\n	if (!httpRequest) {\n		alert('Your browser does not support Ajax.');\n		return false;\n	}\n	// Action http_request\n	httpRequest.onreadystatechange = function()\n	{\n		if (httpRequest.readyState == 4) {\n			if(httpRequest.status == 200) {\n				if(xml) {\n					eval( callFunction+'(httpRequest.responseXML,param)');\n				}\n				else {\n					eval( callFunction+'(httpRequest.responseText,param)');\n				}\n			}\n			else {\n				requestError = 1;\n			}\n		}\n	}\n	httpRequest.open('GET',url,true);\n	httpRequest.send(null);\n}\n\nvar currentid=0;\n\nfunction xmlupdateprofile( xmlDoc, id )\n{\n	var row = document.getElementById(id);\n	row.cells.item(0).innerHTML = xmlDoc.getElementsByTagName('c0')[0].childNodes[0].nodeValue;\n	row.cells.item(1).className = xmlDoc.getElementsByTagName('c1_c')[0].childNodes[0].nodeValue;\n	row.cells.item(1).innerHTML = xmlDoc.getElementsByTagName('c1')[0].childNodes[0].nodeValue;\n	row.cells.item(2).innerHTML = xmlDoc.getElementsByTagName('c2')[0].childNodes[0].nodeValue;\n	row.cells.item(3).innerHTML = xmlDoc.getElementsByTagName('c3')[0].childNodes[0].nodeValue;\n	row.cells.item(4).innerHTML = xmlDoc.getElementsByTagName('c4')[0].childNodes[0].nodeValue;\n	row.cells.item(5).innerHTML = xmlDoc.getElementsByTagName('c5')[0].childNodes[0].nodeValue;\n	row.cells.item(6).innerHTML = xmlDoc.getElementsByTagName('c6')[0].childNodes[0].nodeValue;\n	row.cells.item(7).innerHTML = xmlDoc.getElementsByTagName('c7')[0].childNodes[0].nodeValue;\n	row.cells.item(8).innerHTML = xmlDoc.getElementsByTagName('c8')[0].childNodes[0].nodeValue;\n	row.cells.item(9).innerHTML = xmlDoc.getElementsByTagName('c9')[0].childNodes[0].nodeValue;\n}\n\nfunction profileaction(id, action)\n{\n	return makeHttpRequest( '/profiles?id='+id+'&action='+action , true, 'xmlupdateprofile', 'profile'+id);\n}\n\nfunction dotimer()\n{\n	if (!requestError) {\n		if (currentid>0) profileaction(currentid, 'refresh');\n		t = setTimeout('dotimer()',1000);\n	}\n}\n</script>\n");
	tcp_write(&tcpbuf, sock, http_head_, strlen(http_head_) );
	tcp_writestr(&tcpbuf, sock, "<BODY onload=\"dotimer();\">");
	tcp_write(&tcpbuf, sock, http_menu, strlen(http_menu) );

	sprintf( http_buf, "<br>Total Profiles: %d", total_profiles() ); tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
	sprintf( http_buf, "<br><center><table class=yellow width=100%%><tr><th width=150px>Profile name</th><th width=50px>Port</th><th width=60px>EcmTime</th><th width=60px>TotalECM</th><th width=90px>AcceptedECM</th><th width=80px>Ecm OK</th><th width=80px>Cache Hits</th><th width=80px>Instant Hits</th><th width=80px>Clients</th><th>Caid:Providers</th></tr>");
	tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );

	int alt = 0, j = 0;
	for (i = 0; i < 100; i++) {
		if (alt==1) alt=2; else alt=1;
		snprintf(cell[0], sizeof(cell[0]), "profile %d", i);
		for (j = 1; j < 11; j++)
			snprintf(cell[j], sizeof(cell[j]), "c %d", j);
			
		sprintf( http_buf,"<tr id=\"profile%d\" class=alt%d onMouseOver='currentid=%d'><td>%s</td><td class=%s>%s</td><td align=center>%s</td><td align=center>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>\n",i,alt,i,cell[0],cell[10],cell[1],cell[2],cell[3],cell[4],cell[5],cell[6],cell[7],cell[8],cell[9]);
		tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
	}

	sprintf( http_buf, "</table></center>"); tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
	tcp_flush(&tcpbuf, sock);
}

#include "bmsearch.c"

void http_send_editor(int sock, http_request *req)
{
	char http_buf[1024];
	struct tcp_buffer_data tcpbuf;
	tcp_init(&tcpbuf);
	tcp_write(&tcpbuf, sock, http_replyok, strlen(http_replyok) );
	tcp_write(&tcpbuf, sock, http_html, strlen(http_html) );
	tcp_write(&tcpbuf, sock, http_head, strlen(http_head) );
	sprintf( http_buf, http_title, "Editor"); tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
	tcp_write(&tcpbuf, sock, http_link, strlen(http_link) );
	tcp_write(&tcpbuf, sock, http_style, strlen(http_style) );
	tcp_write(&tcpbuf, sock, http_head_, strlen(http_head_) );
	tcp_write(&tcpbuf, sock, http_body, strlen(http_body) );
	tcp_write(&tcpbuf, sock, http_menu, strlen(http_menu) );

	if (req->type==HTTP_POST) {
		// Check Content-Type
		char *content = isset_header(req, "Content-Type");
		if (!content) {
			debugf(" Invalid form\n");
			return;
		}
		printf("content = %s\n", content);
		// Parse Content-type
		if ( memcmp(content,"multipart/form-data",19) ) {
			debugf(" Invalid Content-type\n");
			return;
		}
		// Get ';'
		while (*content!=';') {
			if (*content==0)  {
				debugf(" Invalid header data\n");
				return;
			}
			content++;
		}
		content++;
		// Skip Spaces
		while (*content==' ') content++;
		// Get Boundry
		if ( memcmp(content,"boundary",8) ) {
			debugf(" Invalid Content-type\n");
			return;
		}
		// Get '='
		while (*content!='=') {
			if (*content==0)  {
				debugf(" Invalid header data\n");
				return;
			}
			content++;
		}
		content++;
		// Skip Spaces
		while (*content==' ') content++;
		// Get Boundary Value
		char boundary[255];
		sprintf( boundary, "\r\n--%s", content);
		printf(" boundary: '%s'\n", boundary);

		// search for boundary in file
		content = req->dbf.data;

		while (content) {
			content = (char*) boyermoore_horspool_memmem( (uchar*)content, req->dbf.datasize-(content-(char*)req->dbf.data), (uchar*)boundary, strlen(boundary) );
			if (content) {
				content += strlen(boundary);
				if ( *content=='\r' && *(content+1)=='\n' ) {
					content+=2;
					// Get Content-Disposition
					// Content-Disposition: form-data; name="textedit"
					char *p = content;
					while (*p!='\r') p++;
					if ( *p=='\r' && *(p+1)=='\n' && *(p+2)=='\r' && *(p+3)=='\n' ) { // Good
						*p=0;
						//printf(" Content: '%s'\n", content);
						char *pdata = p+4;
						// search for newt boundary
						content = (char*)boyermoore_horspool_memmem( (uchar*)content, req->dbf.datasize-(content-(char*)req->dbf.data), (uchar*)boundary, strlen(boundary) );
						*content = 0;
						//printf(" the file is:\n-------------\n%s\n-------------\n", pdata); 
						// save
						FILE *cfgfd = fopen( config_file, "w");
						if (!cfgfd) {
							sprintf( http_buf, "<h2>Error opening config file</h2>");
						}
						else {
							fwrite( pdata, 1, content-pdata, cfgfd);
							fclose(cfgfd);
							sprintf( http_buf, "<script type=\"text/JavaScript\"><!--\nsetTimeout(\"location.href = '/editor';\",5000);\n--></script>\n<h3><center>Config is Successfully Saved</center></h3>");
						}
						
						tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
					}
				}
			}
		}
		tcp_flush(&tcpbuf, sock);
	}
	else {
		sprintf( http_buf, "<form enctype=\"multipart/form-data\"action=\"/editor\" method=\"post\"><center><textarea cols=\"40\" rows=\"10\" spellcheck=\"false\" name=\"textedit\">"); tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
		FILE *fd = fopen(config_file, "r");
		while( !feof(fd) ) {
			int len = fread(http_buf, 1, sizeof(http_buf), fd);
			if (len<=0) break;
			tcp_write(&tcpbuf, sock, http_buf, len );
		}
		sprintf( http_buf, "</textarea><br><input type=\"submit\" value=\"Save '%s'\"><br></center></form>",config_file);
		tcp_write(&tcpbuf, sock, http_buf, strlen(http_buf) );
		tcp_flush(&tcpbuf, sock);
	}
}


int atoint(char *index)
{
  int n=0;
  while (*index)
  { 
    if ( (*index<'0')||(*index>'9') ) return n;
    else n = n*10 + (*index - '0');
    index++;
  }
  return n;
}

#include "base64.c"


void *gererClient(int *param )
{
	int sock = *param;
	http_request req;

	struct pollfd pfd;
	pfd.fd = sock;
	pfd.events = POLLIN | POLLPRI;
	int retval = poll(&pfd, 1, 2000);

	dynbuf_init(&req.dbf, 1024);

	if ( retval>0 )
	if ( pfd.revents & (POLLIN|POLLPRI) ) 
	if ( parse_http_request(sock, &req) ) {

		int auth=0;
		if ((req.type==HTTP_GET) || (req.type==HTTP_POST)) {
			// check for auth
			if (!http.user[0] || !http.user[0]) auth = 1;
			else {
				int i;
				for(i=0; i<req.hdrcount; i++) {
					if( !strcmp(req.headers[i].name,"Authorization") ) {
						//printf("Authorization: %s\n", req.headers[i].value);
						//get auth type
						if (!memcmp(req.headers[i].value, "Basic ",6)) {
							// get encrypted login
							char pass[256];
							char realpass[256];
							base64_pdecode( &req.headers[i].value[6], pass);
							//printf("PASS: %s\n",pass);
							sprintf(realpass,"%s:%s", http.user, http.pass);
							if (!strcmp(pass,realpass)) auth=1;
						}
						break;
					}
				}
			}
			if ( auth ) {
				if (strcmp(req.path,"/")==0) 
					http_send_index(sock,&req);
				else if (strcmp(req.path,"/profiles")==0) {
					http_send_profiles(sock,&req);
				}
				else if (strcmp(req.path,"/restart")==0) {
					http_send_restart(sock,&req);
				}
				else if (strcmp(req.path,"/editor")==0) {
					http_send_editor(sock,&req);
				}
				else if (strcmp(req.path,"/connect.png")==0) {
					http_send_image(sock, &req, connect_png, sizeof(connect_png), "png");
				}
				else if (strcmp(req.path,"/disconnect.png")==0) {
					http_send_image(sock, &req, disconnect_png, sizeof(disconnect_png), "png");
				}
				else if (strcmp(req.path,"/enable.png")==0) {
					http_send_image(sock, &req, enable_png, sizeof(enable_png), "png");
				}
				else if (strcmp(req.path,"/disable.png")==0) {
					http_send_image(sock, &req, disable_png, sizeof(disable_png), "png");
				}
				else if (strcmp(req.path,"/refresh.png")==0) {
					http_send_image(sock, &req, refresh_png, sizeof(refresh_png), "png");
				}
				else if (strcmp(req.path,"/favicon.png")==0) {
					http_send_image(sock, &req, favicon_png, sizeof(favicon_png), "png");
				}
				else if (strcmp(req.path, "/upg") == 0) {
					http_send_html(sock, &req, "./upg.html");
				}
				else if (strcmp(req.path, "/upload") == 0) {
					http_get_bin(sock, &req, "./upg.bin");
				}
				else if (strcmp(req.path, "/check") == 0) {
					http_send_checking(sock, &req);
				}
				else if (strcmp(req.path, "/update") == 0) {
					http_send_updating(sock, &req);
				}
			}
			else { // send( client_sock, (char*)data, strlen(data),0);
				//printf("%s\n", http_buf);
				struct tcp_buffer_data tcpbuf;
				char auth[] = "HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Basic realm=\"TinyHttpserver\"\r\nVary: Accept-Encoding\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<HTML><HEAD><TITLE>Multi Cardserver</TITLE></HEAD><BODY><H2>Access forbidden, authorization required</H2></BODY></HTML>";
				tcp_init(&tcpbuf);
				tcp_write(&tcpbuf, sock, auth, strlen(auth) );
				tcp_flush(&tcpbuf, sock);
				//return;
			}
		}
	}

	dynbuf_free(&req.dbf);

	//printf("*Deconnexion de %s(%d)\n", tt, sock);

	if ( close(sock) ) debugf(" HTTP Server: socket close failed(%d)\n",sock);

	return NULL;
}

void *http_thread(void *param)
{
	int client_sock;
	struct sockaddr_in client_addr;
	socklen_t socklen = sizeof(client_addr);

	while(1) {
		if (http.handle>0) {
			pthread_mutex_lock(&http.lockhttp);

			struct pollfd pfd;
			pfd.fd = http.handle;
			pfd.events = POLLIN | POLLPRI;
			int retval = poll(&pfd, 1, 3000);
			if ( retval>0 ) {
				if ( pfd.revents & (POLLIN|POLLPRI) ) {
					client_sock = accept(http.handle, (struct sockaddr*)&client_addr, /*(socklen_t*)*/&socklen);
					if ( client_sock<0 ) {
						debugf(" HTTP Server: Accept Error\n");
						break;
					}
					else {
						pthread_t cli_tid;
						create_prio_thread(&cli_tid, (threadfn)gererClient, &client_sock, 50);
					}
				}
			}
			else if (retval<0) {
				debugf(" THREAD HTTP: poll error %d(errno=%d)\n", retval, errno);
				usleep(50000);
			}
			pthread_mutex_unlock(&http.lockhttp);
		}
		usleep(30000);
	}// While

	debugf("Exiting HTTP Thread\n");
	return NULL;
}

int start_thread_http()
{
	create_prio_thread(&http.http_tid, http_thread,NULL, 50);
	return 0;
}

