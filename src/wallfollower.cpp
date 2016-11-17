//
// Created by cellis on 11/16/16.
//

#include "ros/ros.h"
#include "std_msgs/Float32.h"

ros::Publisher vel_l_pub;
ros::Publisher vel_r_pub;

float ir_f_val = 0;
float ir_l_val = 0;
float ir_r_val = 0;

float target_distance = 10;

float middle_speed = 10.0;
float scale_factor = 0.5;

int following_right = -1;

void irFCallback(const std_msgs::Float32::ConstPtr& ir_reading)
{
    ir_f_val = ir_reading->data;
}

void irLCallback(const std_msgs::Float32::ConstPtr& ir_reading)
{
    ir_l_val = ir_reading->data;
}

void irRCallback(const std_msgs::Float32::ConstPtr& ir_reading)
{
    ir_r_val = ir_reading->data;
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "wallfollower");

    ros::NodeHandle n;

    vel_l_pub = n.advertise<std_msgs::Float32>("vel_l",1000);
    vel_r_pub = n.advertise<std_msgs::Float32>("vel_r",1000);

    ros::Subscriber ir_f_sub = n.subscribe("ir_f",1000,irFCallback);
    ros::Subscriber ir_l_sub = n.subscribe("ir_l",1000,irLCallback);
    ros::Subscriber ir_r_sub = n.subscribe("ir_r",1000,irRCallback);


    //Because it didn't seem appropriate to have my control loop in a callback
    ros::Rate loop_rate(100); //1000Hz

    std_msgs::Float32 right_speed;
    std_msgs::Float32 left_speed;
    right_speed.data = middle_speed;
    left_speed.data  = middle_speed;
    vel_r_pub.publish(right_speed);
    vel_l_pub.publish(left_speed);

    while(ir_l_val == 0 && ir_r_val == 0)
    {
        loop_rate.sleep();
        vel_r_pub.publish(right_speed);
        vel_l_pub.publish(left_speed);
        ros::spinOnce(); //spin til you get sensor inputs
    }

    if(ir_l_val > ir_r_val) {
        following_right = 1;
    }
    else {
        following_right = 0;
    }
    while (ros::ok())
    {

        //if that there wall is too close to your front
        if (ir_f_val < 20){
            std::cout << "Too close to front";
            std_msgs::Float32 stop;
            std_msgs::Float32 run;

            stop.data = -20.0;
            run.data = 20.0;

            if(following_right)
            {
                vel_l_pub.publish(stop);
                vel_r_pub.publish(run);
            }
            else
            {
                vel_l_pub.publish(run);
                vel_r_pub.publish(stop);
            }
        }

        else if (ir_l_val == 20.0 && ir_r_val == 20.0)
        {
            right_speed.data = middle_speed;
            left_speed.data  = middle_speed;
            vel_r_pub.publish(right_speed);
            vel_l_pub.publish(left_speed);
        }

        else
        {
            if(following_right)
            {
                float error = target_distance - ir_r_val;

                right_speed.data = middle_speed + scale_factor * error;
                left_speed.data  = middle_speed - scale_factor * error;
            }
            else
            {
                float error = target_distance - ir_l_val;

                right_speed.data = middle_speed - scale_factor * error;
                left_speed.data  = middle_speed + scale_factor * error;
            }

            vel_r_pub.publish(right_speed);
            vel_l_pub.publish(left_speed);
        }

        loop_rate.sleep();
        ros::spinOnce();
    }
}