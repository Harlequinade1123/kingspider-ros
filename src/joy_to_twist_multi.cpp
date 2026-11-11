#include <ros/ros.h>
#include <sensor_msgs/Joy.h>
#include <geometry_msgs/Twist.h>

class JoyToTwist
{
public:
  JoyToTwist()
  {
    // Publisher
    pub1_ = nh_.advertise<geometry_msgs::Twist>("cmd_vel1", 10);
    pub2_ = nh_.advertise<geometry_msgs::Twist>("cmd_vel2", 10);

    // Subscriber
    sub_ = nh_.subscribe("joy", 10, &JoyToTwist::joyCallback, this);

    // パラメータ取得（任意）
    nh_.param("scale_linear",  scale_linear_, 2.0);
  }

private:
  void joyCallback(const sensor_msgs::Joy::ConstPtr &joy)
  {
    geometry_msgs::Twist cmd1, cmd2;

    // 軸入力をTwistに変換
    cmd1.angular.z = scale_linear_ * joy->axes[0];
    cmd1.linear.x  = scale_linear_ * joy->axes[1];

    cmd2.angular.z = scale_linear_ * joy->axes[3];
    cmd2.linear.x  = scale_linear_ * joy->axes[4];

    pub1_.publish(cmd1);
    pub2_.publish(cmd2);
  }

  ros::NodeHandle nh_;
  ros::Publisher pub1_, pub2_;
  ros::Subscriber sub_;

  double scale_linear_;
};

int main(int argc, char **argv)
{
  ros::init(argc, argv, "joy_to_twist_pub");
  JoyToTwist joy_to_twist;
  ros::spin();
  return 0;
}
