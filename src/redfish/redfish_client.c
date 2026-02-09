#include "bmctool/redfish.h"
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char* data;
    size_t size;
} http_response_t;

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    http_response_t* resp = (http_response_t*)userp;
    
    char* ptr = realloc(resp->data, resp->size + realsize + 1);
    if (!ptr) {
        bmc_log(LOG_LEVEL_ERROR, "Memory allocation failed");
        return 0;
    }
    
    resp->data = ptr;
    memcpy(&(resp->data[resp->size]), contents, realsize);
    resp->size += realsize;
    resp->data[resp->size] = '\0';
    
    return realsize;
}

int http_get(redfish_ctx_t* ctx, const char* path, char** response_out) {
    CURL* curl;
    CURLcode res;
    
    curl = curl_easy_init();
    if (!curl) {
        return BMC_ERROR_NETWORK;
    }
    
    char url[512];
    snprintf(url, sizeof(url), "%s%s", ctx->base_url, path);
    
    http_response_t response = {0};
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    
    if (ctx->username[0] != '\0') {
        curl_easy_setopt(curl, CURLOPT_USERNAME, ctx->username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, ctx->password);
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    }
    
    if (!ctx->verify_ssl) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }
    
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    bmc_log(LOG_LEVEL_DEBUG, "GET %s", url);
    
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        bmc_log(LOG_LEVEL_ERROR, "curl_easy_perform() failed: %s", 
                curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(response.data);
        return BMC_ERROR_NETWORK;
    }
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    bmc_log(LOG_LEVEL_DEBUG, "HTTP %ld, %zu bytes", http_code, response.size);
    
    curl_easy_cleanup(curl);
    
    if (http_code != 200) {
        bmc_log(LOG_LEVEL_ERROR, "HTTP error: %ld", http_code);
        free(response.data);
        return BMC_ERROR_PROTOCOL;
    }
    
    *response_out = response.data;
    return BMC_SUCCESS;
}
