# header_utils

A collection of header-only C++20 "libraries" that implement small but useful utilities.

**This is in-progress, live-at-head code that most likely has a lot of bugs, was not thoroughly tested, and requires C++20** (or at least, the version of C++20 that MSVC supports, did not test other compilers, though the code should be portable).

## align.h

A small enum to represent alignment within a rectangle (e.g. top-left, bottom-right, etc), and functions to modify it and align stuff.

## assuming.h

An in-my-humble-opinion better version of the `assert` concept.

Pros:
* The macro names "Assuming*" suggest that in the following code we are **assuming** the predicate given to the macro.
* The predicate and its parameters are always evaluated exactly once.
* A custom, user-provided "assumption failure" function is called on assumption failure.
* You can provide additional descriptions and arguments to the assumption failure function, for better debugging.
* Includes variants like `AssumingEqual(a, b, ...)` that assumes arguments are equal; each such variant makes sure to evaluate and stringify the arguments, and give a helpful message, such as: "Assumption Failed: Assuming that a will equal b."
* In non-debug compiles, the library instructs the compiler to actually ASSUME its predicate, so it can optimize the code better; keep in mind that the assumption failing is undefined behavior in non-debug compiles.
* Better macros to control the behavior of this library.

Cons:
* Depends on <format> and <source_location> headers.
* Still macro-based (only for stringification of arguments, thankfully.)
* Using compiler-specific code for enforcing assumptions
* Slightly non-idiomatic naming conventions as well as some tiny assumptions about the user code-base

## di.h

A tiny and as-simple-as-possible Dependency Injection/IoC library, based around `shared_ptr`s.

Works, but is not-documented and might contain bugs. Still, you should be able to register types, instances, factory functions, 
and the construction arguments should resolve automatically, so, a basic DI container.

Supports custom instance lifetimes (strong and weak singletons, per-thread singleton, multiple-instances), naming interfaces/instances, instance creation callbacks, and probably other stuff.

## enum_flags.h

A value struct that represents a set of bits mapped to an enum. E.g.:

    enum class door_flags { closed, locked, blue };
    enum_flags<door_flags> flags;
    flags.set(door_flags::closed, door_flags::blue);
    if (flags.is_set(door_flags::locked)) { ... }
    if (flags.are_all_set(door_flags::closed, door_flags::locked)) { print("Not getting in"); }
    flags.toggle(door_flags::locked);
    for (auto flag : flags)
      print("{} is set", flag);
    
* Supports +/-/+=/-=/==/!= (decided not to support &/|/^/~ to not confuse the model and implementation).
* Is as constexpr as I could make it
* You can specify the base integral value to store bits in (uint64_t by default)
* Uses concepts to ensure most functions do not overflow or produce garbage results (e.g. enum values of -5 or 100 won't work with uint64_t)
* The first template parameter doesn't have to be an enum, any integral value will work
* Also provides a set of *flag_bits* functions to do all the same operations directly on integers, without using the `enum_flags` class template
* Also provides a set of *flag_v* inline variable templates to do all these operations as constexpr variable expressions (e.g. `constexpr auto bits = set_flag_v<10, door_flags::closed>;`)

## named.h

A simple type wrapper to use strong typing for safer coding. E.g.:

    using degrees_t = named<double, "degrees">;
    using radians_t = named<double, "radians">;
    
    void SetAngle(degrees_t deg); void SetAngle(radians_t rad);

## rec2.h

An extension to the `glm` library. Adds a class template that represents a 2D axis-aligned rectangle.

Has all the usual methods, plus: `at_center, at_position, bottom, calculate_area, center, contains, edge_length, edge_point, edge_point_alpha, grow, grown, half_size, height, include, including, intersection, intersects, is_valid, left, left_bottom, left_top, local, make_valid, position, projected, relative_to, right, right_bottom, right_top, scaled, set_center, set_height, set_position, set_size, set_width, shrink, shrunk, size, sized, to_rect_space, to_world_space, top, translated, valid, width, x, y` and others.

## string_ops.h

Adds a few utility functions that deal with strings and string_views.

* The `ascii` namespace contains equivalents to the C standard library character and string functions like `isalpha`, that do not depend on the current locale, are constexpr and inlineable.
* Adds the `make_sv` and `make_string` functions that deal with the fact that in earlier version of the C++ library string_view did not have a range constructor
* trim* API that trims whitespace from different sides of a string_view
* consume* API that "consumes" characters, substrings, ranges, etc. from string_views and returns them (useful for basic parsers):
    * `consume(), consume_at_end(), consume_while(), consume_until(), consume_n(), consume_delimited_list_non_empty(), consume_delimited_list()`
    * Some consume versions that are useful for parsing c-like syntaxes: `consume_c_identifier(), consume_c_identifier_with(), consume_c_float(), consume_c_integer(), consume_c_unsigned(), consume_c_string()`
* A very basic an non-robust UTF-8 API that relies on the input strings/codepoints being valid: `consume_utf8(), append_utf8(), to_utf8(), struct utf8_view`
* Some useful utilities with good APIs that support transforming:
    * `split(), natural_split(), join(), join_and(), replace()`

