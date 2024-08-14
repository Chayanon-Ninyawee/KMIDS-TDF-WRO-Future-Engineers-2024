import gpiod
from enum import Enum

class PGPIO:
    VERSION = '0.2'

    class Direction(Enum):
        INPUT = 0
        OUTPUT = 1

    class Pull(Enum):
        NONE = 0
        UP = 1
        DOWN = 2

    def __init__(self):
        self.chip = gpiod.find_line('ID_SDA').owner()  # find chip with ID_SDA
        self.gpiomap = {}  # This is a dictionary to avoid repeated chip.get_line(gpio) calls

    def setup_gpio(self, gpio, direction: Direction, pud: Pull = Pull.NONE):
        """
        Set gpio as an input or an output.
        direction: Direction.INPUT or Direction.OUTPUT
        pud: Pull.NONE, Pull.UP, Pull.DOWN
        """
        line = self.chip.get_line(gpio)
        self.gpiomap[gpio] = line
        if direction == self.Direction.OUTPUT:
            line.request(consumer="p_gpio", type=gpiod.LINE_REQ_DIR_OUT)
        else:
            f = gpiod.LINE_REQ_FLAG_BIAS_DISABLE
            if pud == self.Pull.UP:
                f = gpiod.LINE_REQ_FLAG_BIAS_PULL_UP
            elif pud == self.Pull.DOWN:
                f = gpiod.LINE_REQ_FLAG_BIAS_PULL_DOWN
            line.request("p_gpio", gpiod.LINE_REQ_DIR_IN, f)
            print('pud= ', pud, 'bias= ', line.bias())

    def input_gpio(self, gpio):
        """
        Input from a GPIO channel.
        Returns HIGH=1=True or LOW=0=False.
        """
        return self.gpiomap[gpio].get_value()

    def output_gpio(self, gpio, value):
        """
        Output to a GPIO channel.
        value - 0/1 or False/True or LOW/HIGH.
        """
        self.gpiomap[gpio].set_value(value)

    def gpio_function(self, gpio):
        """
        Returns the current GPIO direction.
        Only works if gpio is in use.
        Returns 0, 1 (IN, OUT).
        """
        if gpio in self.gpiomap:
            return self.gpiomap[gpio].direction() - 1
        return 0

    def get_pullupdn(self, gpio):
        """
        Returns the current GPIO pull.
        Only works if gpio is in use.
        Returns 0: None/Unknown, 1: Up, 2: Down.
        """
        if gpio in self.gpiomap:
            return self.gpiomap[gpio].bias() - 2
        return 0