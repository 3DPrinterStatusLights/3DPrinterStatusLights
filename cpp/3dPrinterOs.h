#pragma once

#include <cstdlib>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include "Object.h"

// URL of the web server
const std::string url = "https://cloud.3dprinteros.com/apiglobal/";

// Used when an error occurs
Object failure;

class PrinterAPI {
  private:
    std::string username, password, session;
    /**
      Sends a POST request to {url} with {data} as parameters

      @param url the url for the post request.
      @param data parameters for the post request.
      @return the response from the POST request.
    */
    std::string post(std::string url, std::string data) {
      char buf[2048];
      std::string res;
      std::shared_ptr<FILE> pipe(popen(("curl -d \"" + data + "\" -XPOST " + url).c_str(), "r"), pclose);
      if (!pipe) throw std::runtime_error("popen() failed!");
      while (!feof(pipe.get())) if (fgets(buf, 128, pipe.get()) != nullptr) res += buf;
      return res;
    }
  public:
    /**
      Initializes a session with the api.

      @param username the username for login.
      @param password the password for login.
    */
    PrinterAPI(std::string username, std::string password): username(username), password(password), session("") {
      static bool first = true;
      if (first) {
        first = false;
        failure["result"] = false;
      }
      this->login();
    }
    /**
      @return the session key.
    */
    std::string getSession() {
      return this->session;
    }
    /**
      Sends a POST request to login to the api.

      @return a bool representing if the login was successful.
    */
    bool login() {
      Object response = Object::fromString(this->post(url + "login", "username=" + this->username + "&password=" + this->password), 0);
      if (response["result"].asBool()) {
        this->session = response["message"]["session"].asString();
        return true;
      }
      return false;
    }
    /**
      Sends a POST request to gather the printer information.

      @return the response from the POST request.
    */
    Object getPrinterState() {
      Object response = Object::fromString(this->post(url + "get_organization_printers_list", "session=" + this->session), 0);
      if (response["result"].asBool()) {
        Object result;
        result["result"] = true;
        Object file = Object::fromFile("/root/ECE-4180-Project/Info.json");
        int i = 0;
        while (response["message"][i].exists()) {
          Object printer = response["message"][i];
          if (file["IDs"].contains(printer["id"].asString())) {
            if (file["Printers"][printer["id"].asString()]["type"].asString() == "UM2" && printer["local_ip"].asString().substr(0, 10) == "192.168.1.") {
              file["Printers"][printer["id"].asString()]["ip"] = printer["local_ip"].asString();
            }
            Object temp;
            temp["id"] = printer["id"].asString();
            temp["ip"] = file["Printers"][printer["id"].asString()]["ip"];
            temp["state"] = printer["state"];
            result["printers"].append(temp);
          }
          ++i;
        }
        file.saveToFile("/root/ECE-4180-Project/Info.json");
        return result;
      } else {
        this->login();
        return this->getPrinterState();
      }
      return failure;
    }
};
