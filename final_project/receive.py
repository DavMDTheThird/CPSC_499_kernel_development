import serial

# Set up serial connection to the USB UART device
ser = serial.Serial('/dev/ttyUSB0', baudrate=9600, timeout=1)  # Adjust to correct port

# Wait for data and print it when received
while True:
    if ser.in_waiting > 0:
        data = ser.readline().decode('utf-8').strip()  # Read data from the UART
        print(f"Received data: {data}")
        
ser.close()