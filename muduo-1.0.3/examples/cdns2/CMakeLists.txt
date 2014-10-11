add_library(muduo_cdns2 Resolver2.cc)
target_link_libraries(muduo_cdns2 muduo_net)
target_link_libraries(muduo_cdns2 cares)

install(TARGETS muduo_cdns2 DESTINATION lib)
install(FILES Resolver2.h DESTINATION include/muduo/cdns2)

add_executable(cdns2 dns2.cc)
target_link_libraries(cdns2 muduo_cdns2)

