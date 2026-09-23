# dotenv-c

A small and lightweight `.env` file parser written in C.

`dotenv-c` loads environment-style configuration values from a `.env` file into memory and provides simple functions to retrieve them from anywhere in your application.

The library is intentionally simple and has minimal dependencies, making it suitable for small C applications and projects that need basic `.env` configuration support.

---

## Features

- Load `.env` files from a default or custom path
- Read key/value pairs
- Ignore blank lines
- Ignore comments
- Trim whitespace around keys and values
- Support empty values
- Support double-quoted values
- Detect missing closing double quotes
- Retrieve values by key
- Retrieve values with a fallback/default
- Last definition wins when a key appears multiple times
- Dynamically grows its internal storage
- Explicit memory cleanup
- Simple C API
- No external runtime dependencies

---

## Requirements

The current implementation uses POSIX functionality such as `getline()` and `strdup()`.

Therefore, the current version is intended primarily for:

- Linux
- Unix-like systems
- POSIX-compatible environments
- WSL

A fully portable Windows implementation is not currently the goal of this version.

---

# Installation

## Add the source directly

The simplest way to use the library is to include the source files in your project.

Example:

```text
project/
├── main.c
├── dotenv.c
├── dotenv.h
└── .env
````

Compile:

```bash
gcc main.c dotenv.c -o app
```

Run:

```bash
./app
```

---

# Header

The public API is:

```c
#ifndef DOTENV_H
#define DOTENV_H

int load_dotenv(const char *filename);
const char *get_env(const char *key);
const char *get_env_or(const char *key, const char *default_value);
void free_dotenv(void);

#endif
```

---

# Basic Usage

## 1. Create a `.env` file

Example:

```env
DB_HOST=localhost
DB_PORT=5432
DB_USER=postgres
DB_PASSWORD=secret
DB_NAME=mydatabase
```

---

## 2. Load the `.env` file

Call `load_dotenv()` once during application startup.

Passing `NULL` loads `.env` from the current working directory.

```c
#include <stdio.h>
#include <stdlib.h>
#include "dotenv.h"

int main(void)
{
    if (load_dotenv(NULL) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    const char *host = get_env("DB_HOST");

    printf("Database host: %s\n", host);

    free_dotenv();

    return EXIT_SUCCESS;
}
```

---

# Loading a Custom File

You can provide a specific filename:

```c
load_dotenv("config.env");
```

For example:

```c
if (load_dotenv("config.env") != EXIT_SUCCESS) {
    return EXIT_FAILURE;
}
```

You can also provide a path:

```c
load_dotenv("/etc/myapp/config.env");
```

---

# Loading the Default `.env`

If `NULL` is passed:

```c
load_dotenv(NULL);
```

the library attempts to open:

```text
.env
```

in the application's current working directory.

This means the location is relative to the directory from which the program is executed, not necessarily the directory containing the executable.

For example:

```bash
cd /home/user/myapp
./app
```

The library looks for:

```text
/home/user/myapp/.env
```

---

# Getting Values

Use `get_env()` to retrieve a value:

```c
const char *host = get_env("DB_HOST");
```

Example:

```c
printf("%s\n", get_env("DB_HOST"));
```

If the key does not exist, `get_env()` returns:

```c
NULL
```

Therefore, check for `NULL` when the value is required:

```c
const char *host = get_env("DB_HOST");

if (host == NULL) {
    printf("DB_HOST is not configured\n");
}
```

---

# Default Values

Use `get_env_or()` when you want a fallback value.

```c
const char *host = get_env_or("DB_HOST", "localhost");
```

If `DB_HOST` exists:

```text
DB_HOST value is returned
```

If it does not exist:

```text
localhost
```

is returned.

Example:

```c
const char *port = get_env_or("DB_PORT", "5432");
```

This is useful for optional configuration.

---

# Using Values From Multiple Functions

The `.env` file should normally be loaded once.

For example:

```c
int main(void)
{
    if (load_dotenv(NULL) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    start_application();

    free_dotenv();

    return EXIT_SUCCESS;
}
```

Other functions can then use:

```c
const char *host = get_env("DB_HOST");
```

The file is not read again.

The configuration is stored in memory after `load_dotenv()` completes.

Recommended lifecycle:

```text
load_dotenv()
      ↓
Read .env
      ↓
Parse values
      ↓
Store values in memory
      ↓
get_env() from anywhere
      ↓
Application runs
      ↓
free_dotenv()
```

---

# Example Project

```text
myapp/
├── main.c
├── database.c
├── database.h
├── dotenv.c
├── dotenv.h
└── .env
```

### `.env`

```env
DB_HOST=localhost
DB_PORT=5432
DB_USER=postgres
DB_PASSWORD=secret
```

### `main.c`

```c
#include <stdio.h>
#include <stdlib.h>
#include "dotenv.h"
#include "database.h"

int main(void)
{
    if (load_dotenv(NULL) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    connect_database();

    free_dotenv();

    return EXIT_SUCCESS;
}
```

### `database.c`

```c
#include <stdio.h>
#include "dotenv.h"

void connect_database(void)
{
    const char *host = get_env("DB_HOST");
    const char *port = get_env_or("DB_PORT", "5432");
    const char *user = get_env("DB_USER");

    printf("Host: %s\n", host);
    printf("Port: %s\n", port);
    printf("User: %s\n", user);
}
```

This works because the configuration loaded by `main()` is stored in the library's internal static table.

---

# `.env` File Syntax

The basic syntax is:

```env
KEY=VALUE
```

Example:

```env
NAME=John
AGE=30
CITY=Kathmandu
```

---

# Whitespace

Whitespace around the key and value is removed.

For example:

```env
NAME = John
```

becomes:

```text
key   = NAME
value = John
```

Leading and trailing whitespace is removed.

Example:

```env
NAME=    John    
```

becomes:

```text
John
```

Internal whitespace is preserved.

Example:

```env
FULL_NAME=John    Doe
```

becomes:

```text
John    Doe
```

The spaces between `John` and `Doe` are not removed.

---

# Quoted Values

Double quotes are supported.

Example:

```env
MESSAGE="Hello World"
```

The stored value is:

```text
Hello World
```

The surrounding double quotes are removed.

---

## Spaces Inside Quotes

Whitespace inside quotes is preserved.

Example:

```env
MESSAGE="    Hello "
```

results in:

```text
    Hello 
```

The spaces inside the quotes are considered part of the value.

This is different from whitespace outside the quotes.

Example:

```env
MESSAGE=    Hello    
```

results in:

```text
Hello
```

---

# Empty Values

Empty values are supported:

```env
EMPTY=
```

The value returned by:

```c
get_env("EMPTY");
```

is an empty string:

```text
""
```

This is different from a key that does not exist.

For example:

```c
get_env("EMPTY");
```

returns:

```text
""
```

while:

```c
get_env("DOES_NOT_EXIST");
```

returns:

```text
NULL
```

---

# Comments

Lines beginning with `#` are ignored.

Example:

```env
# Database configuration
DB_HOST=localhost

# Application configuration
APP_PORT=8000
```

Whitespace before a comment is also handled:

```env
    # This is a comment
```

Blank lines are ignored.

---

# Duplicate Keys

If a key is defined more than once:

```env
NAME=John
NAME=Bob
```

the last definition is returned.

```c
get_env("NAME");
```

returns:

```text
Bob
```

Internally, both entries are currently stored.

The lookup searches from the newest entry toward the oldest entry.

---

# Important: `.env` Is Configuration, Not a Secure Secret Store

A `.env` file is normally plain text.

For example:

```env
DB_PASSWORD=my-secret-password
API_KEY=my-api-key
```

Anyone who can read the file can read those values.

Do not assume that putting a password in `.env` encrypts it.

If the application needs strong secret management, use an appropriate secret-management system instead.

---

# `.env` Should Usually Not Be Committed to Git

If your `.env` contains passwords, API keys, tokens, or other secrets, add it to `.gitignore`:

```gitignore
.env
```

A safer approach is to commit an example configuration:

```text
.env.example
```

Example:

```env
DB_HOST=localhost
DB_PORT=5432
DB_USER=your_user
DB_PASSWORD=your_password
DB_NAME=your_database
```

Then users can create their own:

```bash
cp .env.example .env
```

and fill in their actual values.

---

# Environment Variables vs `.env`

This library reads a file.

It does not modify the operating system's environment using functions such as:

```c
setenv()
```

Therefore:

```c
get_env("DB_HOST");
```

is the library's lookup function.

It does not mean:

```c
getenv("DB_HOST");
```

and does not automatically read values from the process environment.

The two mechanisms are separate.

---

# Restrictions and Limitations

The current implementation intentionally supports a small subset of `.env` syntax.

## 1. No Variable Expansion

This is not currently supported:

```env
BASE_URL=http://localhost
API_URL=${BASE_URL}/api
```

The library does not expand `${BASE_URL}`.

The value would remain effectively as written rather than being expanded to:

```text
http://localhost/api
```

---

## 2. No `export` Syntax

This is not supported as a special syntax:

```env
export DB_HOST=localhost
```

The parser expects:

```env
DB_HOST=localhost
```

---

## 3. Single Quotes Are Not Special

Double quotes are supported:

```env
NAME="John"
```

Single quotes are not currently treated as quote delimiters:

```env
NAME='John'
```

The single quotes may therefore remain part of the stored value.

---

## 4. No Escape Sequence Processing

Escape sequences are not interpreted.

For example:

```env
MESSAGE="Hello\nWorld"
```

does not currently convert `\n` into an actual newline character.

The parser only handles the surrounding double quotes.

---

## 5. Inline Comments Are Not Supported

A line such as:

```env
NAME=John # username
```

is not interpreted using shell-style inline comment rules.

The parser does not currently remove everything after `#`.

Similarly:

```env
MESSAGE="Hello" # comment
```

should not be assumed to behave like a full dotenv implementation.

---

## 6. Complex Values Containing `=` Have Limitations

The current implementation uses `strtok()` with `=` as the delimiter.

Basic values work:

```env
HOST=localhost
```

But complex values containing additional `=` characters should not be considered fully supported.

For example:

```env
TOKEN=abc=def=ghi
```

is subject to the behavior of the current `strtok()`-based parser.

---

## 7. Multiline Values Are Not Supported

Values are processed one line at a time.

This is not supported:

```env
MESSAGE="Hello
World"
```

The current parser expects a complete key/value pair on a single line.

---

## 8. Double-Quoted Multiline Strings Are Not Supported

A quoted value must have its opening and closing quote on the same line.

Example:

```env
MESSAGE="Hello World"
```

works.

A missing closing quote produces an error:

```env
MESSAGE="Hello World
```

---

## 9. Calling `load_dotenv()` Multiple Times

The normal usage pattern is:

```c
load_dotenv(NULL);
```

once during startup.

Calling it repeatedly is not recommended.

For example:

```c
load_dotenv(NULL);
load_dotenv(NULL);
load_dotenv(NULL);
```

causes the file to be read and parsed multiple times.

The current implementation also appends the newly parsed entries to the existing internal table.

Therefore, repeated loading can:

* consume additional memory
* duplicate entries
* perform unnecessary file I/O
* perform unnecessary parsing

Use:

```text
load once
     ↓
read many times
     ↓
free once
```

---

# Memory Management

The library dynamically allocates memory for keys and values.

For example:

```env
DB_HOST=localhost
```

is copied into the library's internal storage.

The returned pointer:

```c
const char *host = get_env("DB_HOST");
```

belongs to the library.

Do not free it manually.

This is incorrect:

```c
const char *host = get_env("DB_HOST");
free((void *)host);
```

Instead, call:

```c
free_dotenv();
```

when the configuration is no longer needed.

---

# Pointer Lifetime

Values returned by:

```c
get_env()
```

remain valid while the dotenv configuration remains loaded.

Example:

```c
const char *host = get_env("DB_HOST");

printf("%s\n", host);

free_dotenv();
```

After:

```c
free_dotenv();
```

the pointer should no longer be used.

Incorrect:

```c
const char *host = get_env("DB_HOST");

free_dotenv();

printf("%s\n", host);
```

The pointer is no longer valid after the configuration has been freed.

---

# Performance

The configuration file is read only when:

```c
load_dotenv()
```

is called.

After loading, values are stored in memory.

`get_env()` performs a linear search through the stored keys.

For a normal `.env` file containing a relatively small number of configuration variables, this is simple and inexpensive.

For example, a configuration containing:

```text
10–100 variables
```

does not require a complicated lookup structure.

However, this implementation is not designed for millions of configuration entries or extremely high-frequency lookups.

If a future version requires large-scale or high-frequency lookup, the internal storage could be changed to a hash table without changing the public API.

---

# Error Handling

`load_dotenv()` returns:

```c
EXIT_SUCCESS
```

when the file is loaded successfully.

It returns:

```c
EXIT_FAILURE
```

when the file cannot be opened or a parsing error occurs.

Example:

```c
if (load_dotenv(NULL) != EXIT_SUCCESS) {
    fprintf(stderr, "Failed to load configuration\n");
    return EXIT_FAILURE;
}
```

A malformed quoted value can produce an error such as:

```text
Error: Missing closing quotation mark for key 'NAME'
```

---

# Cleanup

Call:

```c
free_dotenv();
```

when the configuration is no longer needed.

Typical application structure:

```c
int main(void)
{
    if (load_dotenv(NULL) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    /* Application code */

    free_dotenv();

    return EXIT_SUCCESS;
}
```

The cleanup function:

* frees every stored key
* frees every stored value
* frees the configuration table
* resets internal counters
* resets internal capacity

This allows the library to be safely cleaned up.

---

# Current Supported Syntax

The following examples are supported:

```env
NAME=John
AGE=30
CITY=Kathmandu

EMPTY=

MESSAGE="Hello World"

SPACED_KEY = Spaced Value

# comment
```

---

# Current Unsupported / Limited Syntax

Do not rely on the following:

```env
export NAME=John
NAME='John'
NAME=${OTHER_NAME}
MESSAGE="Hello\nWorld"
MESSAGE="Hello
World"
NAME=John # comment
```

Complex values containing multiple `=` characters also have limitations due to the current parser implementation.

---

# API Reference

## `load_dotenv`

```c
int load_dotenv(const char *filename);
```

Loads and parses a dotenv file.

### Parameters

`filename`

* `NULL` → loads `.env`
* non-`NULL` → loads the specified file

### Returns

```c
EXIT_SUCCESS
```

or:

```c
EXIT_FAILURE
```

---

## `get_env`

```c
const char *get_env(const char *key);
```

Returns the value associated with a key.

### Example

```c
const char *host = get_env("DB_HOST");
```

Returns:

* value → if the key exists
* `NULL` → if the key does not exist

---

## `get_env_or`

```c
const char *get_env_or(
    const char *key,
    const char *default_value
);
```

Returns the configured value if the key exists, otherwise returns the supplied default value.

Example:

```c
const char *port = get_env_or("DB_PORT", "5432");
```

---

## `free_dotenv`

```c
void free_dotenv(void);
```

Releases all memory allocated by the dotenv library.

Call this when the loaded configuration is no longer needed.

---

# Recommended Application Pattern

For most applications:

```c
#include <stdio.h>
#include <stdlib.h>
#include "dotenv.h"

int main(void)
{
    if (load_dotenv(NULL) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    const char *db_host = get_env_or("DB_HOST", "localhost");
    const char *db_port = get_env_or("DB_PORT", "5432");

    printf("Database host: %s\n", db_host);
    printf("Database port: %s\n", db_port);

    free_dotenv();

    return EXIT_SUCCESS;
}
```

The recommended pattern is:

```text
START
  │
  ├── load_dotenv()
  │
  ├── get_env() / get_env_or()
  │       │
  │       ├── function A
  │       ├── function B
  │       └── function C
  │
  ├── application
  │
  └── free_dotenv()
END
```

---

# Design Philosophy

This library intentionally keeps the API small.

The core functionality is:

```text
Load
 ↓
Parse
 ↓
Store in memory
 ↓
Lookup
 ↓
Cleanup
```

It does not attempt to reproduce every feature of shell syntax or every possible dotenv implementation.

The goal is to provide a small and understandable configuration parser for C applications.

---

# Security Considerations

A `.env` file should be treated as sensitive configuration if it contains:

* passwords
* API keys
* access tokens
* database credentials
* private configuration
* service credentials

Recommended practices:

1. Do not commit `.env` to public repositories.
2. Add `.env` to `.gitignore`.
3. Restrict filesystem permissions where appropriate.
4. Use `.env.example` for non-secret configuration templates.
5. Use a dedicated secret-management system for production environments where stronger security is required.
6. Do not print secrets to logs.

Example:

```env
DB_PASSWORD=super-secret
API_TOKEN=secret-token
```

Avoid:

```c
printf("Password: %s\n", get_env("DB_PASSWORD"));
```

---

# License

Add your project's license here.

For example:

```text
MIT License
```

---

# Project Status

This project currently provides a basic dotenv parser suitable for simple C applications.

The parser is intentionally limited and does not currently implement advanced dotenv features such as:

* variable expansion
* multiline values
* escape sequence processing
* single-quote semantics
* `export` declarations
* inline comments
* full dotenv specification compatibility
* environment-variable synchronization

Future versions may expand the parser while keeping the public API stable where possible.
