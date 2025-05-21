#ifndef CALLBACK_VARIABLE_H
#define CALLBACK_VARIABLE_H

// Variable that runs callback function when value changed
// Most useful for settings variables that need to have side effect when updated
// Callback only called when value changes
template<typename T> class CallbackVariable {
public:
  using Callback = std::function<void(T)>;

  CallbackVariable(T initialValue = T())
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

  void onChange(Callback cb) {
    callback = cb;
  }

private:
  T value;
  Callback callback;
};

#endif
