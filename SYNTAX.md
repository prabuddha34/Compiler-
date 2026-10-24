# 🧾 HyLang Syntax Cheat Sheet

Source files end in `.hy`. Everything the compiler accepts, on one page. Every statement ends with `;` unless it ends with a `{ }` block.

---

## 1. Values

| Kind | Examples | Notes |
|---|---|---|
| Integer | `0` `42` `1000` | 64-bit signed, the main type |
| String | `"hello"` | escapes: `\n` `\t` `\"` `\\`; print with `print("...")` |
| Double | `3.14` | parsed only; arithmetic is integer-only |

There is **no unary minus** (write `0 - 5`) and **no comment syntax** yet.

## 2. Variables

```text
let x = 10;
let y = x * 2 + 1;
```

- Declare once with `let`. A repeated name in scope is a compile error.
- No `x = 5;` reassignment statement.
- Variables declared inside `{ }` vanish at the closing brace.

## 3. Arrays

```text
let a = [1, 2, 3];      // literal (at least one element)
let b[5];               // five zeros, size must be an integer literal
a[0] = 99;              // assign element
print(a[1]);            // read element
print(len(a));          // length
```

An out-of-range index exits the program with code 1.

## 4. Operators

| Precedence | Operators | Associativity |
|:-:|---|---|
| 1 (lowest) | `==` `!=` `<` `>` `<=` `>=` | left |
| 2 | `+` `-` | left |
| 3 | `*` `/` | left |
| 4 (highest) | literals, identifiers, calls, `( )`, `a[i]` | n/a |

Comparisons give `1` or `0`. `/` is signed integer division.

## 5. Statements

```text
print(expr);
exit(expr);

if (cond) { ... }
if (cond) { ... } else { ... }

for (i = start to end) { ... }
```

- `if`: any non-zero value is true. `else` must be followed by `{ }`; there is no `else if`.
- `for`: runs while `i < end`, so **`end` is excluded**. `for (i = 0 to 3)` gives 0, 1, 2.

## 6. Built-in functions

### Parenthesized form: `name(args)`

| Call | Meaning |
|---|---|
| `gcd(a, b)` | greatest common divisor |
| `max(a, b)` / `min(a, b)` | larger / smaller |
| `power(a, b)` | a to the power b |
| `random(a, b)` | random integer, both bounds inclusive |
| `equal(a, b)` | 1 if equal, else 0 |
| `sqrt(x)` | integer square root, rounded down |
| `fact(x)` | factorial |
| `prime(x)` | 1 if prime, else 0 |
| `palindrome(x)` | 1 if palindrome, else 0 |
| `count_digits(x)` | number of digits |
| `time_utc()` | Unix time in seconds |
| `len(arr)` | array length |
| `input().int` | read a signed integer from stdin |

### Prefix form: `name operand` (operand is a single primary)

| Call | Meaning |
|---|---|
| `abs x` | absolute value |
| `even x` | 1 if even, 0 if odd |
| `mid_val x` | `x / 2`, rounded toward zero |
| `digit_sum x` | sum of digits |
| `n_fibonacci x` | n-th Fibonacci number (F0 = 0, F1 = 1) |

Parentheses also work (`abs(x - 5)`). Use them whenever the operand is more than one value, since `abs x - 5` means `(abs x) - 5`.

### List form: `sum a, b, c, ...`

```text
print(sum 1, 2, 3, 4);
```

Needs at least two values. Greedy: wrap it in parentheses inside other calls.

## 7. Reserved words

`let` `print` `exit` `if` `else` `for` `to` `input` `abs` `gcd` `sum` `mid_val` `even` `time_utc` `digit_sum` `n_fibonacci` `count_digits` `prime` `palindrome` `max` `min` `equal` `random` `sqrt` `power` `fact`

`len` and `int` are not reserved: `len` is recognised only before `(`, and `int` only after `input().`.

Identifiers use letters, digits and `_`, and cannot start with a digit. Keywords are case-sensitive.

## 8. Not supported (yet)

comments · `while` · `else if` · `%` · unary `-` · `&&` `||` `!` · reassignment · user functions · double arithmetic · printing negative numbers correctly

---

## 9. Mini grammar (EBNF)

```ebnf
program        = { statement } ;
statement      = print | exit | let | array_let | array_assign | if | for ;

print          = "print" "(" expr ")" ";" ;
exit           = "exit"  "(" expr ")" ";" ;
let            = "let" IDENT "=" expr ";" ;
array_let      = "let" IDENT "=" "[" expr { "," expr } "]" ";"
               | "let" IDENT "[" INT "]" ";" ;
array_assign   = IDENT "[" expr "]" "=" expr ";" ;
if             = "if" "(" expr ")" block [ "else" block ] ;
for            = "for" "(" IDENT "=" expr "to" expr ")" block ;
block          = "{" { statement } "}" ;

expr           = additive { cmp_op additive } ;
additive       = term { ("+" | "-") term } ;
term           = primary { ("*" | "/") primary } ;
primary        = INT | DOUBLE | STRING | IDENT
               | IDENT "[" expr "]" | "len" "(" IDENT ")"
               | call | "(" expr ")" ;
cmp_op         = "==" | "!=" | "<" | ">" | "<=" | ">=" ;
```
