# Investigation of `dynamic_cast` pitfalls with shared libraries.

Using `dynamic_cast` and other RTTI features with class hierarchies defined
across different libraries can easily lead to strange errors and warnings.
This repository contains a simple example exploring some of these issues on
macOS/clang.

The main take-away is that you should export the class (configure its member symbols to be visible in the library)
*and put the first non-inline virtual method definition
for each polymorphic class in a single translation unit (source file)* so that
the compiler generates a *single* vtable for the class in that file. Otherwise,
there can be multiple `vtable` copies that confuse `dynamic_cast`.

# Anatomy of the example

The example consists of four libraries:
- `libbase` contains only the polymorphic base class `Base`.
- `libderived` defines a few classes deriving from `Base` and a factory function `make_derived` for constructing them.
- `lib1` and `lib2` each link against the above libraries and export testing functions that attempt to `dynamic_cast` a `Base` pointer to the derived types.

The `main` executable links against all of the above libraries, calls the testing functions of `lib1` and `lib2`, and attempts to do its own `dynamic_cast` inline.

Depending on how the `Base`/`Derived` class definitions are implemented and
exported in shared libraries, the `dynamic_cast` can either fail or produce
certain warnings.

# 1) Safe approach: out-of-line virtual method in a single translation unit

The apparently safest approach is demonstrated by building with the command:
```bash
./build.sh 1
```

This moves a virtual method definition (in this case, the destructor) out-of-line
and into a *single* translation unit (`Base.cc` for `Base` and `Derived.cc` for
the derived classes).
It also annotates each class with an `EXPORT` macro
(which expands to `__attribute__((visibility("default")))` on GCC/Clang).
This causes the compiler to generate a single `vtable` for each class
in the associated shared library (`libbase.so` and `libderived.so`):

```sh
$ ./grep_vtable_.so.sh
build/lib1.so
build/lib2.so
build/libbase.so
0000000000004008 S vtable for Base
build/libderived.so
0000000000004078 S vtable for Derived1
00000000000040b0 S vtable for Derived2
```

Running the resulting binary produces no warnings or errors, with all `dynamic_cast` calls succeeding.

If we forget to annotate one of the classes with `EXPORT` (*e.g.*, `Derived2`), we get a helpful linker error:
```
Undefined symbols for architecture arm64:
  "typeinfo for Derived2", referenced from:
      attempt_cast(Base const*) in lib1.o
ld: symbol(s) not found for architecture arm64
```
enabling us quickly identify and fix the error.

# 2) Fully inline classes without `EXPORT`: `dynamic_cast error 2`

Building the code with the command:
```bash
./build 2
```
makes the class definitions fully inline and removes the `EXPORT` keyword. In this case, the `dynamic_cast`
operations actually do all work, but scary errors like the following are printed to the system log on macOS for each cast:
```
2024-05-12 12:39:24.947836-0700 main[41145:18140379] dynamic_cast error 2: One or more of the following type_info's has hidden visibility or is defined in more than one translation unit. They should all have public visibility. 4Base, 8Derived1, 8Derived1.
```
Note that these errors only display in the terminal when running the code through a debugger; otherwise they
go "silently" into system log, where they can be found by monitoring `Console.app` or the command:
```
log stream --predicate 'eventMessage CONTAINS[c] "dynamic_cast"'
```

This issue has been observed in [this Stack Overflow thread](https://stackoverflow.com/questions/51297638/how-to-fix-type-infos-has-hidden-visibility-they-should-all-have-public-visib), though
it's unclear if the underlying cause is the same.

Of course in this case, no `vtable`s are visible in the shared libraries:
```sh
$ ./grep_vtable_.so.sh
build/lib1.so
build/lib2.so
build/libbase.so
build/libbase.so: no symbols
build/libderived.so
```
however, all `vtable`s happened to go in `derived.o`:

```sh
$ ./grep_vtable_.o.sh
build/derived.o
0000000000001628 S vtable for Base
00000000000015e0 S vtable for Derived1
0000000000001648 S vtable for Derived2
0000000000001680 S vtable for Derived3
build/lib1.o
build/lib2.o
build/libbase.o
build/main.o
````

# 3) Fully inline classes with `EXPORT`: seemingly OK, but brittle

Building the code with the command
```
./build.sh 3
```
keeps the classes fully inline but adds the `EXPORT` macro to *every* class. This
fixes the `dynamic_cast error 2` messages, and all casts continue to work.
Interestingly, the output of the `grep_vtable_.*.sh` commands are identical to the previous approach
(all `vtable`s are visible in `derived.o`, none show up in the `.so` libraries).

The danger is that now if we forget to annotate a single one of the
classes with the `EXPORT` macro, we no longer get any errors or even warnings at build
time--and dynamic casts to that class now fail. For example, removing the
macro from `Derived2`, we see the following failures:
```
lib1_run dynamic_cast failed: 8Derived2 vs 8Derived1 and 8Derived2
lib2_run dynamic_cast failed: 8Derived2 vs 8Derived1 and 8Derived2
main dynamic_cast failed: 8Derived2 vs 8Derived1 and 8Derived2
```
despite `typeid(*b).name()` matching `typid(Derived2).name()` as seen in the printouts.
The outputs of `grep_vtable_.*.sh` are again identical to what we saw in
approach (2).

This situation is much worse than with approach (1), where forgetting the
`EXPORT` macro produced a linker error.

# 4) Out-of-line classes exported in multiple translation units: `dynamic_cast` failures

Finally, building the code with the command:
```
./build.sh 4
```
attempts to apply approach (1) but switches `libderived` from being a shared library to being a static library (`libderived.a`).
This means that distinct versions of the derived class `vtable`s are generated in `lib1.so` and `lib2.so`:
```
$ ./grep_vtable_.so.sh
build/lib1.so
0000000000004108 S vtable for Derived1
0000000000004140 S vtable for Derived2
build/lib2.so
0000000000004108 S vtable for Derived1
0000000000004140 S vtable for Derived2
build/libbase.so
0000000000004008 S vtable for Base
```

The consequence is that `dynamic_cast` will fail in either `lib1.so` or
`lib2.so` because one of them will try to match against a different copy of the `vtable`:
```
lib1_run dynamic_cast Derived1
lib2_run dynamic_cast failed: 8Derived1 vs 8Derived1 and 8Derived2
main dynamic_cast Derived1
lib1_run dynamic_cast Derived2
lib2_run dynamic_cast failed: 8Derived2 vs 8Derived1 and 8Derived2
main dynamic_cast Derived2
```
Note again that the `typeid(*).name()` strings do match despite the cast failure.

# Related references
The influence of class symbol visibility on RTTI features is discussed in several places online, e.g.:

- [GCC wiki](https://gcc.gnu.org/wiki/Visibility)
- [Apple's C++ Runtime Environment Programming Guide](https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/CppRuntimeEnv/Articles/SymbolVisibility.html#//apple_ref/doc/uid/TP40001670-CJBGBHEJ)
- [Herzbube's wiki](https://wiki.herzbube.ch/index.php/Mac_OS_X_Programming#Symbol_visibility)

although I've not seen this particular `dynamic_cast` issue investigated in detail.
