#ifndef VARIABLE_CALLBACK_H
#define VARIABLE_CALLBACK_H

// Declare Settings to friend it
class Settings;

// Variable that runs callback function when value changed
// Most useful for settings variables that need to have side effect when updated
// Callback only called when value changes
template<typename T> class VariableCallback {
public:
  using Callback = std::function<void(T)>;

  VariableCallback(T initialValue = T())
    : value(initialValue), callback(nullptr) {}

  void set(T newValue) {
    if (newValue != value) {
      value = newValue;
      if (callback) callback(value);
    }
  }

  T get() const {
    return value;
  }

private:
  void onChange(Callback cb) {
    callback = cb;
  }

  T value;
  Callback callback;

  // Allow Settings to access onChange()
  friend class Settings;
};

#endif
