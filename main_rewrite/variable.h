#ifndef VARIABLE_H
#define VARIABLE_H

// Declare classes to friend them
class Battery;
class RX5808;
class Settings;

// Base Variable class with get() and set() access
template<typename T> class Variable {
public:
  Variable(T initialValue = T())
    : value(initialValue) {}

  virtual void set(T newValue) {
    value = newValue;
  }

  virtual T get() const {
    return value;
  }

protected:
  T value;
};

// Variable that runs callback function when value changed
// Most useful for settings variables that need to have side effect when updated
// Callback only called when value changes
template<typename T> class VariableCallback : public Variable<T> {
public:
  using Callback = std::function<void(T)>;

  VariableCallback(T initialValue = T())
    : Variable<T>(initialValue), callback(nullptr) {}

  void set(T newValue) override {
    if (newValue != this->value) {
      this->value = newValue;
      if (callback) callback(this->value);
    }
  }

private:
  void onChange(Callback cb) {
    callback = cb;
  }

  Callback callback;

  // Allow Settings to access onChange()
  friend class Settings;
};

// Variable with restricted set() access
template<typename T> class VariableRestricted : public Variable<T> {
public:
  VariableRestricted(T initialValue = T())
    : Variable<T>(initialValue) {}

private:
  void set(T newValue) override {
    this->value = newValue;
  }

  // Allow classes to access set()
  friend class Battery;
  friend class Settings;
};

// Array variable with restricted set() access
template<typename T, size_t N> class VariableArrayRestricted {
public:
  VariableArrayRestricted(T initialValue = T()) {
    for (size_t i = 0; i < N; i++) {
      values[i] = initialValue;
    }
  }

  T get(size_t index) const {
    return values[index];
  }

  size_t length() const {
    return N;
  }

private:
  void set(size_t index, T newValue) {
    values[index] = newValue;
  }

  T values[N];

  // Allow classes to access set()
  friend class RX5808;
};

#endif
