#include <variant>
#include <string>
#include <vector>
#include <algorithm>
#include <nds.h>
#include <functional>
#include <memory>
#include <optional>
#pragma once
#ifndef __DSIMENUPP_SETTINGS_PAGE_H_
#define __DSIMENUPP_SETTINGS_PAGE_H_
typedef const char *cstr;

/**
 * \brief Represents a settings option with an associated INI 
 *        entry that can be displayed to the user.
 * 
 */
class Option
{
public:
  /*
  * Represents an option that may or may not have
  * a sub option. 
  */
  class Sub
  {
  public:
    virtual std::unique_ptr<Option> sub() = 0;
  };

  /**
   * \brief Represents a boolean option
   * 
   * @param pointer A pointer to the backing boolean that will change
   *                when the user toggles this value.
   */
  class Bool : public Sub
  {
  public:
    typedef std::optional<Option> (*OptionGenerator_Bool)(Bool &);
    //typedef std::function<Option(Bool&)> OptionGenerator_Bool;
    Bool(bool *pointer)
        : _generator(nullptr) { _pointer = pointer; };

    Bool(bool *pointer, const OptionGenerator_Bool generator)
        : _generator(generator)
    {
      _pointer = pointer;
      _generator = generator;
    };

    ~Bool() {}
    void set(bool value) { (*_pointer) = value; };
    bool get() { return *_pointer; };
    std::unique_ptr<Option> sub()
    {
      if (!_generator)
        return nullptr;
      auto option = _generator(*this);
      if (!option.has_value())
        return nullptr;
      return std::make_unique<Option>(*option);
    }

  private:
    bool *_pointer;
    OptionGenerator_Bool _generator;
  };

  /**
   * \brief Represents an integer option
   *  @param pointer A pointer to the backing integer that will change
   *                when the user changes this value.
   */
  class Int : public Sub
  {
  public:
    typedef std::optional<Option> (*OptionGenerator_Int)(Int &);
    //typedef std::function<Option(Int&)> OptionGenerator_Int;
    Int(int *pointer) : _generator(nullptr) { _pointer = pointer; };
    Int(int *pointer, const OptionGenerator_Int generator)
        : _generator(generator)
    {
      _pointer = pointer;
      _generator = generator;
    };

    ~Int() {}
    void set(int value) { (*_pointer) = value; };
    int get() { return *_pointer; };
    std::unique_ptr<Option> sub()
    {
      if (!_generator)
        return nullptr;
      auto option = _generator(*this);
      if (!option.has_value())
        return nullptr;
      return std::make_unique<Option>(*option);
    }

  private:
    int *_pointer;
    OptionGenerator_Int _generator;
  };

  /**
   * \brief Represents aa string option.
   *  @param pointer A pointer to the backing string that will change
   *                when the user changes this value.
   */
  class Str : public Sub
  {
  public:
    typedef std::optional<Option> (*OptionGenerator_Str)(Str &);
    //typedef std::function<Option(Str&)> OptionGenerator_Str;
    Str(std::string *pointer)
        : _generator(nullptr) { _pointer = pointer; };

    Str(std::string *pointer, const OptionGenerator_Str generator)
        : _generator(generator)
    {
      _pointer = pointer;
      _generator = generator;
    };

    ~Str() {}
    void set(std::string value) { (*_pointer) = value; };
    std::string &get() { return *_pointer; };
    std::unique_ptr<Option> sub()
    {
      if (!_generator)
        return nullptr;
      auto option = _generator(*this);
      if (!option.has_value())
        return nullptr;
      return std::make_unique<Option>(*option);
    }

  private:
    std::string *_pointer;
    OptionGenerator_Str _generator;
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
         std::initializer_list<std::string> const labels,
         std::initializer_list<std::variant<bool, int, cstr>> const values)
      : _action(action)
  {
    _displayName = displayName;
    _longDescription = longDescription;
    _action = action;
    _labels = std::vector(std::move(labels));
    _values = std::vector(std::move(values));
  }

  Option(const std::string &displayName,
         const std::string &longDescription,
         Option::Str stringAction,
         std::vector<cstr> const &values)
      : _action(stringAction)
  {
    _displayName = displayName;
    _longDescription = longDescription;
    _action = stringAction;

    for (cstr str : values)
    {
      _labels.emplace_back(str);
      _values.emplace_back(str);
    }
  }

  ~Option() {}

  std::string &displayName() { return _displayName; }
  std::string &longDescription() { return _longDescription; }
  OptVal &action() { return _action; }
  std::vector<std::string> &labels() { return _labels; }
  std::vector<std::variant<bool, int, cstr>> &values() { return _values; }

  /**
   * Gets this option's action as an abstract Sub.
   */
  Option::Sub &action_sub()
  {
    if (auto action = std::get_if<Bool>(&_action))
    {
      return *action;
    }
    if (auto action = std::get_if<Int>(&_action))
    {
      return *action;
    }
    if (auto action = std::get_if<Str>(&_action))
    {
      return *action;
    }
  }

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
          if (value->get().compare(*_value) == 0)
            return i;
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
  SettingsPage(const std::string &title) { _title = title; }
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
      std::initializer_list<std::string> const labels,
      std::initializer_list<std::variant<bool, int, cstr>> const values)
  {
    _options.emplace_back(displayName, longDescription, action, labels, values);
    return *this;
  }

  SettingsPage &option(
      const std::string &displayName,
      const std::string &longDescription,
      Option::Str stringAction,
      std::vector<cstr> const &values)
  {
    _options.emplace_back(displayName, longDescription, stringAction, values);
    return *this;
  }

  /*
  * Gets the option this page has.
  */
  std::vector<Option> &options() { return _options; }

  /**
   * Gets the title of the settings page.
   */
  std::string &title() { return _title; }

private:
  std::vector<Option> _options;
  std::string _title;
};

#endif
