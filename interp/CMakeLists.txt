add_llvm_loadable_module( VerifierPass MODULE main.cpp domain.cpp z3_handler.cpp)

# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(VerifierPass PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(VerifierPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti -fpermissive -g -fopenmp"
)

target_link_libraries(VerifierPass -lapron -lboxD -litv -lgmp -lapron_debug -lboxMPQ_debug -lmpfr -lz3)
