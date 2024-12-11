import RPi.GPIO as GPIO
import time
import serial

# Pin configuration
TX_PIN = 14  # GPIO14 (TX)
RX_PIN = 15  # GPIO15 (RX)

# Set up GPIO pins
GPIO.setmode(GPIO.BCM)
GPIO.setup(TX_PIN, GPIO.OUT)
GPIO.setup(RX_PIN, GPIO.IN)

# Set up the serial connection to the USB UART device
ser = serial.Serial('/dev/ttyUSB0', baudrate=9600, timeout=1)  # Adjust to correct port

# Sending a message via UART (TX pin)
message = "Hello from Raspberry Pi GPIO!"
ser.write(message.encode())  # Send data as bytes over the UART connection

print("Data sent: ", message)

# Clean up and close serial connection
ser.close()
GPIO.cleanup()