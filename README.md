# kingspider-ros

ROS1 (Noetic) package for controlling the KingSpider hexapod via serial and gamepad.
The catkin package name is `kingspider`.

## Build

```bash
cd ~/catkin_ws/src
git clone https://github.com/Harlequinade1123/kingspider-ros.git
cd .. && catkin build kingspider && source devel/setup.bash
```

## Run

```bash
roslaunch kingspider one_robot_one_joy.launch
```

Other launch files: `one_robot`, `two_robots`, `two_robots_one_joy`, `two_robots_two_joy`.

## Firmware

OpenRB-150 firmware is in [`firmware/openrb`](firmware/openrb).

## Related

- [kingspider](https://github.com/Harlequinade1123/kingspider): Processing controller and Arduino firmware

## License

MIT
