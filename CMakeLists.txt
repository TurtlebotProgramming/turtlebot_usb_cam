cmake_minimum_required(VERSION 3.10)
project(turtlebot_usb_cam)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(include ${OpenCV_INCLUDE_DIRS})

add_executable(usb_cam_node
  src/usb_cam_node.cpp
  src/usb_camera.cpp
)

ament_target_dependencies(usb_cam_node
  rclcpp
  sensor_msgs
  cv_bridge
)

target_link_libraries(usb_cam_node
  ${OpenCV_LIBRARIES}
)

install(TARGETS usb_cam_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY config launch
  DESTINATION share/${PROJECT_NAME}
  OPTIONAL
)

ament_package()
