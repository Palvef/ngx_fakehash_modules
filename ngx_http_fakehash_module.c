#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <openssl/evp.h>

// Function to generate 32-character hash
static ngx_int_t generate_fakehash(ngx_http_request_t *r, ngx_str_t *fakehash) {
    // Check if r or its members are NULL
    if (r == NULL || r->connection == NULL || r->connection->addr_text.data == NULL || r->headers_in.user_agent == NULL) {
        return NGX_ERROR;
    }

    // Getting IP, UA, and current time
    ngx_str_t addr = r->connection->addr_text;
    ngx_str_t ua;
    if (r->headers_in.user_agent == NULL || r->headers_in.user_agent->value.len == 0) {
        // If UA is empty, set it to "-"
        ua.data = (u_char *)"-";
        ua.len = 1;
    } else {
        ua = r->headers_in.user_agent->value;
    }
    ngx_time_t *tp = ngx_timeofday();

    // Check if buffers are large enough to hold the data
    if (addr.len + ua.len + 21 > 512) {
        return NGX_ERROR;
    }

    // Converting ngx_str_t to null-terminated C strings
    char addr_str[addr.len + 1];
    char ua_str[ua.len + 1];
    ngx_memcpy(addr_str, addr.data, addr.len);
    ngx_memcpy(ua_str, ua.data, ua.len);
    addr_str[addr.len] = '\0';
    ua_str[ua.len] = '\0';

    char data[512];
    snprintf(data, sizeof(data), "%s|%s|%ld", addr_str, ua_str, tp->sec);

    // Initialize OpenSSL EVP interface
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len, i;

    md = EVP_md5();

    mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        return NGX_ERROR;
    }

    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, data, strlen(data));
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_free(mdctx);

    // Convert hash to hex string
    static char hex[] = "0123456789abcdef";
    char hash_str[33];
    for (i = 0; i < md_len; i++) {
        hash_str[i * 2] = hex[md_value[i] >> 4];
        hash_str[i * 2 + 1] = hex[md_value[i] & 0xF];
    }
    hash_str[32] = '\0';

    // Assign the generated hash to fakehash variable
    fakehash->len = 32;
    fakehash->data = (u_char *)ngx_palloc(r->pool, 33);
    if (fakehash->data == NULL) {
        return NGX_ERROR;
    }
    ngx_memcpy(fakehash->data, hash_str, 32);
    fakehash->data[32] = '\0';

    return NGX_OK;
}

// Variable handler
static ngx_int_t ngx_http_fakehash_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data) {
    ngx_str_t fakehash;
    if (generate_fakehash(r, &fakehash) != NGX_OK) {
        return NGX_ERROR;
    }

    v->len = fakehash.len;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = fakehash.data;

    return NGX_OK;
}

// Module initialization
static ngx_int_t ngx_http_fakehash_add_variables(ngx_conf_t *cf) {
    ngx_http_variable_t *var;
    ngx_str_t name = ngx_string("fakehash");

    var = ngx_http_add_variable(cf, &name, NGX_HTTP_VAR_NOCACHEABLE | NGX_HTTP_VAR_NOHASH);
    if (var == NULL) {
        return NGX_ERROR;
    }
    var->get_handler = ngx_http_fakehash_variable;
    var->data = 0;

    return NGX_OK;
}

// Module context
static ngx_http_module_t ngx_http_fakehash_module_ctx = {
    ngx_http_fakehash_add_variables, // preconfiguration
    NULL,                            // postconfiguration
    NULL,                            // create main configuration
    NULL,                            // init main configuration
    NULL,                            // create server configuration
    NULL,                            // merge server configuration
    NULL,                            // create location configuration
    NULL                             // merge location configuration
};

// Module definition
ngx_module_t ngx_http_fakehash_module = {
    NGX_MODULE_V1,
    &ngx_http_fakehash_module_ctx,  // module context
    NULL,                           // module directives
    NGX_HTTP_MODULE,                // module type
    NULL,                           // init master
    NULL,                           // init module
    NULL,                           // init process
    NULL,                           // init thread
    NULL,                           // exit thread
    NULL,                           // exit process
    NULL,                           // exit master
    NGX_MODULE_V1_PADDING
};
