cmake_minimum_required(VERSION 3.16)
project(rtswe_eval LANGUAGES CXX)

# Hidden tests and probes for RT-SWE-Bench. Copied into /ws/src at evaluation
# time only; never visible to the agent.

find_package(ament_cmake REQUIRED)

if(BUILD_TESTING)
  find_package(ament_cmake_gmock REQUIRED)
  find_package(generate_parameter_library REQUIRED)  # add_rostest_with_parameters_gmock
  find_package(malloc_guard REQUIRED)
  find_package(controller_interface REQUIRED)
  find_package(hardware_interface REQUIRED)
  find_package(rclcpp REQUIRED)
  find_package(forward_command_controller REQUIRED)
  find_package(pid_controller REQUIRED)
  find_package(diff_drive_controller REQUIRED)
  find_package(omni_wheel_drive_controller REQUIRED)
  find_package(mecanum_drive_controller REQUIRED)
  find_package(steering_controllers_library REQUIRED)
  find_package(admittance_controller REQUIRED)
  find_package(joint_trajectory_controller REQUIRED)
  find_package(joint_state_broadcaster REQUIRED)
  find_package(gpio_controllers REQUIRED)
  find_package(ros2_control_test_assets REQUIRED)

  # Upstream test fixtures are reused as-is from the source tree in the image.
  set(ROS2_CONTROLLERS_SRC "/ws/src/ros2_controllers" CACHE PATH
    "ros2_controllers source tree, for upstream test fixtures and parameter files")

  # RTSWE_ONLY: optional list of probe targets to build (evaluation builds one).
  set(RTSWE_ONLY "" CACHE STRING "Build only these probe targets (default: all)")
  function(rtswe_selected target out)
    if(NOT RTSWE_ONLY OR target IN_LIST RTSWE_ONLY)
      set(${out} TRUE PARENT_SCOPE)
    else()
      set(${out} FALSE PARENT_SCOPE)
    endif()
  endfunction()

  # malloc_guard.h needs C++20; the controller libraries stay as built upstream.
  function(rtswe_probe_common target)
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_include_directories(${target} PRIVATE include)
    target_link_libraries(${target}
      controller_interface::controller_interface
      hardware_interface::hardware_interface
      rclcpp::rclcpp
      malloc_guard::malloc_guard
      ${CMAKE_DL_LIBS}
    )
  endfunction()

  rtswe_selected(probe_forward_command_controller _sel)
  if(_sel)
    ament_add_gmock(probe_forward_command_controller
      test/probe_forward_command_controller.cpp
      TIMEOUT 120
    )
    rtswe_probe_common(probe_forward_command_controller)
    target_link_libraries(probe_forward_command_controller
      forward_command_controller::forward_command_controller)
  endif()

  rtswe_selected(probe_pid_controller _sel)
  if(_sel)
    add_rostest_with_parameters_gmock(probe_pid_controller
      test/probe_pid_controller.cpp
      ${ROS2_CONTROLLERS_SRC}/pid_controller/test/pid_controller_params.yaml
    )
    rtswe_probe_common(probe_pid_controller)
    target_include_directories(probe_pid_controller PRIVATE
      ${ROS2_CONTROLLERS_SRC}/pid_controller/test)
    target_link_libraries(probe_pid_controller pid_controller::pid_controller)
  endif()

  rtswe_selected(probe_diff_drive_controller _sel)
  if(_sel)
    ament_add_gmock(probe_diff_drive_controller test/probe_diff_drive_controller.cpp TIMEOUT 120)
    rtswe_probe_common(probe_diff_drive_controller)
    target_link_libraries(probe_diff_drive_controller
      diff_drive_controller::diff_drive_controller)
  endif()

  rtswe_selected(probe_omni_wheel_drive_controller _sel)
  if(_sel)
    ament_add_gmock(probe_omni_wheel_drive_controller
      test/probe_omni_wheel_drive_controller.cpp TIMEOUT 120)
    rtswe_probe_common(probe_omni_wheel_drive_controller)
    target_include_directories(probe_omni_wheel_drive_controller PRIVATE
      ${ROS2_CONTROLLERS_SRC}/omni_wheel_drive_controller/test)
    target_link_libraries(probe_omni_wheel_drive_controller
      omni_wheel_drive_controller::omni_wheel_drive_controller)
  endif()

  rtswe_selected(probe_mecanum_drive_controller _sel)
  if(_sel)
    add_rostest_with_parameters_gmock(probe_mecanum_drive_controller
      test/probe_mecanum_drive_controller.cpp
      ${ROS2_CONTROLLERS_SRC}/mecanum_drive_controller/test/mecanum_drive_controller_params.yaml
    )
    rtswe_probe_common(probe_mecanum_drive_controller)
    target_include_directories(probe_mecanum_drive_controller PRIVATE
      ${ROS2_CONTROLLERS_SRC}/mecanum_drive_controller/test)
    target_link_libraries(probe_mecanum_drive_controller
      mecanum_drive_controller::mecanum_drive_controller)
  endif()

  rtswe_selected(probe_steering_controllers_library _sel)
  if(_sel)
    add_rostest_with_parameters_gmock(probe_steering_controllers_library
      test/probe_steering_controllers_library.cpp
      ${ROS2_CONTROLLERS_SRC}/steering_controllers_library/test/steering_controllers_library_params.yaml
    )
    rtswe_probe_common(probe_steering_controllers_library)
    target_include_directories(probe_steering_controllers_library PRIVATE
      ${ROS2_CONTROLLERS_SRC}/steering_controllers_library/test)
    target_link_libraries(probe_steering_controllers_library
      steering_controllers_library::steering_controllers_library)
  endif()

  rtswe_selected(task_rt_002_pid _sel)
  if(_sel)
    add_rostest_with_parameters_gmock(task_rt_002_pid
      test/task_rt_002_pid.cpp
      ${ROS2_CONTROLLERS_SRC}/pid_controller/test/pid_controller_params.yaml
    )
    rtswe_probe_common(task_rt_002_pid)
    target_include_directories(task_rt_002_pid PRIVATE ${ROS2_CONTROLLERS_SRC}/pid_controller/test)
    target_link_libraries(task_rt_002_pid pid_controller::pid_controller)
  endif()

  rtswe_selected(task_rt_003_steering _sel)
  if(_sel)
    add_rostest_with_parameters_gmock(task_rt_003_steering
      test/task_rt_003_steering.cpp
      ${ROS2_CONTROLLERS_SRC}/steering_controllers_library/test/steering_controllers_library_params.yaml
    )
    rtswe_probe_common(task_rt_003_steering)
    target_include_directories(task_rt_003_steering PRIVATE
      ${ROS2_CONTROLLERS_SRC}/steering_controllers_library/test)
    target_link_libraries(task_rt_003_steering
      steering_controllers_library::steering_controllers_library)
  endif()

  rtswe_selected(task_rt_004_omni_tf _sel)
  if(_sel)
    ament_add_gmock(task_rt_004_omni_tf test/task_rt_004_omni_tf.cpp TIMEOUT 120)
    rtswe_probe_common(task_rt_004_omni_tf)
    target_include_directories(task_rt_004_omni_tf PRIVATE
      ${ROS2_CONTROLLERS_SRC}/omni_wheel_drive_controller/test)
    target_link_libraries(task_rt_004_omni_tf
      omni_wheel_drive_controller::omni_wheel_drive_controller)
  endif()

  rtswe_selected(task_rt_005_omni_odom _sel)
  if(_sel)
    ament_add_gmock(task_rt_005_omni_odom test/task_rt_005_omni_odom.cpp TIMEOUT 120)
    rtswe_probe_common(task_rt_005_omni_odom)
    target_include_directories(task_rt_005_omni_odom PRIVATE
      ${ROS2_CONTROLLERS_SRC}/omni_wheel_drive_controller/test)
    target_link_libraries(task_rt_005_omni_odom
      omni_wheel_drive_controller::omni_wheel_drive_controller)
  endif()

  rtswe_selected(task_rt_006_mecanum_zero _sel)
  if(_sel)
    add_rostest_with_parameters_gmock(task_rt_006_mecanum_zero
      test/task_rt_006_mecanum_zero.cpp
      ${ROS2_CONTROLLERS_SRC}/mecanum_drive_controller/test/mecanum_drive_controller_params.yaml
    )
    rtswe_probe_common(task_rt_006_mecanum_zero)
    target_include_directories(task_rt_006_mecanum_zero PRIVATE
      ${ROS2_CONTROLLERS_SRC}/mecanum_drive_controller/test)
    target_link_libraries(task_rt_006_mecanum_zero
      mecanum_drive_controller::mecanum_drive_controller)
  endif()

  rtswe_selected(task_rt_007_diff_drive_history _sel)
  if(_sel)
    ament_add_gmock(task_rt_007_diff_drive_history test/task_rt_007_diff_drive_history.cpp TIMEOUT 120)
    rtswe_probe_common(task_rt_007_diff_drive_history)
    target_link_libraries(task_rt_007_diff_drive_history
      diff_drive_controller::diff_drive_controller)
  endif()

  rtswe_selected(task_rt_008_diff_drive_open_loop _sel)
  if(_sel)
    ament_add_gmock(task_rt_008_diff_drive_open_loop test/task_rt_008_diff_drive_open_loop.cpp TIMEOUT 120)
    rtswe_probe_common(task_rt_008_diff_drive_open_loop)
    target_link_libraries(task_rt_008_diff_drive_open_loop
      diff_drive_controller::diff_drive_controller)
  endif()

  rtswe_selected(probe_admittance_controller _sel)
  if(_sel)
    add_rostest_with_parameters_gmock(probe_admittance_controller
      test/probe_admittance_controller.cpp
      ${ROS2_CONTROLLERS_SRC}/admittance_controller/test/test_params.yaml
    )
    rtswe_probe_common(probe_admittance_controller)
    target_include_directories(probe_admittance_controller PRIVATE
      ${ROS2_CONTROLLERS_SRC}/admittance_controller/test)
    target_link_libraries(probe_admittance_controller
      admittance_controller::admittance_controller
      ros2_control_test_assets::ros2_control_test_assets)
  endif()

  rtswe_selected(task_rt_009_diff_drive_frame _sel)
  if(_sel)
    ament_add_gmock(task_rt_009_diff_drive_frame test/task_rt_009_diff_drive_frame.cpp TIMEOUT 120)
    rtswe_probe_common(task_rt_009_diff_drive_frame)
    target_link_libraries(task_rt_009_diff_drive_frame
      diff_drive_controller::diff_drive_controller)
  endif()

  rtswe_selected(probe_joint_trajectory_controller _sel)
  if(_sel)
    ament_add_gmock(probe_joint_trajectory_controller
      test/probe_joint_trajectory_controller.cpp TIMEOUT 240)
    rtswe_probe_common(probe_joint_trajectory_controller)
    target_include_directories(probe_joint_trajectory_controller PRIVATE
      ${ROS2_CONTROLLERS_SRC}/joint_trajectory_controller/test)
    target_link_libraries(probe_joint_trajectory_controller
      joint_trajectory_controller::joint_trajectory_controller
      ros2_control_test_assets::ros2_control_test_assets)
  endif()

  rtswe_selected(task_rt_010_jtc_sample _sel)
  if(_sel)
    ament_add_gmock(task_rt_010_jtc_sample test/task_rt_010_jtc_sample.cpp TIMEOUT 240)
    rtswe_probe_common(task_rt_010_jtc_sample)
    target_include_directories(task_rt_010_jtc_sample PRIVATE
      ${ROS2_CONTROLLERS_SRC}/joint_trajectory_controller/test)
    target_link_libraries(task_rt_010_jtc_sample
      joint_trajectory_controller::joint_trajectory_controller
      ros2_control_test_assets::ros2_control_test_assets)
  endif()

  rtswe_selected(task_rt_014_jtc_skip _sel)
  if(_sel)
    ament_add_gmock(task_rt_014_jtc_skip test/task_rt_014_jtc_skip.cpp TIMEOUT 240)
    rtswe_probe_common(task_rt_014_jtc_skip)
    target_include_directories(task_rt_014_jtc_skip PRIVATE
      ${ROS2_CONTROLLERS_SRC}/joint_trajectory_controller/test)
    target_link_libraries(task_rt_014_jtc_skip
      joint_trajectory_controller::joint_trajectory_controller
      ros2_control_test_assets::ros2_control_test_assets)
  endif()

  rtswe_selected(task_rt_015_forward_nan _sel)
  if(_sel)
    ament_add_gmock(task_rt_015_forward_nan test/task_rt_015_forward_nan.cpp TIMEOUT 120)
    rtswe_probe_common(task_rt_015_forward_nan)
    target_link_libraries(task_rt_015_forward_nan
      forward_command_controller::forward_command_controller)
  endif()

  rtswe_selected(task_rt_016_forward_order _sel)
  if(_sel)
    ament_add_gmock(task_rt_016_forward_order test/task_rt_016_forward_order.cpp TIMEOUT 120)
    rtswe_probe_common(task_rt_016_forward_order)
    target_link_libraries(task_rt_016_forward_order
      forward_command_controller::forward_command_controller)
  endif()

  rtswe_selected(task_rt_011_pid_params _sel)
  if(_sel)
    add_rostest_with_parameters_gmock(task_rt_011_pid_params
      test/task_rt_011_pid_params.cpp
      ${ROS2_CONTROLLERS_SRC}/pid_controller/test/pid_controller_params.yaml
    )
    rtswe_probe_common(task_rt_011_pid_params)
    target_include_directories(task_rt_011_pid_params PRIVATE ${ROS2_CONTROLLERS_SRC}/pid_controller/test)
    target_link_libraries(task_rt_011_pid_params pid_controller::pid_controller)
  endif()

  rtswe_selected(task_rt_012_steering_nan _sel)
  if(_sel)
    add_rostest_with_parameters_gmock(task_rt_012_steering_nan
      test/task_rt_012_steering_nan.cpp
      ${ROS2_CONTROLLERS_SRC}/steering_controllers_library/test/steering_controllers_library_params.yaml
    )
    rtswe_probe_common(task_rt_012_steering_nan)
    target_include_directories(task_rt_012_steering_nan PRIVATE
      ${ROS2_CONTROLLERS_SRC}/steering_controllers_library/test)
    target_link_libraries(task_rt_012_steering_nan
      steering_controllers_library::steering_controllers_library)
  endif()

  rtswe_selected(probe_joint_state_broadcaster _sel)
  if(_sel)
    ament_add_gmock(probe_joint_state_broadcaster test/probe_joint_state_broadcaster.cpp TIMEOUT 120)
    rtswe_probe_common(probe_joint_state_broadcaster)
    target_link_libraries(probe_joint_state_broadcaster
      joint_state_broadcaster::joint_state_broadcaster)
  endif()

  rtswe_selected(probe_gpio_command_controller _sel)
  if(_sel)
    ament_add_gmock(probe_gpio_command_controller test/probe_gpio_command_controller.cpp TIMEOUT 120)
    rtswe_probe_common(probe_gpio_command_controller)
    # gpio_controllers exports variables, not a namespaced target
    target_include_directories(probe_gpio_command_controller PRIVATE ${gpio_controllers_INCLUDE_DIRS})
    target_link_libraries(probe_gpio_command_controller ${gpio_controllers_LIBRARIES})
  endif()

  rtswe_selected(task_rt_013_gpio _sel)
  if(_sel)
    ament_add_gmock(task_rt_013_gpio test/task_rt_013_gpio.cpp TIMEOUT 120)
    rtswe_probe_common(task_rt_013_gpio)
    target_include_directories(task_rt_013_gpio PRIVATE ${gpio_controllers_INCLUDE_DIRS})
    target_link_libraries(task_rt_013_gpio ${gpio_controllers_LIBRARIES})
  endif()
endif()

ament_package()
