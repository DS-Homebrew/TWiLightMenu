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
 * Options can have one of Nothing, Boolean, Integer, or String 
 * Actions that "act", or modify directly the value at a given 
 * pointer.
 * 
 * While it is possible to, and indeed Action does do direct ma-
 * nipulation of pointers, Option comes with it a way to define
 * a restricted set of values that an Action is intended to take.
 * 
 * In combination, Option encapsulates the possible values and
 * an accessor to the current value of a settings option.
 * 
 * Actions may also have sub-options and event handlers called 
 * when its value is changed through the Action interface. 
 * 
 * Through the Sub virtual class, Actions may be given a pointer
 * to a function of type OptionGenerator_{T}, which may produce
 * another Option that will be displayed with the SettingsGUI is
 * in 'sub mode' (see SettingsGUI::inSub). The OptionGenerator_{T}
 * is called with the Action that it was associated with it, and
 * can use the Action to determine if a sub option is to be displayed.
 * 
 * In addition, event handlers of type OptionChangedHandler_{T} may
 * be used to cause side effects when the value of the underlying
 * Action has changed. OptionChangedHandler_{T} are only called with
 * the new and previous value whenever Action::set has been called.
 * 
 * Note that changed handlers are only called when the value was
 * changed using the Action::set interface, and not if the under-
 * lying value was changed outside of Action::set. This is useful
 * if a settings option is to be changed, without having to raise
 * any changed handlers.
 * 
 */
class Option
{
public:
  /*
  * Represents an action that may or may not have
  * a sub option. All actions derive from this for
  * polymorphism purposes (see Option::action_sub) 
  */
  class Sub
  {
  public:

    /**
     * Gets a unique_ptr to an Option instance that is
     * the sub option to be displayed. The unique_ptr
     * may be nullptr if has_sub is false.
     */
    virtual std::unique_ptr<Option> sub() = 0;

    /**
     * Whether or not this option has a sub option.
     */
    virtual bool has_sub() = 0;
  };

  /* \brief Represents an action with no valid selections.
   * Although the Nul action can not modify data, it is
   * still a first-class Action, with changed handlers and
   * option generators that can be used as usual.
   */
  class Nul : public Sub
  {
  public:
    typedef std::optional<Option> (*OptionGenerator_Nul)(void);
    typedef void (*OptionChangedHandler_Nul)(void);

    Nul()
        : _generator(nullptr), _changed(nullptr){};

    Nul(const OptionGenerator_Nul generator, const OptionChangedHandler_Nul changed)
        : _generator(generator), _changed(changed)
    {
      _generator = generator;
      _changed = changed;
    };

    Nul(const OptionGenerator_Nul generator)
        : _generator(generator), _changed(nullptr)
    {
      _generator = generator;
    };

    Nul(const OptionChangedHandler_Nul changed)
        : _generator(nullptr), _changed(changed)
    {
      _changed = changed;
    };

    ~Nul() {}

    std::unique_ptr<Option> sub()
    {
      if (!_generator)
        return nullptr;
      auto option = _generator();
      if (!option.has_value())
        return nullptr;
      return std::make_unique<Option>(*option);
    }
    bool has_sub() { return _generator != nullptr; }
    void set()
    {
      if (_changed)
        _changed();
    };

    bool was_modified() { return false; }
  private:
    OptionGenerator_Nul _generator;
    OptionChangedHandler_Nul _changed;
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
    typedef void (*OptionChangedHandler_Bool)(bool, bool);

    //typedef std::function<Option(Bool&)> OptionGenerator_Bool;
    Bool(bool *pointer)
        : _generator(nullptr), _changed(nullptr) { _pointer = pointer; _startValue = *pointer; };

    Bool(bool *pointer, const OptionGenerator_Bool generator)
        : _generator(generator), _changed(nullptr)
    {
      _pointer = pointer;
      _startValue = *pointer;
      _generator = generator;
    };

    Bool(bool *pointer, const OptionGenerator_Bool generator, const OptionChangedHandler_Bool changed)
        : _generator(generator), _changed(changed)
    {
      _pointer = pointer;
      _startValue = *pointer;
      _generator = generator;
      _changed = changed;
    };

    Bool(bool *pointer, const OptionChangedHandler_Bool changed)
        : _generator(nullptr), _changed(changed)
    {
      _pointer = pointer;
      _startValue = *pointer;
      _changed = changed;
    };

    ~Bool() {}
    void set(bool value)
    {
      if (_changed)
        _changed(bool(*_pointer), value);
      (*_pointer) = value;
    };

    bool get() { return *_pointer; };
    bool was_modified() { return _startValue != *_pointer; };

    std::unique_ptr<Option> sub()
    {
      if (!_generator)
        return nullptr;
      auto option = _generator(*this);
      if (!option.has_value())
        return nullptr;
      return std::make_unique<Option>(*option);
    }
    bool has_sub() { return _generator != nullptr; }

  private:
    bool *_pointer;
    bool _startValue;
    OptionGenerator_Bool _generator;
    OptionChangedHandler_Bool _changed;
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
    typedef void (*OptionChangedHandler_Int)(int, int);

    //typedef std::function<Option(Int&)> OptionGenerator_Int;
    Int(int *pointer) : _generator(nullptr), _changed(nullptr) { _pointer = pointer; _startValue = *pointer;};
    Int(int *pointer, const OptionGenerator_Int generator)
        : _generator(generator)
    {
      _pointer = pointer;
      _startValue = *pointer;
      _generator = generator;
    };

    Int(int *pointer, const OptionGenerator_Int generator, const OptionChangedHandler_Int changed)
        : _generator(generator), _changed(changed)
    {
      _pointer = pointer;
      _startValue = *pointer;
      _generator = generator;
      _changed = changed;
    };

    Int(int *pointer, const OptionChangedHandler_Int changed)
        : _generator(nullptr), _changed(changed)
    {
      _pointer = pointer;
      _startValue = *pointer;
      _changed = changed;
    };

    ~Int() {}
    void set(int value)
    {
      if (_changed)
        _changed(int(*_pointer), value);
      (*_pointer) = value;
    };
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
    bool has_sub() { return _generator != nullptr; }

    bool was_modified() { return _startValue != *_pointer; };
  private:
    int *_pointer;
    int _startValue;
    OptionGenerator_Int _generator;
    OptionChangedHandler_Int _changed;
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
    typedef void (*OptionChangedHandler_Str)(std::string, std::string);

    //typedef std::function<Option(Str&)> OptionGenerator_Str;
    Str(std::string *pointer)
        : _generator(nullptr), _changed(nullptr) { _pointer = pointer; _startString = *pointer; };

    Str(std::string *pointer, const OptionGenerator_Str generator)
        : _generator(generator), _changed(nullptr)
    {
      _pointer = pointer;
      _startString = *pointer;
      _generator = generator;
    };

    Str(std::string *pointer, const OptionGenerator_Str generator, const OptionChangedHandler_Str changed)
        : _generator(generator), _changed(changed)
    {
      _pointer = pointer;
      _startString = *pointer;
      _generator = generator;
      _changed = changed;
    };

    Str(std::string *pointer, const OptionChangedHandler_Str changed)
        : _generator(nullptr), _changed(changed)
    {
      _pointer = pointer;
      _startString = *pointer;
      _changed = changed;
    };

    ~Str() {}
    void set(std::string value)
    {
      if (_changed)
        _changed(std::string(*_pointer), value);

      _was_modified = _startString != value;
      (*_pointer) = value;
    };

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
    bool has_sub() { return _generator != nullptr; }
    bool was_modified() { return _was_modified; }
  private:
    std::string *_pointer;
    std::string _startString;
    bool _was_modified = false;
    OptionGenerator_Str _generator;
    OptionChangedHandler_Str _changed;
  };

  /**
   * Algebraic type definition for Action.
   * 
   * Action is one of Bool, Int, or Str. Note
   * that all of Bool, Int, Str inherit from
   * Sub, and thus have an is-a relationship with
   * Option::Sub, but Option::Action is not, and 
   * can not be Option::Sub without matching
   * the variant type.
   */
  typedef std::variant<Bool, Int, Str, Nul> Action;

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
         Option::Action action,
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
         std::vector<std::string> const &values)
      : _action(stringAction)
  {
    _displayName = displayName;
    _longDescription = longDescription;
    _action = stringAction;

    for (auto& str : values)
    {
      _labels.emplace_back(str);
      _values.emplace_back(str.c_str());
    }
  }

  ~Option() {}

  std::string &displayName() { return _displayName; }
  std::string &longDescription() { return _longDescription; }
  Action &action() { return _action; }
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
    else if (auto action = std::get_if<Int>(&_action))
    {
      return *action;
    }
    else if (auto action = std::get_if<Str>(&_action))
    {
      return *action;
    }
    else
    {
      return *std::get_if<Nul>(&_action);
    }
  }

  int selected()
  {
    if (auto value = std::get_if<Bool>(&action()))
    {
      for (int i = 0; i < ((int)_values.size()); i++)
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
      for (unsigned int i = 0; i < _values.size(); i++)
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
      for (unsigned int i = 0; i < _values.size(); i++)
      {
        if (auto _value = std::get_if<cstr>(&_values[i]))
        {
          if (value->get().compare(*_value) == 0)
            return i;
        }
      }
    }
    if (std::get_if<Nul>(&action()))
    {
      return 0;
    }
    return -1;
  }

private:
  std::string _displayName;
  std::string _longDescription;
  Option::Action _action;
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
      Option::Action action,
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
      std::vector<std::string> const &values)
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
