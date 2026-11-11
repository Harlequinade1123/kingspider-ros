#include <ros/ros.h>
#include <sensor_msgs/Joy.h>
#include <geometry_msgs/Twist.h>

class JoyToJoys
{
public:
  JoyToJoys()
  {
    // Publisher
    pub1_ = nh_.advertise<sensor_msgs::Joy>("joy1", 10);
    pub2_ = nh_.advertise<sensor_msgs::Joy>("joy2", 10);

    // Subscriber
    sub_ = nh_.subscribe("joy", 10, &JoyToJoys::joyCallback, this);
  }

private:
  void joyCallback(const sensor_msgs::Joy::ConstPtr &joy)
  {
    // --- joy1: 元のデータをそのまま転送 ---
    sensor_msgs::Joy joy1 = *joy;

    // --- joy2: 右スティック(axes[3], axes[4])を左スティック(axes[0], axes[1])に置換 ---
    sensor_msgs::Joy joy2 = *joy;

    if (joy->axes.size() >= 5)  // 安全チェック
    {
      // 右スティックの値を上書きして左スティック
      joy2.axes[3] = joy->axes[0];  // 左スティックX に右スティックX
      joy2.axes[4] = joy->axes[1];  // 左スティックY に右スティックY

      joy1.axes[0] = 0.0;
      joy1.axes[1] = abs(std::min(0.0, joy->axes[5]-0.5));
      joy2.axes[0] = 0.0;
      joy2.axes[1] = abs(std::min(0.0, joy->axes[2]-0.5));
    }
    else
    {
      ROS_WARN_THROTTLE(1.0, "Joy message has fewer axes than expected.");
    }

    // Publish
    pub1_.publish(joy1);
    pub2_.publish(joy2);
  }

  ros::NodeHandle nh_;
  ros::Publisher pub1_, pub2_;
  ros::Subscriber sub_;
};

int main(int argc, char **argv)
{
  ros::init(argc, argv, "joy_to_joys_pub");
  JoyToJoys joy_to_joys;
  ros::spin();
  return 0;
}
