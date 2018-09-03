#include <variant>
#include <string>
#include <vector>
#include <algorithm>
#include <nds.h>
#include <functional>
#include <memory>
#pragma once
#ifndef __DSIMENUPP_SETTINGS_PAGE_H_
#define __DSIMENUPP_SETTINGS_PAGE_H_
typedef const char* cstr;

/**
 * \brief Represents a settings option with an associated INI 
 *        entry that can be displayed to the user.
 * 
 */
class Option
{
public:
  // /**
  //  * \brief Represents a submenu, or jump to a page.
  //  */
  // class Sub
  // {
  // public:
  //   Sub(int page) { _page = page; }
  //   ~Sub() {}
  //   int page() { return _page; }

  // private:
  //   int _page;
  // };

  /**
   * \brief Represents a boolean option
   * 
   * @param pointer A pointer to the backing boolean that will change
   *                when the user toggles this value.
   */
  class Bool
  {
  public:
    Bool(bool *pointer) 
      : _generator(nullptr) { _pointer = pointer; };
   
    Bool(bool *pointer, const std::function<Option(Bool&)>& generator) 
      : _generator(generator) { _pointer = pointer; _generator = generator; };

    ~Bool() {}
    void set(bool value) { (*_pointer) = value; };
    bool get() { return *_pointer; };
    std::unique_ptr<Option> sub() { return _generator ? std::make_unique<Option>(_generator(*this)) : nullptr; }
  private:
    bool *_pointer;
    std::function<Option(Bool&)> _generator;

  };

  /**
   * \brief Represents an integer option
   *  @param pointer A pointer to the backing integer that will change
   *                when the user changes this value.
   */
  class Int
  {
  public:
    Int(int *pointer) : _generator(nullptr) { _pointer = pointer; };
    Int(int *pointer, const std::function<Option(Int&)>& generator) 
      : _generator(generator) { _pointer = pointer; _generator = generator; };

    ~Int() {}
    void set(int value) { (*_pointer) = value; };
    int get() { return *_pointer; };
    std::unique_ptr<Option> sub() { return _generator ? std::make_unique<Option>(_generator(*this)) : nullptr; }

  private:
    int *_pointer;
    std::function<Option(Int&)> _generator;
  };

  /**
   * \brief Represents aa string option.
   *  @param pointer A pointer to the backing string that will change
   *                when the user changes this value.
   */
  class Str
  {
  public:
    Str(std::string *pointer) 
      : _generator(nullptr) { _pointer = pointer; };

    Str(std::string *pointer, const std::function<Option(Str&)>& generator) 
      : _generator(generator) { _pointer = pointer; _generator = generator; };
      
    ~Str() {}
    void set(std::string value) { (*_pointer) = value; };
    std::string &get() { return *_pointer; };
    std::unique_ptr<Option> sub() { return _generator ? std::make_unique<Option>(_generator(*this)) : nullptr; }

  private:
    std::string *_pointer;
    std::function<Option(Str&)> _generator;
  };
  
  typedef std::variant<Bool, Int, Str> OptVal;

public:
  /**
 * \brief Default constructor for an option
 *  
 * \remark This should not be called directly, instead, call
 *         SettingsPage::option instead, to avoid orphaning
 *         options.
 * 
 * @param displayName     The display name of the option
 * 
 * @param longDescription The long description shown on the top screen.
 *                        Lines should be broken with the \n delimiter.
 * 
 * @param action          The action that will happen when the user changes or
 *                        selects this option. One of Option::{Sub, Int, Bool, Str}.
 * 
 * @param labels          The label of the types of values that will be shown to the
 *                        user. The label corresponds to the value given in the values
 *                        parameter. Each value must have a corresponding label.
 * 
 * @param values          The possible list of possible values this option can take when
 *                        the user selects the corresponding label.
 */
  Option(const std::string &displayName,
         const std::string &longDescription,
         Option::OptVal action,
         std::initializer_list<std::string> const &labels,
         std::initializer_list<std::variant<bool, int, cstr>> const &values)
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
  OptVal &action() { return _action; }
  std::vector<std::string> &labels() { return _labels; }
  std::vector<std::variant<bool, int, cstr>> &values() { return _values; }

  int selected()
  {
    if (auto value = std::get_if<Bool>(&action()))
    {
      for (int i = 0; i < _values.size(); i++)
      {
        if (auto _value = std::get_if<bool>(&_values[i]))
        {
          if (*_value == value->get())
            return i;
        }
      }
    }

    if (auto value = std::get_if<Int>(&action()))
    {
      for (int i = 0; i < _values.size(); i++)
      {
        if (auto _value = std::get_if<int>(&_values[i]))
        {
          if (*_value == value->get())
            return i;
        }
      }
    }

    if (auto value = std::get_if<Str>(&action()))
    {
       //nocashMessage(value->get().c_str());
      for (int i = 0; i < _values.size(); i++)
      {
        if (auto _value = std::get_if<cstr>(&_values[i]))
        {
          if (value->get().compare(*_value) == 0) return i;
        }
      }
    }
    return -1;
  }

private:
  std::string _displayName;
  std::string _longDescription;
  Option::OptVal _action;
  std::vector<std::string> _labels;
  std::vector<std::variant<bool, int, cstr>> _values;
};

class SettingsPage
{
public:
  SettingsPage(const std::string& title) { _title = title; }
  ~SettingsPage() {}

  /*
 * \brief Adds an option to this settings page.
 * 
 * \remake This should be the preferred way to create an option, and forwards
 *         the arguments to the Option constructor created directly into a 
 *         vector. 
 * 
 * @param displayName     The display name of the option
 * 
 * @param longDescription The long description shown on the top screen.
 *                        Lines should be broken with the \n delimiter.
 * 
 * @param action          The action that will happen when the user changes or
 *                        selects this option. One of Option::{Sub, Int, Bool, Str}.
 * 
 * @param labels          The label of the types of values that will be shown to the
 *                        user. The label corresponds to the value given in the values
 *                        parameter. Each value must have a corresponding label.
 * 
 * @param values          The possible list of possible values this option can take when
 *                        the user selects the corresponding label.
 */
  SettingsPage &option(
      const std::string &displayName,
      const std::string &longDescription,
      Option::OptVal action,
      std::initializer_list<std::string> const &labels,
      std::initializer_list<std::variant<bool, int, cstr>> const &values)
  {
    _options.emplace_back(displayName, longDescription, action, labels, values);
    return *this;
  }

  /*
  * Gets the option this page has.
  */
  std::vector<Option> &options() { return _options; }

  /**
   * Gets the title of the settings page.
   */
  std::string& title() { return _title; }

private:
  std::vector<Option> _options;
  std::string _title;
};

#endif
