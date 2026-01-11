cmake_minimum_required(VERSION 3.16)
project(flow_browser_cpp)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 COMPONENTS Widgets WebEngineWidgets Sql Network REQUIRED)

add_executable(flow_browser_cpp
    src/main.cpp
    src/MainWindow.cpp
    src/BookmarksManager.cpp
    src/AuthManager.cpp
    src/LoginDialog.cpp
    src/SessionManager.cpp
    src/HistoryManager.cpp
    src/BookmarksPanel.cpp
    src/BookmarksConflictDialog.cpp
    src/HistoryPanel.cpp
    src/WorkspaceManager.cpp
    src/NotesManager.cpp
    src/NotesPanel.cpp
    src/NotesConflictDialog.cpp
    src/TodosManager.cpp
    src/TodosPanel.cpp
    src/TodosConflictDialog.cpp
    src/MainWindow.h
)

target_link_libraries(flow_browser_cpp PRIVATE Qt6::Widgets Qt6::WebEngineWidgets Qt6::Sql Qt6::Network)

# Unit tests
find_package(Qt6 COMPONENTS Test REQUIRED)
add_executable(test_bookmarks_manager
    ../test/bookmarks_manager_test.cpp
)
target_include_directories(test_bookmarks_manager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_bookmarks_manager PRIVATE Qt6::Test Qt6::Widgets)

add_executable(test_history_search
    ../test/history_search_test.cpp
)
target_include_directories(test_history_search PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_history_search PRIVATE Qt6::Test Qt6::Widgets)
add_executable(test_notes_manager
    ../test/notes_manager_test.cpp
    ../test/notes_manager_undo_test.cpp
    ../test/notes_conflict_test.cpp
)
target_include_directories(test_notes_manager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_notes_manager PRIVATE Qt6::Test Qt6::Widgets)

add_executable(test_todos_manager
    ../test/todos_manager_test.cpp
    ../test/todos_manager_undo_test.cpp
    ../test/todos_conflict_test.cpp
)
target_include_directories(test_todos_manager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_todos_manager PRIVATE Qt6::Test Qt6::Widgets)
add_executable(test_todos_manager
    ../test/todos_manager_test.cpp
    ../test/todos_manager_undo_test.cpp
)
target_include_directories(test_todos_manager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_todos_manager PRIVATE Qt6::Test Qt6::Widgets)

add_executable(test_devtools
    ../test/devtools_test.cpp
)
target_include_directories(test_devtools PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_devtools PRIVATE Qt6::Test Qt6::Widgets Qt6::WebEngineWidgets)

add_executable(test_incognito
    ../test/incognito_test.cpp
)
target_include_directories(test_incognito PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_incognito PRIVATE Qt6::Test Qt6::Widgets Qt6::WebEngineWidgets)
