#ifndef VARIABLE_RESTRICTED_H
#define VARIABLE_RESTRICTED_H

// Declare Settings to friend it
class Settings;

// Variable that provides get() method
// set() method only available to permitted classes
template<typename T> class VariableRestricted {
public:
  VariableRestricted(T initialValue = T())
    : value(initialValue) {}

  T get() const {
    return value;
  }

private:
  void set(T newValue) {
    value = newValue;
  }

  T value;

  // Allow Settings to access set()
  friend class Settings;
};

#endif
