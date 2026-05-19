#ifndef USB_CAMERA_HPP
#define USB_CAMERA_HPP

#define NON_DEBUG

#ifdef DEBUG
#define CERR(x) std::cerr << x
#define CERR_ENDL(x) std::cerr << x << std::endl
#define COUT(x) std::cout << x
#define COUT_ENDL(x) std::cout << x << std::endl
#else
#define CERR(x) \
  do {          \
  } while (0)
#define CERR_ENDL(x) \
  do {               \
  } while (0)
#define COUT(x) \
  do {          \
  } while (0)
#define COUT_ENDL(x) \
  do {               \
  } while (0)
#endif

#include <opencv2/opencv.hpp>

#include <dirent.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cstring>
#include <iostream>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

struct ResolutionInfo
{
  std::pair<int, int> resolution;
  std::vector<float> fps;
};

struct m_deviceInfo
{
  std::string device_name;
  std::string driver;
  std::string bus_info;

  std::vector<std::string> formats;
  std::vector<ResolutionInfo> resolution_info;
};

struct m_deviceConfig
{
  std::string path;
  std::string device_name;
  std::string format;
  std::pair<int, int> resolution;
  float fps;
};

enum v4l2_stream_err {
  STREAM_OK = 0,
  STREAM_ERR_NO_DEVICE,
  STREAM_ERR_FORMAT,
  STREAM_ERR_FPS,
  STREAM_ERR_REQUEST_BUFFER,
  STREAM_ERR_QUERY_BUFFER,
  STREAM_ERR_MAP_BUFFER,
  STREAM_ERR_QUEUE_BUFFER,
  STREAM_ERR_STREAM_FAIL
};

class usb_cam
{
public:
  usb_cam();
  ~usb_cam();

  m_deviceInfo get_device_info(const std::string & devicePath);
  v4l2_stream_err start_stream(const m_deviceConfig & config);
  void stop_stream();

  bool set_control(int control_id, int value);
  bool set_control(std::string control_name, int value);
  int get_control(int control_id);
  int get_control(std::string control_name);
  bool query_control(int control_id, v4l2_queryctrl & queryctl);
  bool query_control(std::string control_name, v4l2_queryctrl & queryctl);
  void reset_controls_to_default();

  cv::Mat m_image;
  std::mutex image_mutex;
  std::atomic<bool> streaming;

  std::string get_control_name(int control_id);
  int get_control_by_name(std::string name);

  std::string normalize_format(const std::string & format);
  bool is_format_supported(
    const std::string & supported_format, const std::string & requested_format);

private:
  std::vector<void *> buffers;
  std::vector<size_t> buffer_lengths;
  std::thread stream_thread;
  int m_fd;

  int xioctl(int fd, unsigned long request, void * arg);
};

#endif
