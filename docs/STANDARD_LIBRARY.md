# ObSL Standard Library Reference

This document provides comprehensive documentation for ObSL's built-in standard library.

## Table of Contents

- [Conversion Functions](#conversion-functions)
- [Math Functions](#math-functions)
- [String Functions](#string-functions)
- [Regex Functions](#regex-functions)
- [System Functions](#system-functions)
- [Reflection Functions](#reflection-functions)
- [Error Handling](#error-handling)

## Conversion Functions

### `to_string(value)`

Converts a number to its string representation.

**Parameters**:

- `value`: Number

**Returns**: String representation

**Examples**:

```obsl
to_string(42)        // "42"
to_string(3.14)      // "3.14"
to_string(-0.5)      // "-0.5"
```

### `to_fixed(value, decimals)`

Formats a number with a fixed number of decimal places.

**Parameters**:

- `value`: Number to format
- `decimals`: Number of decimal places (0 or positive integer)

**Returns**: Formatted string

**Examples**:

```obsl
to_fixed(3.14159, 2)  // "3.14"
to_fixed(5, 3)        // "5.000"
to_fixed(0.123, 1)    // "0.1"
```

### `to_num(string)`

Parses a string as a number.

**Parameters**:

- `string`: String containing a number

**Returns**: Parsed number

**Throws**: Error if string is not a valid number

**Examples**:

```obsl
to_num("42")       // 42
to_num("-3.14")    // -3.14
to_num("abc")      // Error
```

## Math Functions

### Basic Math

#### `sqrt(x)`

Square root of x.

**Parameters**: `x` (number)
**Returns**: Square root
**Example**: `sqrt(25)` → `5`

#### `pow(base, exponent)`

Raises base to the power of exponent.

**Parameters**: `base`, `exponent` (numbers)
**Returns**: Result
**Example**: `pow(2, 3)` → `8`

#### `abs(x)`

Absolute value of x.

**Parameters**: `x` (number)
**Returns**: Absolute value
**Example**: `abs(-5)` → `5`

#### `min(a, b)`

Returns the smaller of two values.

**Parameters**: `a`, `b` (numbers)
**Returns**: Minimum value
**Example**: `min(3, 5)` → `3`

#### `max(a, b)`

Returns the larger of two values.

**Parameters**: `a`, `b` (numbers)
**Returns**: Maximum value
**Example**: `max(3, 5)` → `5`

#### `floor(x)`

Rounds down to nearest integer.

**Parameters**: `x` (number)
**Returns**: Floored value
**Example**: `floor(3.7)` → `3`

#### `ceil(x)`

Rounds up to nearest integer.

**Parameters**: `x` (number)
**Returns**: Ceiled value
**Example**: `ceil(3.2)` → `4`

#### `round(x)`

Rounds to nearest integer.

**Parameters**: `x` (number)
**Returns**: Rounded value
**Example**: `round(3.6)` → `4`

#### `random()`

Returns a random number between 0 and 1.

**Returns**: Random number in [0, 1)
**Example**: `random()` → `0.123456` (varies)

#### `clamp(value, min, max)`

Clamps value between min and max.

**Parameters**: `value`, `min`, `max` (numbers)
**Returns**: Clamped value
**Example**: `clamp(10, 0, 5)` → `5`

### Trigonometry

#### `sin(x)`

Sine of x (in radians).

**Parameters**: `x` (number, radians)
**Returns**: Sine value
**Example**: `sin(0)` → `0`

#### `cos(x)`

Cosine of x (in radians).

**Parameters**: `x` (number, radians)
**Returns**: Cosine value
**Example**: `cos(0)` → `1`

#### `tan(x)`

Tangent of x (in radians).

**Parameters**: `x` (number, radians)
**Returns**: Tangent value
**Example**: `tan(0)` → `0`

#### `atan2(y, x)`

Arc tangent of y/x (in radians).

**Parameters**: `y`, `x` (numbers)
**Returns**: Angle in radians
**Example**: `atan2(1, 1)` → `0.785398` (π/4)

### Constants and Conversions

#### `pi()`

Returns the value of π (pi).

**Returns**: `3.141592653589793`
**Example**: `pi()` → `3.141592653589793`

#### `rad(degrees)`

Converts degrees to radians.

**Parameters**: `degrees` (number)
**Returns**: Radians
**Example**: `rad(180)` → `3.141592653589793` (π)

#### `deg(radians)`

Converts radians to degrees.

**Parameters**: `radians` (number)
**Returns**: Degrees
**Example**: `deg(3.14159)` → `180`

#### `lerp(start, end, t)`

Linear interpolation between start and end.

**Parameters**:

- `start`: Starting value
- `end`: Ending value
- `t`: Interpolation factor (0 to 1)

**Returns**: Interpolated value
**Example**: `lerp(0, 10, 0.5)` → `5`

#### `map_value(value, inMin, inMax, outMin, outMax)`

Maps a value from one range to another.

**Parameters**:

- `value`: Value to map
- `inMin`, `inMax`: Input range
- `outMin`, `outMax`: Output range

**Returns**: Mapped value
**Example**: `map_value(50, 0, 100, 0, 1)` → `0.5`

## String Functions

### `len(text)`

Returns the length of a string.

**Parameters**: `text` (string)
**Returns**: Length (number)
**Example**: `len("hello")` → `5`

### `to_upper(text)`

Converts string to uppercase.

**Parameters**: `text` (string)
**Returns**: Uppercase string
**Example**: `to_upper("hello")` → `"HELLO"`

### `to_lower(text)`

Converts string to lowercase.

**Parameters**: `text` (string)
**Returns**: Lowercase string
**Example**: `to_lower("HELLO")` → `"hello"`

### `trim(text)`

Removes whitespace from both ends.

**Parameters**: `text` (string)
**Returns**: Trimmed string
**Example**: `trim("  hello  ")` → `"hello"`

### `contains(text, value)`

Checks if string contains substring.

**Parameters**:

- `text`: String to search
- `value`: Substring to find

**Returns**: `true` if found, `false` otherwise
**Example**: `contains("hello", "ell")` → `true`

### `starts_with(text, value)`

Checks if string starts with substring.

**Parameters**:

- `text`: String to check
- `value`: Prefix to find

**Returns**: `true` if starts with, `false` otherwise
**Example**: `starts_with("hello", "hell")` → `true`

### `substring(text, start, length)`

Extracts a substring.

**Parameters**:

- `text`: Source string
- `start`: Starting index (0-based)
- `length`: Length of substring

**Returns**: Extracted substring
**Example**: `substring("hello", 1, 3)` → `"ell"`

### `replace(text, old, new)`

Replaces all occurrences of substring.

**Parameters**:

- `text`: Source string
- `old`: Substring to replace
- `new`: Replacement string

**Returns**: Modified string
**Example**: `replace("hello", "l", "x")` → `"hexxo"`

## Regex Functions

### `regex_match(text, pattern)`

Checks if entire string matches regex pattern.

**Parameters**:

- `text`: String to test
- `pattern`: Regex pattern

**Returns**: `true` if matches, `false` otherwise
**Example**: `regex_match("123", "^\\d+$")` → `true`

### `regex_search(text, pattern)`

Checks if string contains regex match.

**Parameters**:

- `text`: String to search
- `pattern`: Regex pattern

**Returns**: `true` if found, `false` otherwise
**Example**: `regex_search("abc123", "\\d+")` → `true`

### `regex_replace(text, pattern, replacement)`

Replaces regex matches with replacement string.

**Parameters**:

- `text`: Source string
- `pattern`: Regex pattern
- `replacement`: Replacement string

**Returns**: Modified string
**Example**: `regex_replace("abc123", "\\d+", "XYZ")` → `"abcXYZ"`

### `regex_find_all(text, pattern)`

Finds all regex matches in string.

**Parameters**:

- `text`: String to search
- `pattern`: Regex pattern

**Returns**: Array of matches
**Example**: `regex_find_all("a1b2c3", "\\d")` → `["1", "2", "3"]`

## System Functions

### File Operations

#### `file_exists(path)`

Checks if file exists.

**Parameters**: `path` (string)
**Returns**: `true` if exists, `false` otherwise
**Example**: `file_exists("data.txt")` → `true` or `false`

#### `read_file(path)`

Reads entire file as string.

**Parameters**: `path` (string)
**Returns**: File contents
**Throws**: Error if file doesn't exist
**Example**: `read_file("data.txt")` → file contents

#### `write_file(path, data)`

Writes string to file.

**Parameters**:

- `path`: File path
- `data`: String to write

**Example**: `write_file("output.txt", "Hello")`

#### `get_file_ext(path)`

Gets file extension (with leading dot).

**Parameters**: `path` (string)
**Returns**: Extension including the dot
**Example**: `get_file_ext("file.txt")` → `".txt"`

### Input/Output

#### `read()`

Reads a single word from standard input (whitespace-delimited).

**Returns**: Input string
**Example**: `var input = read();`

#### `readln()`

Reads an entire line from standard input.

**Returns**: Input string
**Example**: `var input = readln();`

### Environment

#### `get_env(name)`

Gets the value of an environment variable.

**Parameters**: `name` (string)
**Returns**: Value string, or empty string if not set
**Example**: `get_env("PATH")`

### Process

#### `exit(code)`

Terminates script execution with a status code (thrown as exception).

**Parameters**: `code` (number)
**Throws**: Runtime error with exit message
**Example**: `exit(1)`

### Time Functions

#### `clock()`

Returns current time in milliseconds (converted to seconds).

**Returns**: Time in seconds as double
**Example**: `var start = clock();`

#### `sleep_thread(seconds)`

Sleeps current thread.

**Parameters**: `seconds` (number)
**Example**: `sleep_thread(1.5)`

## Reflection Functions

### `type_of(value)`

Returns the type of a value.

**Parameters**: `value` (any)
**Returns**: Type name string

**Possible Returns**:

- `"null"`
- `"bool"`
- `"number"`
- `"string"`
- `"array"`
- `"object"`
- `"callable"`

**Examples**:

```obsl
type_of(42)        // "number"
type_of("hello")   // "string"
type_of(null)      // "null"
type_of([])        // "array"
type_of(Object())  // "object"
type_of(fn() {})   // "callable"
```

### `get_arity(function)`

Returns the number of parameters a function expects.

**Parameters**: `function` (callable)
**Returns**: Number of parameters
**Example**: `get_arity(fn(a, b) {})` → `2`

### `has_field(object, fieldName)`

Checks if an object has a field.

**Parameters**:

- `object`: Object to check
- `fieldName`: Field name (string)

**Returns**: `true` if field exists, `false` otherwise
**Example**: `has_field(obj, "value")` → `true` or `false`

### `get_fields(object)`

Returns all field names of an object.

**Parameters**: `object` (object)
**Returns**: Array of field names
**Example**: `get_fields(obj)` → `["field1", "field2"]`

## Error Handling

### `assert(condition, message)`

Throws an error if condition is false.

**Parameters**:

- `condition`: Boolean expression
- `message`: Error message (string)

**Throws**: Error with message if condition is false
**Example**: `assert(x > 0, "x must be positive")`

### `throw(message)`

Throws a runtime error.

**Parameters**: `message` (string)
**Throws**: Error with message
**Example**: `throw("Invalid operation")`

### Try/Catch

```obsl
try {
    // Code that might throw
    var result = risky_operation();
} catch (error) {
    // Handle error
    println "Error: " + error;
}
```

## Usage Examples

### Math Example

```obsl
var radius = 5;
var area = pi() * pow(radius, 2);
println "Area: " + to_fixed(area, 2);
```

### String Processing

```obsl
var text = "  Hello World!  ";
var cleaned = trim(to_lower(text));
println "Length: " + len(cleaned);
```

### File Processing

```obsl
if (file_exists("data.txt")) {
    var content = read_file("data.txt");
    var upper = to_upper(content);
    write_file("output.txt", upper);
}
```

### Data Validation

```obsl
var email = "user@example.com";
if (regex_match(email, "^[^@]+@[^@]+\\.[^@]+$")) {
    println "Valid email";
} else {
    println "Invalid email";
}
```

### Reflection Example

```obsl
var obj = Object();
obj.name = "Test";
obj.value = 42;

if (has_field(obj, "name")) {
    println "Object has 'name' field";
}

var fields = get_fields(obj);
println "Fields: " + fields;
```