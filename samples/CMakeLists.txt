FUNCTION(sample_test file_name)
    add_executable(       ${file_name}     ${file_name}.cpp)
    target_link_libraries(${file_name}     pthread)
endfunction(sample_test)


sample_test(sample1)