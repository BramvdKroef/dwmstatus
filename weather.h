
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

#include <curl/curl.h>


/**
 * Download weather rss data from a url and extract data using an xpath.
 *  
 * This could potentially work to monitor other types of rss feeds.
 */

struct weather_data {
  const char* url;
  const char* weather_xpath;
  const char* warning_xpath;
};


xmlChar* xpath(xmlXPathContextPtr xpathCtx, const char* expression) {
  xmlXPathObjectPtr xpathObj;
  xmlChar *content = NULL;

  xpathObj = xmlXPathEvalExpression(expression, xpathCtx);

  if(xpathObj == NULL) {
    printf("n/a\n");
    
  } else if (xpathObj->nodesetval->nodeNr > 0 &&
             xpathObj->nodesetval->nodeTab[0]->type == XML_TEXT_NODE) {
    content = xmlStrdup(xpathObj->nodesetval->nodeTab[0]->content);
  }
  xmlXPathFreeObject(xpathObj);
  return content;
}

size_t write_curl_data(void *buffer, size_t size, size_t nmemb, void
                       *userp) {
  xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr)userp;
  size_t real_size = size * nmemb;
  xmlParseChunk(ctxt, buffer, real_size, 0);
  return real_size;
}

void weather_update(char* status, size_t length, const char* format,
                    struct weather_data *weatherdata) {

  xmlParserCtxtPtr ctxt;
  xmlDocPtr doc;
  xmlXPathContextPtr xpathCtx; 

  int wellformed;
    
  CURL *curl;
  CURLcode res;

  xmlChar* weather;
  size_t len;

  /* Init libxml */     
  xmlInitParser();
  LIBXML_TEST_VERSION
    ;

  ctxt = xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, weatherdata->url);
  if (ctxt == NULL) {
    fprintf(stderr, "Failed to create parser context !\n");
    return;
  }
  
  curl_global_init(CURL_GLOBAL_DEFAULT);
 
  curl = curl_easy_init();
  if(!curl) {
    fprintf(stderr, "Failed to initialize curl \n");
    return;
  }

  curl_easy_setopt(curl, CURLOPT_URL, weatherdata->url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_curl_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)ctxt);
 
  /* Perform the request, res will get the return code */ 
  res = curl_easy_perform(curl);
  /* always cleanup */ 
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  /* Check for errors */ 
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
    return;
  }

  xmlParseChunk(ctxt, NULL, 0, 1);
  doc = ctxt->myDoc;
  wellformed = ctxt->wellFormed;
  xmlFreeParserCtxt(ctxt);

  if (!wellformed) {
    fprintf(stderr, "Failed to parse %s\n", weatherdata->url);
 
  } else {
    /* Create xpath evaluation context */
    xpathCtx = xmlXPathNewContext(doc);
    
    if(xpathCtx == NULL) {
      fprintf(stderr,"Error: unable to create new XPath context\n");
    } else {

      if(xmlXPathRegisterNs(xpathCtx, "atom",
                            "http://www.w3.org/2005/Atom") != 0) {
        fprintf(stderr,"Error: failed to register namespace\n");
      } else {
        weather = xpath(xpathCtx, weatherdata->weather_xpath);
        if (weather != NULL) {
          if (xmlStrncmp(weather, "Current Conditions: ", 20) == 0) {
            weather += 20;
          }
          fprintf(stderr, "%s\n", weather);
          len = snprintf(status, length + 1, format, weather);
          status[min(len, length)] = ' ';
          
        }
        //xpath(xpathCtx, weatherdata->warning_xpath);
      } 
      xmlXPathFreeContext(xpathCtx); 
    } 
    xmlFreeDoc(doc); 
  }

  /* Shutdown libxml */
  xmlCleanupParser();
    
  return;
}

int weather_ctl(CtlOp op, int fd, CtlData* data) {
  switch(op) {
  case CTL_START:
    weather_update(data->status, data->length, data->format, data->arg);
    return timer_start(3600);

  case CTL_CHECK:
    weather_update(data->status, data->length, data->format, data->arg);
  
    timer_check(fd);
    return 0;
  }
}

#endif
