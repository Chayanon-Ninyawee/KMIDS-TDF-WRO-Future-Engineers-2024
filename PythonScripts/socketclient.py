import socket
import numpy as np
import cv2
import struct

class SocketClient:
    """
    A client class for receiving image and ultrasonic sensor data from a server.

    Attributes:
        ip_address (str): The IP address of the server.
        port (int): The port number of the server.
        image_width (int): The width of the image to be received.
        image_height (int): The height of the image to be received.
        ultrasonic_data_size (int): The size of the ultrasonic sensor data in bytes.
        buffer_size (int): The total size of the buffer for receiving data.
    """

    def __init__(self, ip_address: str, port: int, image_width: int, image_height: int, ultrasonic_data_size: int):
        """
        Initializes the SocketClient with the specified IP address, port, image dimensions, and ultrasonic data size.

        Args:
            ip_address (str): The IP address of the server.
            port (int): The port number of the server.
            image_width (int): The width of the image to be received.
            image_height (int): The height of the image to be received.
            ultrasonic_data_size (int): The size of the ultrasonic sensor data in bytes.
        """
        self.ip_address: str = ip_address
        self.port: int = port
        self.image_width = image_width
        self.image_height = image_height
        self.ultrasonic_data_size = ultrasonic_data_size
        self.buffer_size = self.image_width * self.image_height * 3 + self.ultrasonic_data_size

        self._packet_buffer: bytes = b''

        server_address = (self.ip_address, self.port)

        # Connect to the server
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.connect(server_address)

    def __receive_data(self):
        """
        Receives data from the socket until the buffer size is reached.

        Returns:
            bytes: A packet of data received from the server.
        """
        while True:
            self._packet_buffer += self._sock.recv(self.buffer_size)
            if len(self._packet_buffer) < self.buffer_size:
                continue
            dequeued_packet_buffer = self._packet_buffer[:self.buffer_size]
            self._packet_buffer = self._packet_buffer[self.buffer_size:]
            return dequeued_packet_buffer

    def process_data(self):
        """
        Processes the received data, separating the image and ultrasonic sensor data.

        This method extracts the ultrasonic sensor data from the received packet
        and converts the remaining image data into a NumPy array.
        """
        data = self.__receive_data()

        image_data = data[:-4*4]
        ultrasonic_data = data[-4*4:]

        self._front_ultrasonic = struct.unpack('f', ultrasonic_data[:4])[0]
        self._back_ultrasonic = struct.unpack('f', ultrasonic_data[4:8])[0]
        self._left_ultrasonic = struct.unpack('f', ultrasonic_data[8:12])[0]
        self._right_ultrasonic = struct.unpack('f', ultrasonic_data[12:16])[0]

        # Convert the raw image_data to a numpy array
        self._image = np.frombuffer(image_data, dtype=np.uint8)
        self._image = self._image.reshape((self.image_height, self.image_width, 3))
        self._image = cv2.cvtColor(self._image, cv2.COLOR_RGB2BGR)

    def get_front_ultrasonic(self):
        """
        Returns the front ultrasonic sensor data.

        Returns:
            float: The front ultrasonic sensor data.
        """
        return self._front_ultrasonic

    def get_back_ultrasonic(self):
        """
        Returns the back ultrasonic sensor data.

        Returns:
            float: The back ultrasonic sensor data.
        """
        return self._back_ultrasonic

    def get_left_ultrasonic(self):
        """
        Returns the left ultrasonic sensor data.

        Returns:
            float: The left ultrasonic sensor data.
        """
        return self._left_ultrasonic

    def get_right_ultrasonic(self):
        """
        Returns the right ultrasonic sensor data.

        Returns:
            float: The right ultrasonic sensor data.
        """
        return self._right_ultrasonic

    def get_image(self):
        """
        Returns the received image data.

        Note:
            The returned image is a reference to the internal image array. Modifying
            this image will affect the internal state of the SocketClient instance.
            If you need to modify the image, create a copy first using `image.copy()`.

        Returns:
            np.ndarray: The received image data as a NumPy array.
        """
        return self._image

    def get_image_copy(self):
        """
        Returns a copy of the received image data.

        Returns:
            np.ndarray: A copy of the received image data as a NumPy array.
        """
        return self._image.copy()
