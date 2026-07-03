# The Obliberry Scripting Language

ObSL is a small interpreted, thread safe, dynamically typed programming language designed for game logic inside
the [Obliberry Game
Engine](https://github.com/torkelicious/obliberry) made to be easily extendable.

---

## Documentation

- [Architecture Overview](docs/ARCHITECTURE.md)
- [Standard Library Reference](docs/STANDARD_LIBRARY.md)

---

# Features

- Dynamically typed variables
- Functions with default parameters
- Closures and first-class functions
- Structs with custom constructors
- Dynamic objects
- Dynamic arrays with negative indexing
- Module importing
- Exception handling
- Runtime reflection
- Built-in regex support
- Automatic garbage collection with cyclic reference handling

---

## Building from Source

### Prerequisites

- C++20 compatible compiler (GCC, Clang, MSVC)
- CMake
- Git

### Clone the Repository

```bash
git clone https://github.com/torkelicious/ObSL.git
cd ObSL
```

### Build the Project

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Run the Interpreter

```bash
./obsl_runtime
```

---

# Quick Start

```obsl
println "Hello, World!";
````

Variables are dynamically typed:

```obsl
var score = 100;
var name = "Player";
var alive = true;
var nothing = null;
```

---

# Language Basics

## Variables

Variables are declared using the `var` keyword:

```obsl
var number = 100;
var text = "hello";
var enabled = true;
```

Variables can change type:

```obsl
var value = 10;
value = "Now a string";
```

---

# Printing

ObSL provides `print` and `println`.

### print (no newline)

```obsl
print "Hello ";
print "World";
```

Output:

```
Hello World
```

### println (with newline)

```obsl
println "Hello";
println "World";
```

Output:

```
Hello
World
```

---

# Operators

## Arithmetic

```obsl
var a = 10 + 5;
var b = 10 - 5;
var c = 5 * 2;
var d = 10 / 2;
var remainder = 10 % 3;
```

---

## Comparison

```obsl
a == b;
a != b;

a > b;
a >= b;

a < b;
a <= b;
```

---

## Logical

```obsl
true and false;
true or false;
!true;

// Alternative syntax
true && false;
true || false;
```

---

## Assignment

```obsl
a = 10;

a += 5;
a -= 2;
a *= 3;
a /= 2;

a++;
a--;
```

---

## Bitwise

```obsl
var bitAnd = 5 & 3;
var bitOr = 5 | 3;
var bitXor = 5 ^ 3;

var bitNot = ~0;

var left = 2 << 3;
var right = 16 >> 2;
```

---

# Control Flow

## If Statements

```obsl
var score = 100;

if (score > 50) {
    println "Passed!";
} else {
    println "Failed!";
}
```

---

## Switch

```obsl
var value = 2;

switch (value) {
    case 1:
        println "One";
    case 2:
        println "Two";
    default:
        println "Unknown";
}
```

---

## Type Checking

The `is` operator checks runtime type:

```obsl
var value = 10;

if (value is number) {
    println "Number";
}
```

### Supported Types

```
number
string
boolean / bool
null / nil
function / fn
array
object
```

> **Note**: These are the type names accepted by `is`, which are distinct from the strings returned by the reflection function `type_of()` — most notably, `is` uses `function`/`fn` where `type_of()` returns `"callable"`. See the [Standard Library Reference](docs/STANDARD_LIBRARY.md#type_ofvalue) for `type_of()`'s exact return values.

### Expressions

```obsl
var result = 5 + 3;

if (result is number) {
    println "Math result";
}
```

### Precedence

`is` binds tighter than equality checks:

```obsl
(x == y) is boolean;
```

### Null Checking

```obsl
var value = null;

if (value is null) {
    println "Empty";
}

if (value is nil) {
    println "Also empty";
}
```

---

# Loops

## While

```obsl
var i = 0;

while (i < 5) {
    println i;
    i++;
}
```

---

## For

```obsl
var total = 0;

for (var i = 0; i < 10; i++) {
    total += i;
}
```

---

## Foreach

```obsl
var numbers = [10, 20, 30];
var sum = 0;

foreach (var number in numbers) {
    sum += number;
}
```

---

## Break

```obsl
for (var i = 0; i < 10; i++) {
    if (i == 5) {
        break;
    }
}
```

---

# Arrays

Arrays are dynamic and support negative indexing:

```obsl
var arr = [10, 20, 30];

println arr[0];

arr[-1] = 99;
```

### Negative Indexing

```
arr[-1] // last element
arr[-2] // second last element
```

### Methods

```obsl
println arr.len;

arr.push(100);

var item = arr.pop();

arr.clear();
```

---

# Functions

## Basic Functions

```obsl
fn sayHello(name) {
    println "Hello " + name;
}

sayHello("Player");
```

---

## Return Values

```obsl
fn add(a, b) {
    return a + b;
}

var result = add(5, 10);
```

---

## Default Parameters

```obsl
fn damage(amount = 10) {
    return amount;
}

println damage();
```

---

## Closures

```obsl
fn createAdder(amount) {
    fn add(value) {
        return value + amount;
    }

    return add;
}

var addFive = createAdder(5);
println addFive(10);
```

---

# Structs

```obsl
struct Vector3 {
    x;
    y;
    z;
}
```

## Default Values

```obsl
struct Player {
    health = 100;
    name = "Unknown";
}
```

Example:

```obsl
var player = Player();
println player.health;
```

---

# Objects

```obsl
var obj = Object();

obj.value = 42;
obj.printValue = printValue;
```

---

# Modules

```obsl
using "assets/scripts/module.obsl";
```

---

# Error Handling

## Try / Catch

```obsl
try {
    var result = sqrt("invalid");
} catch (err) {
    println err;
}
```

## Assertions

```obsl
assert(player != null, "Player missing");
```

## Throwing Errors

```obsl
throw("Something failed!");
```

---

# Built-in Library

## Conversion

```obsl
to_string(value);
to_fixed(value, decimals);
to_num(string);
```

---

## Math

```obsl
sqrt(x);
pow(a, b);
abs(x);
min(a, b);
max(a, b);
floor(x);
ceil(x);
round(x);
random();
clamp(value, min, max);
```

### Trigonometry

```obsl
sin(x);
cos(x);
tan(x);
atan2(y, x);
```

### Helpers

```obsl
pi();
rad(degrees);
deg(radians);
lerp(start, end, t);
map_value(value, inMin, inMax, outMin, outMax);
```

---

## Strings

```obsl
len(text);
to_upper(text);
to_lower(text);
trim(text);
contains(text, value);
starts_with(text, value);
substring(text, start, length);
replace(text, old, new);
```

---

## Regex

```obsl
regex_match(text, pattern);
regex_search(text, pattern);
regex_replace(text, pattern, replacement);
regex_find_all(text, pattern);
```

---

## System Utilities

### Files

```obsl
file_exists(path);
read_file(path);
write_file(path, data);
get_file_ext(path);
```

### Input

```obsl
read();
readln();
```

### Time

```obsl
clock();
sleep_thread(seconds);
```

### Environment

```obsl
get_env(name);
```

### Process

```obsl
exit(code);
```

---

# Reflection

## Types

```obsl
type_of(value);
```

Returns:

```
null
bool
number
string
array
object
callable
```

## Functions

```obsl
get_arity(function);
```

## Objects

```obsl
has_field(object, "name");
get_fields(object);
```

---

# Garbage Collection

ObSL includes automatic garbage collection with cyclic reference handling.

---

# Examples

See the `Examples/` directory for additional runnable usage examples.

---

# todo:

- Proper Linter
- LSP?

This is still very much a toy language :)

---

# Resources & Acknowledgments

## Learning Resources

ObSL's design and implementation were informed by:

- [Crafting Interpreters](https://craftinginterpreters.com/contents.html) by Robert Nystrom
- [A Quick Guide to Interpreter Design in Modern C++](https://simplifycpp.org/books/cpp/Quick_Guide_to_Interpreter_Design_by_Modern_CPP.pdf) by Ayman Alheraki

## Third-Party Libraries

| Library | License | Purpose | More |
|---|---|---|---|
| [nlohmann/json](https://github.com/nlohmann/json) v3.12.0 | MIT | JSON output for `--lint` mode | [docs/THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md) |

# License
 
ObSL is licensed under the [MIT License](LICENSE). See third-party dependency licenses in [THIRD_PARTY_LICENSES.md](docs/THIRD_PARTY_LICENSES.md).
