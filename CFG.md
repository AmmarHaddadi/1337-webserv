# Webserv Config (Caddyfile-like)

**note:** this file has been rewritten using AI for consistency, with manual edits, it might have a mistake or two

This project uses a small, Caddyfile-inspired configuration format to define one or more HTTP servers and optional per-route behavior (custom roots, uploads, redirects, etc).

By default the server looks for a `server.conf` in CWD, to use a custom file pass it as param
```bash
./webser path/to/file.conf
```
***note**: it doesn't need to end in a .conf*

---

## 1) Syntax

### 1.1 General structure

A configuration file is a sequence of **server blocks**:

```caddyfile
<address> {
    <statements...>
}
```

- `<address>` is written as:  
  - `:PORT` (bind all interfaces), e.g. `:8080`
  - `HOST:PORT` (bind specific interface), e.g. `127.0.0.1:8080`
- Ports below `1024` usually require elevated privileges.
- Port is **16-bit unsigned integer** capped at 65535, it can overflow if you use higher

Inside a server block you can declare:
- **server-level directives** (apply to the whole server)
- **route blocks** using `handle` (apply only to matching paths)

---

### 1.2 Lexical rules (how the file is read)

The parser should follow these rules:

- **Whitespace** (spaces/tabs/newlines) separates tokens (surrounding in "" is not supported ).
- `{` and `}` delimit blocks.
- **Comments** start with `#` and continue until end-of-line.
- Paths are treated as single tokens and therefore must not contain spaces (v1).

---

### 1.3 Server blocks

Example:

```caddyfile
:8080 {
    root /www
    error_page 404 404.html
    max_body_sz 2048

    handle /upload {
        upload_enabled /tmp/uploads
    }
}
```

Server-level directives can appear in any order. Route blocks can also appear in any order.

---

### 1.4 Route blocks (`handle`)

A route block is declared inside a server block:

```caddyfile
handle <path> {
    <route-directives...>
}
```

- `<path>` must begin with `/` (e.g. `/`, `/api`, `/upload`).
- Route blocks allow per-path overrides (e.g. different root, uploads, redirect).

Example:

```caddyfile
:8080 {
    root /www

    handle /p1 {
        root /tmp/abc
    }
}
```

---

## 2) Keywords / Directives

This section defines each supported keyword, its scope, parameters, defaults, and notes.
a param surrouneded in `<>` is required, `[]` means optional

### 2.1 `root`
**Scope:** server, route  
**Syntax:**
```caddyfile
root <path>
```

- **Server scope**
  - **Required:** yes
  - Sets the base filesystem path where files are looked up.
- **Route scope**
  - **Required:** no
  - **Default:** inherits server `root`
  - Overrides the filesystem root for this route.

Notes:
 - must be absolute path.
 - must not end with `/`.



Example:
```caddyfile
:8080 {
    root /www

    handle /images {
        root /data/images
    }
}
```

---

### 2.2 `error_page`
**Scope:** server  
**Syntax:**
```caddyfile
error_page <code> <path>
```

- **Required:** no  
- **Default:** built-in webserv error pages
- `<code>` is the status code relevant to the page 
- `<path>` is a path **relative to the server root** that will be served on errors (must not start with `/`)

Example:
```caddyfile
:8080 {
    root /www
    error_page 404 404.html
}
```

---

### 2.3 `max_body_sz`
**Scope:** server  
**Syntax:**
```caddyfile
max_body_sz <kilobytes>
```

- **Required:** no  
- **Default:** `1024` (KB)
- Maximum request body size in **KB**.
- Value must be a positive integer (must fit in `size_t`).

Notes:
- File uploads are excluded from this limit (TBD: separate upload limit).

---

### 2.4 `methods` 
**Scope:** route  
**Syntax:**
```caddyfile
methods <M1> [M2 ...]
```

- **Methods allowed:** `GET`, `POST`, `DELETE`
- **Default:** nothing is allowed 
- Behavior, defaults, and requirement are **TBD**.

Examples:
```caddyfile
handle /route1 {
    methods GET
}

handle /route2 {
    methods GET POST DELETE
}
```

---

### 2.5 `default_file`
**Scope:** route  
**Syntax:**
```caddyfile
default_file <path>
```

- **Required:** no  
- **Default:** `index.html`
- Default file to serve when the requested resource is a directory.
- If `default_file` is specified, directory listing / browsing only works if such file doens't exist in that dir.

---

### 2.6 `upload_enabled`
**Scope:** route  
**Syntax:**
```caddyfile
upload_enabled <path>
```

- **Required:** no  
- Enables file uploads for this route.
- `path` is required, uploaded files are saved there.

Examples:
```caddyfile
handle /abc {
    upload_enabled /tmp/uplaods
}
```

```caddyfile
handle /abc {
    upload_enabled /var/user_data
}
```

Notes:
- Enabling uploads automatically enables support for the `POST` method on this route.

---

### 2.7 `redirect`
**Scope:** route  
**Syntax:**
```caddyfile
redirect <code> <location>
```

- Responds with an HTTP redirect.
- `<code>` should be a valid redirect code (commonly `301`, `302`, `307`, `308`).
- `<location>` is the `Location` header value (path or absolute URL).

Reference:
- https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Redirections

---

### 2.8 `file_server`
**Scope:** route  
**Syntax:**
```caddyfile
file_server [browse]
```

- **Required:** no  
- Enables static file serving from the active `root`.
- Optional argument:
  - `browse` enables directory listing if the request targets a directory and no `default_file` is served.

Examples:
```caddyfile
handle /xyz {
    file_server
}
```

```caddyfile
handle /xyz {
    file_server browse
}
```

Notes:
- Enabling `file_server` implies support for `GET` requests.
- To enable file_server serverwide make route for `/` and add it to it

---

## 3) Minimal complete example

```caddyfile
:8080 {
    root /www
    error_page 404 404.html
    max_body_sz 1024

    handle /upload {
        upload_enabled /tmp/uploads
    }

    handle /old {
        redirect 302 /new
    }
}
```
