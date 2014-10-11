add_executable(muduo_server server/worker.cc server/main.cc server/forward.cc)
target_link_libraries(muduo_server muduo_net)

add_executable(muduo_source source/source.cc source/main.cc)
target_link_libraries(muduo_source muduo_net)

add_executable(muduo_box box/box.cc box/main.cc)
target_link_libraries(muduo_box muduo_net)

