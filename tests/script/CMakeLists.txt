project(dbusmenuscript)

add_library(dbusmenuscript STATIC dbusmenuscript.cpp)

set_target_properties(dbusmenuscript PROPERTIES COMPILE_FLAGS -fPIC)

qt5_use_modules(dbusmenuscript Core DBus Test)

