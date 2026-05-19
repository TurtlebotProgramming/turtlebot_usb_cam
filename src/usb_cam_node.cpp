#include "turtlebot_usb_cam/usb_cam_node.hpp"
#include <opencv2/opencv.hpp>

UsbCamNode::UsbCamNode(const rclcpp::NodeOptions & options)
: rclcpp::Node("usb_cam_node", options)
{
    declare_parameter("device",          std::string("/dev/video0"));
    declare_parameter("frame_id",        std::string("camera"));
    declare_parameter("resolution",      std::string("640x480"));
    declare_parameter("fps",             30.0);
    declare_parameter("brightness",      -1);
    declare_parameter("contrast",        -1);
    declare_parameter("saturation",      -1);
    declare_parameter("hue",             -1);
    declare_parameter("gamma",           -1);
    declare_parameter("sharpness",       -1);
    declare_parameter("whitebalance",    -1);
    declare_parameter("exposure",        -1);
    declare_parameter("focus",           -1);
    declare_parameter("zoom",            -1);
    declare_parameter("rotate",          -1);
    declare_parameter("auto_whitebalance", true);
    declare_parameter("auto_exposure",     true);
    declare_parameter("auto_focus",        false);
    declare_parameter("horizontal_flip",   false);
    declare_parameter("vertical_flip",     false);

    load_params();

    image_pub_      = create_publisher<sensor_msgs::msg::Image>("/camera/image_raw", 10);
    compressed_pub_ = create_publisher<sensor_msgs::msg::CompressedImage>("/camera/compressed", 10);

    camera_ = std::make_shared<usb_cam>();

    m_deviceConfig cfg = build_config();
    set_controls();

    v4l2_stream_err err = camera_->start_stream(cfg);
    if (err != STREAM_OK)
    {
        RCLCPP_ERROR(get_logger(), "Failed to start camera stream (err=%d)", err);
        return;
    }

    int period_ms = static_cast<int>(1000.0 / fps_);
    timer_ = create_wall_timer(
        std::chrono::milliseconds(period_ms),
        std::bind(&UsbCamNode::timer_callback, this));

    RCLCPP_INFO(get_logger(), "Camera streaming: %s @ %dx%d %.0ffps",
                device_.c_str(), width_, height_, fps_);
}

UsbCamNode::~UsbCamNode()
{
    if (camera_) camera_->stop_stream();
}

void UsbCamNode::timer_callback()
{
    cv::Mat frame;
    {
        std::lock_guard<std::mutex> lock(camera_->image_mutex);
        if (camera_->m_image.empty())
            return;
        frame = camera_->m_image.clone();
    }

    std_msgs::msg::Header header;
    header.stamp    = now();
    header.frame_id = frame_id_;

    auto img_msg = cv_bridge::CvImage(header, "bgr8", frame).toImageMsg();
    image_pub_->publish(*img_msg);

    sensor_msgs::msg::CompressedImage compressed;
    compressed.header = header;
    compressed.format = "jpeg";
    cv::imencode(".jpg", frame, compressed.data);
    compressed_pub_->publish(compressed);
}

void UsbCamNode::load_params()
{
    get_parameter("device",          device_);
    get_parameter("frame_id",        frame_id_);
    get_parameter("resolution",      resolution_str_);
    get_parameter("fps",             fps_);
    get_parameter("brightness",      brightness_);
    get_parameter("contrast",        contrast_);
    get_parameter("saturation",      saturation_);
    get_parameter("hue",             hue_);
    get_parameter("gamma",           gamma_);
    get_parameter("sharpness",       sharpness_);
    get_parameter("whitebalance",    whitebalance_);
    get_parameter("exposure",        exposure_);
    get_parameter("focus",           focus_);
    get_parameter("zoom",            zoom_);
    get_parameter("rotate",          rotate_);
    get_parameter("auto_whitebalance", auto_whitebalance_);
    get_parameter("auto_exposure",     auto_exposure_);
    get_parameter("auto_focus",        auto_focus_);
    get_parameter("horizontal_flip",   horizontal_flip_);
    get_parameter("vertical_flip",     vertical_flip_);

    auto x = resolution_str_.find('x');
    if (x != std::string::npos)
    {
        width_  = std::stoi(resolution_str_.substr(0, x));
        height_ = std::stoi(resolution_str_.substr(x + 1));
    }
    else
    {
        width_ = 640; height_ = 480;
    }
}

m_deviceConfig UsbCamNode::build_config()
{
    m_deviceConfig cfg;
    cfg.path       = device_;
    cfg.resolution = {width_, height_};
    cfg.fps        = static_cast<float>(fps_);
    cfg.format     = "MJPEG";
    return cfg;
}

void UsbCamNode::set_controls()
{
    if (brightness_ >= 0) camera_->set_control("brightness",        brightness_);
    if (contrast_   >= 0) camera_->set_control("contrast",          contrast_);
    if (saturation_ >= 0) camera_->set_control("saturation",        saturation_);
    if (hue_        >= 0) camera_->set_control("hue",               hue_);
    if (gamma_      >= 0) camera_->set_control("gamma",             gamma_);
    if (sharpness_  >= 0) camera_->set_control("sharpness",         sharpness_);
    if (exposure_   >= 0) camera_->set_control("exposure_absolute", exposure_);
    if (focus_      >= 0) camera_->set_control("focus_absolute",    focus_);
    if (zoom_       >= 0) camera_->set_control("zoom_absolute",     zoom_);

    camera_->set_control("white_balance_temperature_auto", auto_whitebalance_ ? 1 : 0);
    camera_->set_control("exposure_auto",                  auto_exposure_ ? 3 : 1);
    camera_->set_control("focus_auto",                     auto_focus_ ? 1 : 0);
    if (horizontal_flip_) camera_->set_control("horizontal_flip", 1);
    if (vertical_flip_)   camera_->set_control("vertical_flip",   1);
}

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<UsbCamNode>());
    rclcpp::shutdown();
    return 0;
}
