#pragma once

#include "turtlebot_usb_cam/usb_camera.hpp"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>
#include <cv_bridge/cv_bridge.h>

#include <string>
#include <memory>

class UsbCamNode : public rclcpp::Node
{
public:
    explicit UsbCamNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
    ~UsbCamNode();

private:
    void load_params();
    m_deviceConfig build_config();
    void set_controls();
    void timer_callback();

    std::shared_ptr<usb_cam> camera_;
    rclcpp::TimerBase::SharedPtr timer_;

    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr image_pub_;
    rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr compressed_pub_;

    /* params */
    std::string device_;
    std::string frame_id_;
    std::string resolution_str_;
    double fps_;
    int width_, height_;

    int brightness_, contrast_, saturation_, hue_, gamma_,
        sharpness_, whitebalance_, exposure_, focus_, zoom_, rotate_;
    bool auto_whitebalance_, auto_exposure_, auto_focus_;
    bool horizontal_flip_, vertical_flip_;
};
