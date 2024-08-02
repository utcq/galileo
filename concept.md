# Galileo

## Interpretation

One shot only.

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
┌─────────────────────────────────────┐   ┌─────────────────────────────────────┐  ┌─────────────────────────────────────┐
│                                     │   │                                     │  │                                     │
│                                     │   │                                     │  │                                     │
│                                     │   │                                     │  │                                     │
│         Module A                    │   │         Module X                    │  │          Module Y                   │
│                                     │   │                                     │  │                                     │
│                                     │   │                                     │  │                                     │
│                                     │   │                                     │  │                                     │
└─────────────────┬───────────────────┘   └────────────────────┬────────────────┘  └─────────────────────────────────────┘
                  │                           ▲                │                                         ▲
                  │                           │                │                                         │
                  │                           │                │                                         │
                  │                           │                │                                         │
                  │                           │                │                                         │
                  │                           │                │                                         │
                  │                           │                │                                         │
                  │                           │                │                                         │
                  │                           │                │                                         │
                  │                           │                │                                         │
                  │                           │                │                                         │
                  │                           │                │                                         │
                  │                           │                │                                         │
                  │                           │                │                                         │
                  │                           │                │                                         │
                  ├───────────────────────────┘                └─────────────────────────────────────────┘
                  │                                                                    ▲
                  │                                                                    │
                  │                                                                    │
                  │                                                                    │
                  │                                                                    │
                  │                                                                    │
                  │                                                                    │
                  │                                                                    │
                  │                                                                    │
                  │                                                                    │
                  │                                                                    │
                  └────────────────────────────────────────────────────────────────────┘
```
