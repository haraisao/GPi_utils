/*
 *
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdio>
#include <cmath>
#include <chrono>
#include <future>
#include <thread>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

#define JOY_DEV "/dev/input/js0"

#define INTERVAL 	100000
#define MAX_AXIS 	32767
#define WAIT_TIME	3

#define A_BTN 	0
#define B_BTN 	1
#define X_BTN 	2
#define Y_BTN 	3
#define LB_BTN 	4
#define RB_BTN 	5
#define SELECT_BTN 	6
#define START_BTN 	7
#define FLAG_BTN 	8


using namespace std::chrono_literals;


/*
 *
 */
class JoyStick
{
public:
  JoyStick(std::string name) {
    dev_name = name;
    fd = -1;
    num_of_axis = 0;
    num_of_buttons = 0;
  }

  ~JoyStick() {
    this->close();
  }

  int open(int trial=10, bool blocking=true){
    for(int i=0; i<trial; i++){
      this->fd = ::open(JOY_DEV,O_RDONLY);
      if (this->fd > 0) { break; }
      //std::cerr << "Failed to open " << dev_name << std::endl;
      sleep(WAIT_TIME);
    }
    
    init(blocking);
    return fd;
  }

  void init(bool blocking=true) {
    if (this->fd > 0) {

      auto res1 = ioctl(this->fd, JSIOCGAXES, &num_of_axis);
      auto res2 = ioctl(this->fd, JSIOCGBUTTONS, &num_of_buttons);
      auto res3 = ioctl(this->fd, JSIOCGNAME(80), &name_of_joystick);

      joy_button.resize(num_of_buttons,0);
      joy_axis.resize(num_of_axis,0);
      hat.resize(2, 0);

      if (blocking) {
        fcntl(fd, F_SETFL, O_NONBLOCK);   // using non-blocking mode
      }
    }
    std::cerr << " ----->" << dev_name << std::endl;
    return;
  }

  void close(){
    if (this->fd > 0) {
      ::close(this->fd);
    }
  }

  void print_info() {
    std::cout << "Joystick: "  << name_of_joystick << std::endl
              << "  axis: "    << num_of_axis << std::endl
              << "  buttons: " << num_of_buttons << std::endl;
    return;
  }

  void print_event() {
    get_hat(6, 7);

    std::cout << "H: " ;
    for (int x : hat) {
      std::cout << x << " " ;
    }

    std::cout << " B: " ;
    for (int x : joy_button) {
      std::cout << x << " " ;
    }
    std::cout << std::endl;
    return;
  }

  void read() {
    ::read(fd, &js, sizeof(js_event));
    switch (js.type & ~JS_EVENT_INIT) {
      case JS_EVENT_AXIS:
        if((int)js.number < joy_axis.size()) {
          joy_axis[(int)js.number] = js.value;
	}
        break;

      case JS_EVENT_BUTTON:
        if((int)js.number < joy_button.size()) {
          joy_button[(int)js.number] = js.value;
	}
        break;
    }
    return;
  }

  bool chk_button(std::vector<int> ids) {
    for (int x : ids) {
      if (joy_button[x] == 0) return false; 
    }
    return true;
  }

  std::vector<int>
  get_hat(int x=6, int y=7) {
    hat[0] = joy_axis[x]/MAX_AXIS;
    hat[1] = -joy_axis[y]/MAX_AXIS;

    return hat;
  }

  /* variables */
  std::string dev_name;
  std::vector<char> joy_button;
  std::vector<int> joy_axis;
  std::vector<int> hat;

private:
  int fd;
  int num_of_axis;
  int num_of_buttons;
  char name_of_joystick[80];
  js_event js;
};


int gpi_callback() {
  system("/opt/local/bin/gpi_callback.sh"); 
  return 0;
}
/*
 *
 */

int
main(int argc, char **argv) {
  std::vector<int> Hotkey0{ LB_BTN, SELECT_BTN };
  std::vector<int> Hotkey1{ LB_BTN, FLAG_BTN };

  std::future<int> ft;

  auto js = JoyStick(JOY_DEV);

  if(js.open(10, false) < 0){
    std::cerr << "Failed to open " << js.dev_name << std::endl;
    return -1;
  }

  js.print_info();

  while(1){
    js.read();

    /*** Call external command **/
    if (js.chk_button(Hotkey0)) {
      try{
        auto status = ft.wait_for(0ms);
        if(status == std::future_status::ready){
          ft = std::async(gpi_callback);
        }else{
         std::cout << "Skip..." << std::endl;
	}
      }catch(...){
        ft = std::async(gpi_callback);
      }
    }
    
    /*
    if (js.chk_button(Hotkey1)) {
      std::cerr << "PRESS HotKey1 " << std::endl;
    }
    */
    //usleep(INTERVAL);
  }

  js.close();
  return 0;
}
