#pragma once

#include <string>
#include <map>
#include <vector>
#include <ostream>
#include <fstream>
#include <streambuf>

// Different types that can be stored with a key
enum Types {
  VALUE = 0,
  ARRAY = 1,
  OBJECT = 2
};

class Object;
static Object *nil = nullptr;

// This class is used to represent a JSON in C++
class Object {
  private:
    std::string value;
    std::vector<Object> array;
    std::map<std::string, Object> map;
    int type;
    bool valid;
    int depth;
  public:
    Object(): valid(false), depth(0) {
      if (!nil) {
        /* Must use malloc to avoid recursive loop */
        nil = (Object *) malloc(sizeof(Object));
        nil->valid = false;
      }
    }
    Object(int value) {
      this->type = VALUE;
      this->value = std::to_string(value);
      this->valid = true;
    }
    Object(double value) {
      this->type = VALUE;
      this->value = std::to_string(value);
      this->valid = true;
    }
    Object(bool value) {
      this->type = VALUE;
      this->value = (value ? "true" : "false");
      this->valid = true;
    }
    Object(std::string value, bool quote = true) {
      this->type = VALUE;
      if (quote) this->value = "\"";
      this->value += value;
      if (quote) this->value += "\"";
      this->valid = true;
    }
    Object(char *value) {
      *this = Object(std::string(value));
    }
    Object(const char *value) {
      *this = Object((char *) value);
    }
    Object(std::vector<Object> value) {
      this->type = ARRAY;
      this->array = value;
      this->valid = true;
    }
    bool exists() {
      return this->valid;
    }
    bool contains(std::string key) {
      if (!this->valid) return false;
      if (this->type == OBJECT) {
        return (*this)[key].exists();
      } else if (this->type == ARRAY) {
        for (unsigned long i = 0; i < this->array.size(); ++i)
          if (this->array[i].asString() == key)
            return true;
      }
      return false;
    }
    template <typename T>
    bool contains(T key) {
      if (!this->valid) return false;
      if (this->type == OBJECT) {
        return (*this)[key].exists();
      } else if (this->type == ARRAY) {
        for (unsigned long i = 0; i < this->array.size(); ++i)
          if (this->array[i].asString() == std::to_string(key))
            return true;
      }
      return false;
    }
    Object &operator[](std::string key) {
      if (!this->valid && this->map.find(key) == this->map.end()) {
        this->insert(key, "");
        this->valid = true;
        this->type = OBJECT;
      }
      if (!this->valid || this->type != OBJECT) return *nil;
      return this->map[key];
    }
    bool insert(std::string key, Object value) {
      if (!this->valid || this->type == OBJECT) {
        value.depth = this->depth + 1;
        this->valid = true;
        this->type = OBJECT;
        this->map.insert(std::pair<std::string, Object>(key, value));
        return true;
      }
      return false;
    }
    void remove(std::string key) {
      if (this->valid && this->type == OBJECT) this->map.erase(key);
    }
    Object &operator[](unsigned long i) {
      if (this->valid && this->type == ARRAY && i < this->array.size()) return this->array[i];
      return *nil;
    }
    bool append(Object value) {
      if ((this->type != ARRAY && !this->valid) || (this->type == VALUE && this->value == "\"\"")) this->type = ARRAY;
      if (this->type == ARRAY) {
        this->valid = true;
        value.depth = this->depth + 1;
        this->array.push_back(value);
      }
      return false;
    }
    bool insert(unsigned long i, Object value) {
      if ((this->type != ARRAY && !this->valid) || (this->type == VALUE && this->value == "")) this->type = ARRAY;
      if (this->type == ARRAY) {
        this->valid = true;
        value.depth = this->depth + 1;
        this->array.insert(this->array.begin() + i, value);
      }
      return false;
    }
    bool remove(unsigned long i) {
      if (this->type == ARRAY) {
        this->valid = true;
        this->array.erase(this->array.begin() + i);
      }
      return false;
    }
    int asInt() {
      if (this->type == VALUE) return stoi(this->value);
      return 0;
    }
    double asDouble() {
      if (this->type == VALUE) return stod(this->value);
      return 0;
    }
    bool asBool() {
      if (this->type == VALUE) {
        std::string temp = this->value;
        for (auto &c : temp)  c = std::toupper(c);
        return temp != "0" || temp == "TRUE" || temp == "T";
      }
      return 0;
    }
    std::string asString() {
      if (this->type == VALUE) {
        if (this->value[0] == '\"' && this->value[this->value.size() - 1] == '\"' && this->value.size() - 2 > 0) {
          return this->value.substr(1, this->value.size() - 2);
        }
        return this->value;
      }
      return "";
    }
    static Object fromString(std::string str, int depth) {
      Object out;
      out.depth = depth;
      unsigned long start, i, curls, brackets;
      bool quoted;
      switch (str[0]) {
        case '{':
          start = 2;
          while (start < str.size()) {
            unsigned long i = start;
            while (i < str.size() && str[i] != '\"') ++i;
            std::string key = str.substr(start, i - start);
            unsigned long v = i = i + 2;
            curls = 0;
            brackets = 0;
            quoted = false;
            while ((i < str.size() && str[i] != ',' && str[i] != '}') || curls || brackets || quoted) {
              if (str[i] == '{') ++curls;
              else if (str[i] == '}') --curls;
              else if (str[i] == '[') ++brackets;
              else if (str[i] == ']') --brackets;
              else if (str[i] == '\"') quoted = !quoted;
              ++i;
            }
            start = i + 2;
            out[key] = Object::fromString(str.substr(v, i - v), out.depth + 1);
          }
          break;
        case '[':
          start = i = 1;
          curls = 0;
          brackets = 0;
          quoted = false;
          while (i < str.size()) {
            while ((i < str.size() && str[i] != ',' && str[i] != ']') || curls || brackets || quoted) {
              if (str[i] == '{') ++curls;
              else if (str[i] == '}') --curls;
              else if (str[i] == '[') ++brackets;
              else if (str[i] == ']') --brackets;
              else if (str[i] == '\"') quoted = !quoted;
              ++i;
            }
            out.append(Object::fromString(str.substr(start, i - start), out.depth + 1));
            start = i = i + 1;
          }
          break;
        default:
          return out = Object(str, false);
          break;
      }
      return out;
    }
    void saveToFile(const char *file) {
      std::ofstream f(file);
      f << *this << std::endl;
    }
    static Object fromFile(const char *file) {
      std::ifstream f(file);
      std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
      bool quoted = false;
      auto it = str.begin();
      for (auto itr = str.begin(); itr != str.end(); ++itr) {
        if (!isspace(*itr) || quoted) *(it++) = *itr;
        if (*itr == '\"') quoted = !quoted;
      }
      str.erase(it, str.end());
      return fromString(str, 0);
    }
    bool operator ==(std::string str) {
      return this->type == VALUE && this->value.substr(1, this->value.size() - 2) == str;
    }
    friend std::ostream& operator <<(std::ostream& os, Object obj) {
      if (!obj.valid) return os << "\n";
      switch (obj.type) {
        case VALUE:
          os << obj.value;
          break;
        case ARRAY:
          os << "[ ";
          for (unsigned long i = 0; i < obj.array.size() - 1; ++i)
            os << obj.array[i] << ", ";
          os << obj.array[obj.array.size() - 1] << " ]";
          break;
        case OBJECT:
          os << "{\n";
          auto stop = obj.map.end();
          --stop;
          for (auto pair = obj.map.begin(); pair != stop; ++pair) {
            for (int i = 0; i <= obj.depth; ++i) os << "\t";
            os << "\"" << pair->first << "\"" << ": " << pair->second << ",\n";
          }
          for (int i = 0; i <= obj.depth; ++i) os << "\t";
          os << "\"" << stop->first << "\"" << ": " << stop->second << "\n";
          for (int i = 0; i < obj.depth; ++i) os << "\t";
          os << "}";
          break;
      }
      return os;
    }
};
