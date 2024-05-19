Name
====

ngx_http_fakehash_modules

## Overview
The `ngx_http_fakehash_module` is an Nginx module that generates a unique 32-character hash string for each request based on the client's IP address, User-Agent, and the current access time. This hash is generated using the MD5 algorithm via the OpenSSL EVP interface and can be used for various purposes such as session identification, request tracking, or debugging.

## Installation

```
git clone https://github.com/nginx/nginx.git
cd nginx
```

Create a directory for the fakehash module within the Nginx source tree.

```
cd modules
git clone https://github.com/Palvef/ngx_http_fakehash_modules.git
```
Compile the Module:

```
./configure --add-dynamic-module=modules/ngx_http_fakehash_modules
make modules
```

## Configuration
Add the following line to the top of your nginx.conf file to load the module:

```
load_module /path/to/ngx_http_fakehash_module.so;
```

Use the $fakehash Variable:

```
http {
    server {
        location / {
            return 200 "$fakehash";
        }
    }
}

```