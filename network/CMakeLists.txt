FUNCTION(sample_test file_name)
    add_executable(       ${file_name}     ${file_name}.cpp)
    target_link_libraries(${file_name}     pthread)
endfunction(sample_test)

add_subdirectory(reactor_model)

sample_test(server)
sample_test(client)

sample_test(nonblocking_connect)
sample_test(linux_nonblocking_connect)

sample_test(select_server)
sample_test(select_client)

sample_test(select_client_tv0)
sample_test(select_client_tvnull)

sample_test(epoll_server)

sample_test(blocking_server)
sample_test(blocking_client)

sample_test(nonblocking_client)
sample_test(blocking_client_recv)
sample_test(nonblocking_client_recv)

add_executable(       reactor_server    reactor_server.cpp myreactor.cpp)
target_link_libraries(reactor_server    pthread)