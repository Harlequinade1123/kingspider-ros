#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/Int16.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

bool running                = false;
bool robot_is_initialized   = false;
bool robot_is_walking_right = false;
bool robot_is_walking_left  = false;
bool robot_is_turning_right = false;
bool robot_is_turning_left  = false;
int current_speed           = 300;
int fd;

std::mutex mtx;

int open_serial(const char *device_name);
void planMotion(bool up, bool down, bool right, bool left, int cmd_speed);
void initialize();
void walkstart();
void walkRight();
void walkLeft();
void walkEndRight();
void walkEndLeft();
void turnRight();
void turnLeft();
void perform();
void commandWrite(std::string command);
void speedCommandWrite(int cmd_speed);
int  openSerial(const char *device_name, int baudrate_int);
bool tryConvertingBaudrate(unsigned int &BAUDRATE, int baudrate_int);

class KingspiderNode
{
    public:
        KingspiderNode();
        ~KingspiderNode();
        void callback(const geometry_msgs::TwistConstPtr &twist_msg);
        void intCallback(const std_msgs::Int16 &int_msg);
        void update();
        void publish();

        bool up;
        bool down;
        bool right;
        bool left;
        int speed;

    private:
        ros::NodeHandle nh_;
        ros::NodeHandle param_nh_;
        ros::Subscriber mobile_twist_sub_;
        ros::Subscriber mobile_int_sub_;
};

KingspiderNode::KingspiderNode() : nh_(""), param_nh_("~")
{
    up    = false;
    down  = false;
    right = false;
    left  = false;
    speed = 300;
    mobile_twist_sub_ = nh_.subscribe("/cmd_vel", 100, &KingspiderNode::callback,    this);
    mobile_int_sub_   = nh_.subscribe("/slider",  100, &KingspiderNode::intCallback, this);
}

KingspiderNode::~KingspiderNode()
{}

void KingspiderNode::callback(const geometry_msgs::TwistConstPtr &twist_msg)
{
    std::lock_guard lock{mtx};
    double x = twist_msg->angular.z;
    double y = twist_msg->linear.x;
    up    = false;
    down  = false;
    right = false;
    left  = false;
    if (1 < y)
    {
        up = true;
    }
    else if (y < -1)
    {
        down = true;
    }
    else if (1 < x)
    {
        left = true;
    }
    else if (x < -1)
    {
        right = true;
    }
}

void KingspiderNode::intCallback(const std_msgs::Int16 &int_msg)
{
    std::lock_guard lock{mtx};
    speed = int_msg.data;
}

int main(int argc, char** argv)
{
    char device_name[]="/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_AG0K43TG-if00-port0";
    fd = openSerial(device_name, 57600);

    ros::init(argc, argv, "king_spider_node");
    KingspiderNode spider;
    printf("START: king spider Node\n");

    ros::AsyncSpinner spinner(2);
    spinner.start();
    ros::Rate rate(100);

    while (ros::ok())
    {
        bool tmp_up, tmp_down, tmp_right, tmp_left;
        int tmp_speed;
        std::unique_lock lock{mtx};
        // if (spider.up)
        // {
        //     printf("UP ");
        // }
        // if (spider.down)
        // {
        //     printf("DOWN ");
        // }
        // if (spider.right)
        // {
        //     printf("RIGHT ");
        // }
        // if (spider.left)
        // {
        //     printf("LEFT ");
        // }
        // if (spider.up || spider.down || spider.right || spider.left)
        // {
        //     printf("\n");
        // }
        tmp_up    = spider.up;
        tmp_down  = spider.down;
        tmp_right = spider.right;
        tmp_left  = spider.left;
        tmp_speed = spider.speed;
        lock.unlock();
        planMotion(tmp_up, tmp_down, tmp_right, tmp_left, tmp_speed);
        rate.sleep();
    }
    spinner.stop();
    
    return 0;
}

void planMotion(bool up, bool down, bool right, bool left, int cmd_speed)
{
    if (running)
    {
        return;
    }
    else
    {
        running = false;
    }

    speedCommandWrite(cmd_speed);
    if (!robot_is_initialized)
    {
        initialize();
        robot_is_initialized = true;
    }
    else if (up && !robot_is_walking_right && !robot_is_walking_left)
    {
        walkstart();
        robot_is_walking_right = true;
    }
    else if (up && !robot_is_walking_right && robot_is_walking_left)
    {
        walkRight();
        robot_is_walking_right = true;
        robot_is_walking_left  = false;
        return;
    }
    else if (up && robot_is_walking_right && !robot_is_walking_left)
    {
        walkLeft();
        robot_is_walking_right = false;
        robot_is_walking_left  = true;
    }
    else if (!up && !robot_is_walking_right && robot_is_walking_left)
    {
        walkEndRight();
        robot_is_walking_right = false;
        robot_is_walking_left  = false;
    }
    else if (!up && robot_is_walking_right && !robot_is_walking_left)
    {
        walkEndLeft();
        robot_is_walking_right = false;
        robot_is_walking_left  = false;
    }
    else if (right && !robot_is_walking_right && !robot_is_walking_left)
    {
        turnRight();
    }
    else if (left && !robot_is_walking_right && !robot_is_walking_left)
    {
        turnLeft();
    }
    else if (down && !robot_is_walking_right && !robot_is_walking_right)
    {
        perform();
    }

    running = false;
}

void initialize()
{
    commandWrite("[a,512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,512]\n");
    commandWrite("[a,477,546,471,552,461,562,512,512,551,472,555,468,546,477,552,471,572,461]\n");
    commandWrite("[a,442,581,433,590,418,605,512,512,589,434,594,429,583,442,591,433,623,418]\n");
    commandWrite("[a,407,616,395,628,382,641,512,512,627,396,631,392,622,407,629,395,667,382]\n");
    commandWrite("[a,372,651,358,665,354,669,512,512,665,358,665,358,661,372,668,358,704,354]\n");
    commandWrite("[a,338,685,321,702,332,691,512,512,703,320,697,326,700,338,707,321,734,332]\n");
}

void walkstart()
{
    commandWrite("[a,338,685,321,702,332,691,512,512,703,320,697,326,700,338,707,321,734,332]\n");
    commandWrite("[a,338,685,321,702,332,691,512,512,703,320,697,326,700,338,707,321,734,332]\n");
    commandWrite("[a,342,689,298,691,325,693,501,501,690,294,686,306,705,342,734,336,766,356]\n");
    commandWrite("[a,346,694,276,680,321,695,491,491,677,268,675,287,710,346,761,351,798,381]\n");
    commandWrite("[a,350,698,256,668,318,697,481,481,664,243,663,270,716,350,790,367,829,406]\n");
    commandWrite("[a,353,703,236,655,317,697,471,471,651,217,650,254,722,353,819,383,859,433]\n");
    commandWrite("[a,357,709,218,642,318,697,461,461,639,191,637,239,728,357,850,400,889,461]\n");
    commandWrite("[a,360,714,248,654,351,719,451,451,651,218,646,258,735,360,819,393,880,466]\n");
    commandWrite("[a,363,720,278,666,384,740,441,441,663,244,655,278,742,363,786,386,870,473]\n");
    commandWrite("[a,367,727,308,678,420,761,431,431,675,271,663,299,749,367,753,380,858,480]\n");
    commandWrite("[a,370,733,339,689,457,782,422,422,687,297,670,321,757,370,721,375,844,488]\n");
    commandWrite("[a,372,740,370,700,497,802,412,412,699,323,677,345,766,372,689,370,828,497]\n");
}

void walkRight()
{
    commandWrite("[a,280,649,321,651,220,524,610,610,699,322,677,344,659,280,674,321,586,220]\n");
    commandWrite("[a,280,649,321,651,220,524,610,610,699,322,677,344,659,280,674,321,586,220]\n");
    commandWrite("[a,293,654,289,653,216,552,590,590,687,295,673,317,665,293,708,330,639,249]\n");
    commandWrite("[a,306,661,259,653,217,577,571,571,676,268,668,292,672,306,742,341,689,280]\n");
    commandWrite("[a,317,667,233,650,221,599,551,551,664,241,661,270,680,317,775,353,737,313]\n");
    commandWrite("[a,327,675,210,645,228,618,531,530,651,214,652,251,689,327,808,367,783,348]\n");
    commandWrite("[a,336,683,190,637,240,635,510,510,639,188,641,234,698,336,842,383,826,385]\n");
    commandWrite("[a,344,692,225,653,285,672,490,490,651,215,651,251,709,344,817,375,834,401]\n");
    commandWrite("[a,352,702,260,667,333,707,470,470,663,241,660,271,721,352,789,370,838,421]\n");
    commandWrite("[a,359,713,295,679,384,740,450,450,675,268,667,293,733,359,758,367,838,443]\n");
    commandWrite("[a,365,725,331,689,437,772,430,430,687,295,672,317,748,365,724,367,835,468]\n");
    commandWrite("[a,371,738,369,699,496,801,411,411,698,322,676,345,764,371,689,369,827,496]\n");
}

void walkLeft()
{
    commandWrite("[a,372,740,370,700,497,802,412,412,699,323,677,345,766,372,689,370,828,497]\n");
    commandWrite("[a,372,740,370,700,497,802,412,412,699,323,677,345,766,372,689,370,828,497]\n");
    commandWrite("[a,366,726,367,731,468,805,431,431,725,334,704,348,749,366,682,331,801,438]\n");
    commandWrite("[a,359,714,368,761,444,804,450,450,752,345,728,353,735,359,674,295,773,384]\n");
    commandWrite("[a,352,703,370,787,422,800,470,470,779,357,751,360,722,352,665,260,743,334]\n");
    commandWrite("[a,345,693,376,810,402,793,490,490,806,370,770,369,710,345,654,225,711,286]\n");
    commandWrite("[a,336,684,383,830,386,781,511,511,832,382,788,379,699,336,642,191,677,240]\n");
    commandWrite("[a,327,676,367,795,348,736,531,531,806,370,771,369,690,327,653,210,664,229]\n");
    commandWrite("[a,317,668,353,761,313,688,551,551,779,357,751,360,681,317,661,233,649,221]\n");
    commandWrite("[a,306,661,341,726,280,637,571,571,752,345,729,353,673,306,668,260,630,217]\n");
    commandWrite("[a,294,655,330,689,249,583,591,591,725,334,704,347,666,294,672,289,610,217]\n");
    commandWrite("[a,280,649,321,651,220,524,610,610,699,322,677,344,659,280,674,321,586,220]\n");
}

void walkEndRight()
{
    commandWrite("[a,280,649,321,651,220,524,610,610,699,322,677,344,659,280,674,321,586,220]\n");
    commandWrite("[a,280,649,321,651,220,524,610,610,699,322,677,344,659,280,674,321,586,220]\n");
    commandWrite("[a,287,651,290,646,207,533,600,600,687,295,670,320,662,287,703,332,623,240]\n");
    commandWrite("[a,293,654,260,641,195,541,590,590,675,269,662,298,665,293,733,343,658,260]\n");
    commandWrite("[a,300,657,231,635,186,548,580,580,663,243,654,277,669,300,762,355,692,281]\n");
    commandWrite("[a,306,661,202,628,178,554,571,571,650,216,646,257,672,306,792,367,724,303]\n");
    commandWrite("[a,311,664,174,621,173,560,561,561,638,189,636,238,676,311,822,379,755,325]\n");
    commandWrite("[a,317,667,205,637,203,588,551,551,651,215,649,253,680,317,801,366,754,324]\n");
    commandWrite("[a,322,671,235,654,233,614,541,541,664,241,662,269,684,322,779,353,751,324]\n");
    commandWrite("[a,327,675,263,669,265,640,531,530,677,267,674,286,689,327,755,341,746,326]\n");
    commandWrite("[a,331,679,292,685,297,665,520,520,689,292,685,305,694,331,731,330,740,328]\n");
    commandWrite("[a,336,683,320,700,331,689,510,510,702,318,695,325,698,336,706,320,732,331]\n");
}

void walkEndLeft()
{
    commandWrite("[a,372,740,370,700,497,802,412,412,699,323,677,345,766,372,689,370,828,497]\n");
    commandWrite("[a,372,740,370,700,497,802,412,412,699,323,677,345,766,372,689,370,828,497]\n");
    commandWrite("[a,369,733,374,730,487,814,421,421,724,334,700,351,757,369,679,338,809,457]\n");
    commandWrite("[a,366,726,379,760,479,826,431,431,751,346,723,358,749,366,669,308,789,419]\n");
    commandWrite("[a,362,720,385,789,472,835,440,441,777,358,744,366,742,362,658,278,770,384]\n");
    commandWrite("[a,359,714,392,818,466,843,450,450,804,371,764,375,735,359,648,248,750,350]\n");
    commandWrite("[a,356,708,400,846,460,849,460,460,831,383,782,384,728,356,637,217,729,318]\n");
    commandWrite("[a,352,703,383,815,433,819,470,470,805,370,768,371,722,352,652,236,731,317]\n");
    commandWrite("[a,348,698,366,786,406,788,480,480,779,357,752,359,716,348,666,255,733,318]\n");
    commandWrite("[a,345,693,351,757,380,756,490,490,753,344,735,347,710,345,680,276,733,321]\n");
    commandWrite("[a,341,688,335,729,355,724,501,501,728,332,716,335,705,341,693,297,733,325]\n");
    commandWrite("[a,336,684,320,701,331,690,511,511,702,319,696,325,699,336,706,320,732,331]\n");
}

void turnRight()
{
    commandWrite("[a,338,685,321,702,332,691,512,512,703,320,697,326,700,338,707,321,734,332]\n");
    commandWrite("[a,338,685,321,702,332,691,512,512,703,320,697,326,700,338,707,321,734,332]\n");
    commandWrite("[a,355,685,333,727,342,711,512,530,728,332,717,336,718,338,693,295,722,311]\n");
    commandWrite("[a,372,685,346,753,353,730,512,549,754,345,736,348,737,338,680,269,709,292]\n");
    commandWrite("[a,389,685,359,778,366,748,512,568,780,358,754,360,755,338,667,244,696,274]\n");
    commandWrite("[a,406,684,372,805,378,765,512,587,806,371,772,373,774,338,655,217,682,257]\n");
    commandWrite("[a,424,685,384,831,392,782,512,605,833,384,788,388,792,338,642,191,668,241]\n");
    commandWrite("[a,440,684,372,805,383,765,512,624,806,372,772,379,810,338,654,217,676,257]\n");
    commandWrite("[a,457,685,360,778,375,748,512,642,780,360,754,372,828,338,666,244,683,274]\n");
    commandWrite("[a,474,685,348,753,368,730,512,660,754,348,736,366,846,338,679,269,689,292]\n");
    commandWrite("[a,491,685,337,727,361,711,512,678,728,337,717,361,863,338,691,295,695,311]\n");
    commandWrite("[a,507,685,325,702,355,690,512,696,703,325,696,356,880,338,703,321,700,332]\n");
    commandWrite("[a,491,685,299,689,331,680,512,678,690,299,686,330,863,338,729,333,726,342]\n");
    commandWrite("[a,474,685,273,676,308,669,512,660,677,273,675,306,846,338,756,346,750,352]\n");
    commandWrite("[a,457,685,247,664,286,659,512,642,664,246,665,283,828,338,784,358,774,363]\n");
    commandWrite("[a,440,685,220,651,266,647,512,624,652,219,653,262,810,338,812,371,796,374]\n");
    commandWrite("[a,424,685,193,638,247,636,512,605,639,192,642,242,792,338,840,384,817,386]\n");
    commandWrite("[a,406,685,219,651,261,647,512,587,652,217,653,256,774,338,813,371,803,374]\n");
    commandWrite("[a,389,685,244,664,276,659,512,568,664,243,665,271,755,338,786,358,788,363]\n");
    commandWrite("[a,372,685,270,676,293,669,512,549,677,268,675,287,737,338,759,346,771,352]\n");
    commandWrite("[a,355,685,295,689,312,680,512,530,690,294,686,306,718,338,733,333,753,342]\n");
    commandWrite("[a,338,684,321,702,332,690,512,512,703,320,697,326,700,338,707,321,733,332]\n");
}

void turnLeft()
{
    commandWrite("[a,338,685,321,702,332,691,512,512,703,320,697,326,700,338,707,321,734,332]\n");
    commandWrite("[a,338,685,321,702,332,691,512,512,703,320,697,326,700,338,707,321,734,332]\n");
    commandWrite("[a,338,667,295,689,311,680,493,512,690,294,686,305,700,320,733,333,754,342]\n");
    commandWrite("[a,338,650,269,676,292,668,474,512,677,268,674,286,700,303,760,346,774,353]\n");
    commandWrite("[a,338,633,244,663,274,656,455,512,664,242,662,268,700,286,787,359,793,365]\n");
    commandWrite("[a,338,616,217,650,257,643,436,512,651,216,649,251,700,269,814,372,811,378]\n");
    commandWrite("[a,338,599,191,638,241,630,418,512,638,189,634,234,700,251,843,384,828,392]\n");
    commandWrite("[a,338,582,217,650,257,639,399,512,650,216,642,251,700,234,814,372,811,383]\n");
    commandWrite("[a,338,565,244,662,274,647,381,512,662,242,650,268,700,218,787,360,793,375]\n");
    commandWrite("[a,338,548,269,673,292,654,363,512,674,268,656,286,700,201,760,348,774,367]\n");
    commandWrite("[a,338,531,295,685,311,661,345,512,685,294,661,305,700,184,733,337,754,361]\n");
    commandWrite("[a,338,515,321,697,332,666,327,512,697,320,666,326,700,168,707,325,734,355]\n");
    commandWrite("[a,338,531,333,723,342,691,345,512,723,332,692,336,700,184,693,299,723,330]\n");
    commandWrite("[a,338,548,346,749,352,714,363,512,749,345,716,347,700,201,681,273,712,307]\n");
    commandWrite("[a,338,565,358,775,363,736,381,512,776,358,739,357,700,218,668,246,701,286]\n");
    commandWrite("[a,338,582,371,802,374,756,399,512,803,370,760,369,700,234,655,220,690,266]\n");
    commandWrite("[a,338,599,384,829,386,775,418,512,830,383,780,380,700,251,642,193,678,247]\n");
    commandWrite("[a,338,616,371,803,374,761,436,512,805,370,766,369,700,269,655,218,690,261]\n");
    commandWrite("[a,338,633,358,778,363,746,455,512,779,358,751,357,700,286,668,244,701,276]\n");
    commandWrite("[a,338,650,346,752,352,729,474,512,754,345,735,347,700,303,681,270,712,293]\n");
    commandWrite("[a,338,667,333,727,342,710,493,512,728,332,716,336,700,320,693,295,723,312]\n");
    commandWrite("[a,338,685,321,702,332,690,512,512,703,320,696,325,700,338,707,321,734,331]\n");
}

void perform()
{
    commandWrite("[a,338,685,321,702,332,691,512,512,703,320,697,326,700,338,707,321,734,332]\n");
    commandWrite("[a,338,685,321,702,332,691,512,512,703,320,697,326,700,338,707,321,734,332]\n");
    commandWrite("[a,338,685,308,714,321,701,512,512,715,307,707,315,700,338,720,308,744,321]\n");
    commandWrite("[a,338,685,295,727,311,711,512,512,728,294,717,305,700,338,733,295,754,311]\n");
    commandWrite("[a,338,685,282,740,302,720,512,512,741,281,726,296,700,338,746,282,764,302]\n");
    commandWrite("[a,338,685,269,753,292,730,512,512,754,268,736,286,700,338,760,269,774,292]\n");
    commandWrite("[a,338,685,257,765,283,739,512,512,767,255,745,277,700,338,773,257,783,283]\n");
    commandWrite("[a,338,685,269,753,292,730,512,512,754,268,736,286,700,338,760,269,774,292]\n");
    commandWrite("[a,338,685,282,740,302,720,512,512,741,281,726,296,700,338,746,282,764,302]\n");
    commandWrite("[a,338,685,295,727,311,711,512,512,728,294,717,305,700,338,733,295,754,311]\n");
    commandWrite("[a,338,685,308,714,321,701,512,512,715,307,707,315,700,338,720,308,744,321]\n");
    commandWrite("[a,338,685,321,702,332,690,512,512,702,320,696,325,700,338,707,321,734,332]\n");
}

void commandWrite(std::string command)
{
    int rec = write(fd, command.c_str(), command.size());
    // std::cout << command << std::flush;
    float t = 50000. * (600. - (float)current_speed) / 300.;
    useconds_t us_time = (int)t;
    usleep(us_time);
}

void speedCommandWrite(int cmd_speed)
{
    if (current_speed == cmd_speed)
    {
        return;
    }
    current_speed = std::clamp(cmd_speed, 100, 500);
    std::string speed_str = std::to_string(current_speed);
    std::string command = "[v," + speed_str + "]\n";
    int rec = write(fd, command.c_str(), command.size());
    // std::cout << command << std::flush;
    usleep(50000);
}

int openSerial(const char *device_name, int baudrate_int)
{
    int file_descriptor = open(device_name, O_RDWR);
    // Create new termios struct, we call it 'tty' for convention
    struct termios tty;

    // Read in existing settings, and handle any error
    if(tcgetattr(file_descriptor, &tty) != 0)
    {
        printf("Serial Failed 1: %s\n", device_name);
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN]  = 0;

    // Set in/out baud rate
    unsigned int BAUDRATE;
    if (tryConvertingBaudrate(BAUDRATE, baudrate_int) == false)
    {
        printf("Invalid BAUDRATE : %s\n", device_name);
    }
    cfsetispeed(&tty, BAUDRATE);
    cfsetospeed(&tty, BAUDRATE);

    // Save tty settings, also checking for error
    if (tcsetattr(file_descriptor, TCSANOW, &tty) != 0)
    {
        printf("Serial Failed 2: %s\n", device_name);
    }

    if (file_descriptor < 0)
    {
        printf("Serial Failed 0: %s\n", device_name);
    }
    else
    {
        printf("Serial Open : %s\n", device_name);
    }

    return file_descriptor;
}

bool tryConvertingBaudrate(unsigned int &BAUDRATE, int baudrate_int)
{
    switch(baudrate_int)
    {
        case 0       : BAUDRATE = B0;       break;
        case 50      : BAUDRATE = B50;      break;
        case 75      : BAUDRATE = B75;      break;
        case 110     : BAUDRATE = B110;     break;
        case 134     : BAUDRATE = B134;     break;
        case 150     : BAUDRATE = B150;     break;
        case 200     : BAUDRATE = B200;     break;
        case 300     : BAUDRATE = B300;     break;
        case 600     : BAUDRATE = B600;     break;
        case 1200    : BAUDRATE = B1200;    break;
        case 1800    : BAUDRATE = B1800;    break;
        case 2400    : BAUDRATE = B2400;    break;
        case 4800    : BAUDRATE = B4800;    break;
        case 9600    : BAUDRATE = B9600;    break;
        case 19200   : BAUDRATE = B19200;   break;
        case 38400   : BAUDRATE = B38400;   break;
        case 57600   : BAUDRATE = B57600;   break;
        case 115200  : BAUDRATE = B115200;  break;
        case 230400  : BAUDRATE = B230400;  break;
        case 460800  : BAUDRATE = B460800;  break;
        case 500000  : BAUDRATE = B500000;  break;
        case 576000  : BAUDRATE = B576000;  break;
        case 921600  : BAUDRATE = B921600;  break;
        case 1000000 : BAUDRATE = B1000000; break;
        case 1152000 : BAUDRATE = B1152000; break;
        case 1500000 : BAUDRATE = B1500000; break;
        case 2000000 : BAUDRATE = B2000000; break;
        case 2500000 : BAUDRATE = B2500000; break;
        case 3000000 : BAUDRATE = B3000000; break;
        case 3500000 : BAUDRATE = B3500000; break;
        case 4000000 : BAUDRATE = B4000000; break;
        default      : return false;
    }
    return true;
}
