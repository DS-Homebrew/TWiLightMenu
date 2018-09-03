#include <variant>
#include <string>
#include <vector>

#pragma once
#ifndef __DSIMENUPP_SETTINGS_PAGE_H_
#define __DSIMENUPP_SETTINGS_PAGE_H_

class Option
{
public:
  class Sub
  {
  public:
    Sub(int page) { _page = page; }
    ~Sub() {}
    int page() { return _page; }

  private:
    int _page;
  };

  class Bool
  {
  public:
    Bool(bool *pointer) { _pointer = pointer; };
    ~Bool() {}
    void set(bool value) { (*_pointer) = value; };
    bool get() { *_pointer; };

  private:
    bool *_pointer;
  };

  class Int
  {
  public:
    Int(int *pointer) { _pointer = pointer; };
    ~Int() {}
    void set(int value) { (*_pointer) = value; };
    int get() { *_pointer; };

  private:
    int *_pointer;
  };

  class Str
  {
  public:
    Str(std::string *pointer) { _pointer = pointer; };
    ~Str() {}
    void set(std::string value) { (*_pointer) = value; };
    std::string &get() { *_pointer; };

  private:
    std::string *_pointer;
  };

public:
  Option(const std::string &displayName,
                 const std::string &longDescription,
                 std::variant<Sub, Bool, Int, Str> action,
                 std::initializer_list<std::string> const &labels,
                 std::initializer_list<std::variant<bool, int, std::string>> const &values)
      : _action(action)
  {
    _displayName = displayName;
    _longDescription = longDescription;
    _action = action;
    _labels = std::vector(std::move(labels));
    _values = std::vector(std::move(values));
  }
  ~Option() {}

  std::string &displayName() { return _displayName; }
  std::string &longDescription() { return _longDescription; }
  std::variant<Sub, Bool, Int, Str> &action() { return _action; }
  std::vector<std::string> &labels() { return _labels; }

private:
  std::string _displayName;
  std::string _longDescription;
  std::variant<Sub, Bool, Int, Str> _action;
  std::vector<std::string> _labels;
  std::vector<std::variant<bool, int, std::string>> _values;
};

class SettingsPage
{
public:
  SettingsPage() {}
  ~SettingsPage() {}

  SettingsPage &option(
                const std::string &displayName,
                const std::string &longDescription,
                std::variant<Option::Sub, Option::Bool, Option::Int, Option::Str> action,
                std::initializer_list<std::string> const &labels,
                std::initializer_list<std::variant<bool, int, std::string>> const &values)
  {
    _options.emplace_back(displayName, longDescription, action, labels, values);
    return *this;
  }

  std::vector<Option> &options() { return _options; }

private:
  std::vector<Option> _options;
};

#endif
