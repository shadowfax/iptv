#include "sockets/socket.h"
#include <stdio.h>
#include <curl/curl.h>

static const char *client_profile_url =
    "http://172.26.22.23:2001/appserver/mvtv.do?action=getClientProfile";
static const char *platform_profile_url =
    "http://172.26.22.23:2001/appserver/mvtv.do?action=getPlatformProfile";

/* holder for curl fetch */
struct curl_fetch_st {
    char *payload;
    size_t size;
};

/* callback for curl fetch */
size_t curl_callback (void *contents, size_t size, size_t nmemb, void *userp) 
{
	size_t realsize = size * nmemb;                             /* calculate buffer size */
	struct curl_fetch_st *p = (struct curl_fetch_st *) userp;   /* cast pointer to fetch struct */
 
	/* expand buffer */
	p->payload = (char *) realloc(p->payload, p->size + realsize + 1);
 
	/* check buffer */
	if (p->payload == NULL) {
		/* this isn't good */
		fprintf(stderr, "ERROR: Failed to expand buffer in curl_callback");
		/* free buffer */
		free(p->payload);
		/* return */
		return -1;
	}
 
	/* copy contents to buffer */
	memcpy(&(p->payload[p->size]), contents, realsize);
 
	/* set new buffer size */
	p->size += realsize;
 
	/* ensure null termination */
	p->payload[p->size] = 0;
 
	/* return size */
	return realsize;
}

CURLcode curl_fetch_url(CURL *ch, const char *url, struct curl_fetch_st *fetch)
{
	CURLcode rcode;                   /* curl result code */
 
	/* init payload */
	fetch->payload = (char *) calloc(1, sizeof(fetch->payload));
 
	/* check payload */
	if (fetch->payload == NULL) {
		/* log error */
		fprintf(stderr, "ERROR: Failed to allocate payload in curl_fetch_url");
		/* return error */
		return CURLE_FAILED_INIT;
	}
 
	/* init size */
	fetch->size = 0;
 
	/* set url to fetch */
	curl_easy_setopt(ch, CURLOPT_URL, url);
 
	/* set calback function */
	curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, curl_callback);
 
	/* pass fetch struct pointer */
	curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void *) fetch);
 
	/* set default user agent */
	curl_easy_setopt(ch, CURLOPT_USERAGENT, "libcurl-agent/1.0");
 
	/* set timeout */
	curl_easy_setopt(ch, CURLOPT_TIMEOUT, 5);
 
	/* enable location redirects */
	curl_easy_setopt(ch, CURLOPT_FOLLOWLOCATION, 1);
 
	/* set maximum allowed redirects */
	curl_easy_setopt(ch, CURLOPT_MAXREDIRS, 1);
 
	/* fetch the url */
	rcode = curl_easy_perform(ch);
 
	/* return */
	return rcode;
}

int main(int argc, char * argv[])
{
	CURL *ch;												/* curl handle */
	CURLcode rcode;                                         /* curl result code */
	struct curl_slist *headers = NULL;                      /* http headers to send with request */
	struct curl_fetch_st curl_fetch;                        /* curl fetch struct */
	struct curl_fetch_st *cf = &curl_fetch;                 /* pointer to fetch struct */

#ifdef WIN32
	WSADATA wsaData;

	if (0 != WSAStartup(MAKEWORD(2,2), &wsaData)) {
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}
#endif

	/* init curl handle */
    if ((ch = curl_easy_init()) == NULL) {
        /* log error */
        fprintf(stderr, "ERROR: Failed to create curl handle in fetch_session");
        /* return error */
        return 1;
    }

	/* set content type */
    headers = curl_slist_append(headers, "Accept: application/json");

	/* fetch page and capture return code */
	rcode = curl_fetch_url(ch, platform_profile_url, cf);

	/* cleanup curl handle */
	curl_easy_cleanup(ch);
 
	/* free headers */
	curl_slist_free_all(headers);

	/* check return code */
	if (rcode != CURLE_OK || cf->size < 1) {
		/* log error */
		fprintf(stderr, "ERROR: Failed to fetch url (%s) - curl said: %s", platform_profile_url, curl_easy_strerror(rcode));
		/* return error */
		return 2;
    }

	/* check payload */
	if (cf->payload != NULL) {
		/* print result */
		printf("CURL Returned: \n%s\n", cf->payload);
		/* parse return */
		//json = json_tokener_parse_verbose(cf->payload, &jerr);
        /* free payload */
		free(cf->payload);
	} else {
		/* error */
		fprintf(stderr, "ERROR: Failed to populate payload");
		/* free payload */
		free(cf->payload);
		/* return */
		return 3;
    }

#ifdef WIN32
	WSACleanup();
#endif

	printf("\nPress any key to continue...");
	getchar();
}