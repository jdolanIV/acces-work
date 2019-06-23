import ctypes
import time

_handle = ctypes.CDLL('./libaiowatchdog.so')

def aio_watchdog_library_init():
  return _handle.aio_watchdog_library_init()

def aio_watchdog_library_term():
  _handle.aio_watchdog_library_term()

def aio_watchdog_reinit(card):
  return _handle.aio_watchdog_reinit(ctypes.c_int(card))

def aio_watchdog_period_set(card, milliseconds):
  return _handle.aio_watchdog_period_set(ctypes.c_int(card), ctypes.c_int(milliseconds))

def aio_watchdog_start(card):
  return _handle.aio_watchdog_start(ctypes.c_int(card))

def aio_watchdog_pet(card):
  return _handle.aio_watchdog_pet(ctypes.c_int(card))

def aio_watchdog_stop(card):
  return _handle.aio_watchdog_stop(ctypes.c_int(card))

def aio_watchdog_pulse_width_set(card, pulse_width):
  return _handle.aio_watchdog_pulse_width_set(ctypes.c_int(card), ctypes.c_int(pulse_width))

def aio_watchdog_temp_read(card):
  temperature = ctypes.c_double()
  status = _handle.aio_watchdog_temp_read(ctypes.c_int(card), ctypes.byref(temperature))
  return (status, temperature.value)

def aio_watchdog_humidity_read(card):
  humidity = ctypes.c_double()
  status = _handle.aio_watchdog_humidity_read(ctypes.c_int(card), ctypes.byref(humidity))
  return (status, humidity.value)

if __name__ == "__main__":
  WATCHDOG_PERIOD=5000
  WATCHDOG_PET_INTERVAL=3
  WATCHDOG_NUM_PETS=3
  WATCHDOG_PULSE_WIDTH=254

  print("Initializing aio_watchdog library\n")
  status = aio_watchdog_library_init()
  print ("Found %d watchdog cards.\n" % status)
  if status != 0:
    print("Reinitalizing watchdog card\n")
    aio_watchdog_reinit(0)
    time.sleep(2)
    print("Setting watchdog period to %s\n" % WATCHDOG_PERIOD)
    aio_watchdog_period_set (0, WATCHDOG_PERIOD)
    print("Setting watchdog pulse width\n")
    aio_watchdog_pulse_width_set(0, 128)
    print("Starting watchdog\n")
    aio_watchdog_start(0)

    for x in range (0, WATCHDOG_NUM_PETS):
      (status, degrees) = aio_watchdog_temp_read(0)
      print("degrees = %f\n" % degrees)
      (status, humidity) = aio_watchdog_humidity_read(0)
      print("humidiy = %f\n" % humidity)
      time.sleep(WATCHDOG_PET_INTERVAL)
      print("Petting watchdog\n")
      aio_watchdog_pet(0)
    aio_watchdog_library_term()