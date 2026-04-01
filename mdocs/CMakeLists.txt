# Создаем CMakeLists.txt для проекта

 cmake_minimum_required(VERSION 3.10)
 project(NetServer)

 # Настройка компилятора
 add_compile_options(-std=c++17)

 # Указываем папки для исходных файлов
 set(CMAKE_CXX_STANDARD 17)
 set(CMAKE_CXX_EXTENSIONS OFF)

 # Добавляем библиотеки
 include(GNUInstallDirs)
 install(TARGETS \$<CONFIG:netserver_LIBRARY_FILE_NAME> DESTINATION \$<INSTALL_DIR>

 # Компиляция и установка
 add_executable(netserver src/main.cpp)
 target_link_libraries(netserver netserver_lib)
 install(TARGETS netserver DESTINATION bin)
