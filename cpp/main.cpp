#include <cstdio>
#include <string>
#include <iostream>
#include "3dPrinterOs.h"
#include "Object.h"
#include <pigpio.h>
#include <chrono>
#include <thread>

// Port used for web api
const char *PORT = "8888";

// Digital out pin numbers for SPI
unsigned latch = 22, enable = 17;

int main(int argc, char **argv) {

  // Initializing GPIO and SPI
  gpioInitialise();
  gpioSetMode(latch, 1);
  gpioWrite(latch, 0);
  gpioSetMode(enable, 1);
  gpioWrite(enable, 0);
  int handle = spiOpen(0, 500000, 32);

  if (handle >= 0) {

    // Grabbing the ID of the printer from the command line
    std::string id = std::string((const char *) argv[1]);
    // Getting login credentials from file and logging into a session with the api (exit if failed)
    Object login = Object::fromFile("/root/ECE-4180-Project/login.cfg");
    PrinterAPI api(login["username"].asString(), login["password"].asString());
    if (api.getSession() == "") {
      std::cout << "Login Failed" << std::endl;
      return 1;
    }

    // Getting the printer data from the api (exit if failed)
    Object data = api.getPrinterState();
    if (!data["result"].asBool()) {
      std::cout << "Printer List Failed" << std::endl;
      return 1;
    }

    // Getting printer info from the response from the api
    Object printerInfo = data["printers"];
    // Opening file for state information (colors)
    Object info = Object::fromFile("/root/ECE-4180-Project/Info.json");

    // For loop to find the ID we are concerned with
    unsigned int i = 0;
    while (printerInfo[i].exists()) {

      if (printerInfo[i]["id"] == id) {

        // Grabbing the color and creating a pointer to it to use with SPI
        unsigned color = stol(info["Statuses"][printerInfo[i]["state"].asString()].asString().substr(2), nullptr, 16);
        unsigned *data = &color;

        // Writing the color to SPI
        spiWrite(handle, (char *) data, 4);
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        gpioWrite(latch, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        gpioWrite(latch, 0);

        break;

      }

      ++i;

    }

    // Closing the SPI connection
    spiClose(handle);

  } else {

    // Error message for SPI failure
    std::cout << "Failed to open SPI." << std::endl;

  }

  return 0;
  
}
