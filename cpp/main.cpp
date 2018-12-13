#include <cstdio>
#include <string>
#include <iostream>
#include "3dPrinterOs.h"
#include "Object.h"
#include <pigpio.h>
#include <chrono>
#include <thread>

const char *PORT = "8888";

unsigned latch = 22, enable = 17;

int main(int argc, char **argv) {
  gpioInitialise();
  gpioSetMode(latch, 1);
  gpioWrite(latch, 0);
  gpioSetMode(enable, 1);
  gpioWrite(enable, 0);
  int handle = spiOpen(0, 500000, 32);
  if (handle >= 0) {
    std::string id = std::string((const char *) argv[1]);
    Object login = Object::fromFile("/root/ECE-4180-Project/login.cfg");
    PrinterAPI api(login["username"].asString(), login["password"].asString());
    if (api.getSession() == "") {
      std::cout << "Login Failed" << std::endl;
      return 1;
    }
    Object data = api.getPrinterState();
    if (!data["result"].asBool()) {
      std::cout << "Printer List Failed" << std::endl;
      return 1;
    }
    Object printerInfo = data["printers"];
    Object info = Object::fromFile("/root/ECE-4180-Project/Info.json");
    unsigned int i = 0;
    while (printerInfo[i].exists()) {
      if (printerInfo[i]["id"] == id) {
        std::cout << "Match" << std::endl;
        std::cout << printerInfo[i] << std::endl;
        std::cout << info["Statuses"][printerInfo[i]["state"].asString()].asString().substr(2) << std::endl;
        unsigned color = stol(info["Statuses"][printerInfo[i]["state"].asString()].asString().substr(2), nullptr, 16);
        unsigned *data = &color;
        std::cout << "Bytes written: " << spiWrite(handle, (char *) data, 4) << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        gpioWrite(latch, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        gpioWrite(latch, 0);
        break;
      }
      ++i;
    }
    spiClose(handle);
  } else {
    std::cout << "Failed to open SPI." << std::endl;
  }
  return 0;
}
