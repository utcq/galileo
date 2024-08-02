# Galileo

## Module Management

### Caching

`a:`

```py
import x
import y
```

`x:`

```py
import y
```

the module `y` will be imported once (by `x`), processed into a scope that contains all the declarations.
When `a` will try to import `y` the already proecssed scope will be appended to `a` as a sub-module.

```
         ┌───────────────────────────────────────────────────────────────┐
         │                                                               │
         │                   ┌───────────────────────────────────────────┤
         │                   │                                           │
         │   Scope A         │                          ┌────────────────┤
         │   (Scope X)       │      Scope X             │                │
         │   (Scope Y)       │      (Scope Y)           │   Scope Y      │
         │                   │                          │   ()           │
┌────────┴───────────┬────┬──┴──────────────────┬────┬──┴────────────────┴─┐
│                    │    │                     │    │                     │
│                    │    │                     │    │                     │
│   Module A         │    │    Module X         │    │     Module Y        │
│                    │    │                     │    │                     │
│                    │    │                     │    │                     │
└───────┬────────────┘    └──────────────┬──────┘    └─────────────────────┘
        │                         ▲      │                       ▲
        │                         │      │                       │
        │                         │      │                       │
        │                         │      │                       │
        │                         │      │                       │
        │                         │      └───────────────────────┘
        │                         │                   ▲
        │                         │                   │
        │                         │                   │
        ├─────────────────────────┘                   │
        │                                             │
        └─────────────────────────────────────────────┘
```

### Visibility

`a:`

```py
import x
```

`x:`

```py
import y
```

> In this case while we're in `a` the scope `x` will be accessible but `x.y` won't

`a:`

```py
import x
```

`b:`

```py
pub import y
```

> In this case `a` can access both `x` and `x.y` because the import policy for `y` is public

#### Same thing for any other declaration

```ts
const var myVar: uint = 10;
```

`myVar` will be private

```ts
pub const var myVar: uint = 10;
```

(pretty long statement uh)

`myVar` will be public

## Syntax

### Functions

```c
f!int add(a: int, b:int) {
  return a + b;
}
```

`f!` prefix indicates a function **declaration + implementation**

Galileo isn't tab dipendent so you can do pretty declarations:

```c
f!int
add(
  a: int,
  b: int
) {
  return a + b;
}
```

#### But I would keep the standard to inline declaration

### Loops

```c
for (a:int=0; a < 8; a++) {
  // ...
}
while (1) {
  // ...
}
```

( Yes I do love C and its syntax)

### Conditional

```c
if (x==0) {
  // ...
}
elif (x==1) {
  // ...
}
else {
  // ...
}
```

## Preprocessor

> Galileo comes with a preprocessor!

> **It isn't a simple find and replace, a new sub-lexer is spawned on every macro call**

And It looks like this:

```c
m!def ADD(a,b) a+b
```

```c
ADD(1,2)
// Expands to
1 + 2
```

---

```c
m!def VALUE_DEF 50
```

## Prefixes

### Galileo is prefix based

This means that most declarations have a prefix indicating the type

```c
m!def NAME VAL
```

`m!` stands for **macro** and `def` obv stands for **definition**

```c
f!int main() {}
```

`f!` stands for **function** while `int` is the function return type
