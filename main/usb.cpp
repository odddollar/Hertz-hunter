#include "usb.h"

#ifdef BATTERY_MONITORING
UsbSerial::UsbSerial(Settings *s, RX5808 *r, Battery *b)
  : settings(s), receiver(r), battery(b)
#else
UsbSerial::UsbSerial(Settings *s, RX5808 *r)
  : settings(s), receiver(r)
#endif
{
}

// Start serial connection
void UsbSerial::beginSerial(unsigned long baud) {
  // Don't start connection if already running
  if (serialOn) return;

  Serial.begin(baud);
}

// Start listening for commands
void UsbSerial::listen() {

}
