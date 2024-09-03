#include <iostream>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <linux/input.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/types.h>

using std::cout;
using std::endl;

void signal_handler(int s);

bool is_running;
int consoleFd;

void set_signal_handler() {
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = signal_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);
}

void signal_handler(int s) {
  printf("\ncaught signal %d\n", s);
  is_running = false;
}

bool waitForKeyPress(char & key, int timeout_ms) {
  // std::cout << "Press any key to continue..." << std::endl;

  struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);  // get current terminal attributes
  newt = oldt;                     // start with current settings
  newt.c_lflag &= ~(ICANON | ECHO); // disable canonical mode and echo
  tcsetattr(STDIN_FILENO, TCSANOW, &newt); // apply new settings

  fd_set set;
  struct timeval timeout;

  // Initialize the set
  FD_ZERO(&set);
  FD_SET(STDIN_FILENO, &set);

  // Set the timeout
  timeout.tv_sec = timeout_ms / 1000;
  timeout.tv_usec = (timeout_ms % 1000) * 1000;

  // Wait for input or timeout
  int res = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);
  if (res > 0) {
    key = getchar(); // key was pressed
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // restore old settings
    return true;
  } else {
    key = 0; // no key was pressed
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // restore old settings
    return false; // timeout
  }
}

void beep(int frequency_hz) {
  // Prepare the input event
  struct input_event e;
  e.type = EV_SND;
  e.code = SND_TONE;
  e.value = frequency_hz; // Set the frequency in Hz

  // Write the event to the device
  if (write(consoleFd, &e, sizeof(struct input_event)) < 0) {
    std::cerr << "Failed to write to /dev/buzzer" << std::endl;
    close(consoleFd);
  } else {
    // std::cout << "Buzzer is now making a sound" << std::endl;
  }
}

int main() {

  set_signal_handler();

  consoleFd = open("/dev/buzzer", O_WRONLY);

  // Open the device file
  if (consoleFd < 0) {
    std::cerr << "Failed to open /dev/buzzer" << std::endl;
    return 1;
  }

  int frequency = 2200;

  is_running = true;

  auto print_frequency = [&]() {
    cout << "Frequency: " << frequency << " Hz" << std::endl;
  };

  while (is_running) {
    char key;
    if (waitForKeyPress(key, 1000)) {
      cout << "Key pressed: " << key << std::endl;
      switch (key)
      {
      case 'q':
        cout << "Exiting..." << std::endl;
        is_running = false;
        break;
      case 'a':
        frequency -= 100;
        print_frequency();
        beep(frequency);
        break;
      case 's':
        frequency += 100;
        print_frequency();
        beep(frequency);
        break;
      case 'z':
        frequency -= 5;
        print_frequency();
        beep(frequency);
        break;
      case 'x':
        frequency += 5;
        print_frequency();
        beep(frequency);
        break;
      case '0':
        frequency = 0;
        print_frequency();
        beep(frequency);
        break;	
      case '1':
        frequency = 880;
        print_frequency();
        beep(frequency);
        break;
      case '2':
        frequency = 2450;
        print_frequency();
        beep(frequency);
        break;

      default:
          continue;
      }
    }
  }

  // Close the device file
  close(consoleFd);
  return 0;
}