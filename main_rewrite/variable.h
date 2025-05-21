#ifndef VARIABLE_H
#define VARIABLE_H

// Variable that provides set() and get() methods
template<typename T> class Variable {
public:
  Variable(T initialValue = T())
    : value(initialValue) {}

  void set(T newValue) {
    value = newValue;
  }

  T get() const {
    return value;
  }

private:
  T value;
};

#endif
