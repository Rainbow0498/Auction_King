cmake_minimum_required(VERSION 3.20)

project(BlindBoxAuctionEstimator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

find_package(OpenCV QUIET)
find_package(nlohmann_json CONFIG QUIET)

set(PROJECT_SOURCES
    src/main.cpp
    src/core/AuctionAdvisor.cpp
    src/core/CombinationSolver.cpp
    src/core/Estimator.cpp
    src/core/PriceCalculator.cpp
    src/data/ConfigLoader.cpp
    src/data/ItemDatabase.cpp
    src/data/RoleConfigLoader.cpp
    src/data/RoleDatabase.cpp
    src/ui/CapturePanel.cpp
    src/ui/ColorInputCard.cpp
    src/ui/CombinationPanel.cpp
    src/ui/DynamicInputPanel.cpp
    src/ui/EstimateResultPanel.cpp
    src/ui/MainWindow.cpp
    src/ui/RoleSelectorWidget.cpp
    src/ui/ResultPanel.cpp
    src/utils/Logger.cpp
    src/utils/StringUtils.cpp
    src/vision/GridRecognizer.cpp
    src/vision/ImagePreprocessor.cpp
    src/vision/QualityRecognizer.cpp
    src/vision/ScreenCapture.cpp
    src/vision/ShapeRecognizer.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(nlohmann_json_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
else()
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/minijson)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()

if(OpenCV_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
    target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE BBAE_HAS_OPENCV=1)
else()
    message(WARNING "OpenCV was not found. Vision stubs will compile without OpenCV-backed implementation.")
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/data
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
)

enable_testing()

add_executable(test_item_database
    tests/test_item_database.cpp
    src/data/ConfigLoader.cpp
    src/data/ItemDatabase.cpp
)
target_include_directories(test_item_database PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(nlohmann_json_FOUND)
    target_link_libraries(test_item_database PRIVATE nlohmann_json::nlohmann_json)
else()
    target_include_directories(test_item_database PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/minijson)
endif()
if(MSVC)
    target_compile_options(test_item_database PRIVATE /utf-8)
endif()
add_test(NAME test_item_database COMMAND test_item_database)

add_executable(test_estimator
    tests/test_estimator.cpp
    src/core/Estimator.cpp
    src/data/ConfigLoader.cpp
    src/data/ItemDatabase.cpp
)
target_include_directories(test_estimator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(nlohmann_json_FOUND)
    target_link_libraries(test_estimator PRIVATE nlohmann_json::nlohmann_json)
else()
    target_include_directories(test_estimator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/minijson)
endif()
if(MSVC)
    target_compile_options(test_estimator PRIVATE /utf-8)
endif()
add_test(NAME test_estimator COMMAND test_estimator)

add_executable(test_combination_solver
    tests/test_combination_solver.cpp
    src/core/CombinationSolver.cpp
    src/data/ConfigLoader.cpp
    src/data/ItemDatabase.cpp
)
target_include_directories(test_combination_solver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(nlohmann_json_FOUND)
    target_link_libraries(test_combination_solver PRIVATE nlohmann_json::nlohmann_json)
else()
    target_include_directories(test_combination_solver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/minijson)
endif()
if(MSVC)
    target_compile_options(test_combination_solver PRIVATE /utf-8)
endif()
add_test(NAME test_combination_solver COMMAND test_combination_solver)

add_executable(test_price_calculator
    tests/test_price_calculator.cpp
    src/core/CombinationSolver.cpp
    src/core/PriceCalculator.cpp
    src/data/ConfigLoader.cpp
    src/data/ItemDatabase.cpp
)
target_include_directories(test_price_calculator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(nlohmann_json_FOUND)
    target_link_libraries(test_price_calculator PRIVATE nlohmann_json::nlohmann_json)
else()
    target_include_directories(test_price_calculator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/minijson)
endif()
if(MSVC)
    target_compile_options(test_price_calculator PRIVATE /utf-8)
endif()
add_test(NAME test_price_calculator COMMAND test_price_calculator)

add_executable(test_role_config_loader
    tests/test_role_config_loader.cpp
    src/data/ConfigLoader.cpp
    src/data/RoleConfigLoader.cpp
)
target_include_directories(test_role_config_loader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(nlohmann_json_FOUND)
    target_link_libraries(test_role_config_loader PRIVATE nlohmann_json::nlohmann_json)
else()
    target_include_directories(test_role_config_loader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/minijson)
endif()
if(MSVC)
    target_compile_options(test_role_config_loader PRIVATE /utf-8)
endif()
add_test(NAME test_role_config_loader COMMAND test_role_config_loader)

add_executable(test_auction_advisor
    tests/test_auction_advisor.cpp
    src/core/AuctionAdvisor.cpp
)
target_include_directories(test_auction_advisor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(MSVC)
    target_compile_options(test_auction_advisor PRIVATE /utf-8)
endif()
add_test(NAME test_auction_advisor COMMAND test_auction_advisor)

add_executable(test_quality_recognizer
    tests/test_quality_recognizer.cpp
    src/vision/QualityRecognizer.cpp
)
target_include_directories(test_quality_recognizer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(MSVC)
    target_compile_options(test_quality_recognizer PRIVATE /utf-8)
endif()
add_test(NAME test_quality_recognizer COMMAND test_quality_recognizer)
