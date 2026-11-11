# kingspider-ros

ROS package for controlling the KingSpider hexapod via serial and gamepad.

## Supported ROS

| Branch | ROS | OS | Status |
|---|---|---|---|
| `main` | Noetic (ROS1) | Ubuntu 20.04 | current |

## Related repositories

- [kingspider](https://github.com/Harlequinade1123/kingspider): Processing controller and Arduino firmware

## Build

```bash
cd ~/catkin_ws/src
git clone https://github.com/Harlequinade1123/kingspider-ros.git
cd ..
rosdep install --from-paths src --ignore-src -y
catkin build kingspider      # or catkin_make
source devel/setup.bash
```

The ROS package name is `kingspider` (catkin does not allow hyphens).

## Nodes

| Executable | Source | Description |
|---|---|---|
| `kingspider_node` | `src/main_param.cpp` | Serial control node |
| `kingspider_joy_node` | `src/main_param_joy.cpp` | Gamepad (`sensor_msgs/Joy`) driven control node |
| `joy_to_joys_node` | `src/joy_to_joys.cpp` | Splits one `/joy` into `/joy1` and `/joy2` |
| `joy_to_twist_node` | `src/joy_to_twist_multi.cpp` | `Joy` to `Twist` conversion |

### Parameters (`kingspider_joy_node`)

| Name | Default | Description |
|---|---|---|
| `~joy_topic` | `/joy` | Input `sensor_msgs/Joy` topic |
| `~device_name` | `/dev/ttyUSB0` | Serial device |
| `~baudrate` | `57600` | Serial baudrate |

## Launch

```bash
roslaunch kingspider one_robot_one_joy.launch
roslaunch kingspider two_robots_two_joy.launch
```

Available: `one_robot`, `one_robot_one_joy`, `two_robots`, `two_robots_one_joy`, `two_robots_two_joy`.

## License

MIT
