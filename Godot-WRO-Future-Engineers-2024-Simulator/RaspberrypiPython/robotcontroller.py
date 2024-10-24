import cv2
import time

import robotdata
import pgpio

from openchallenge import process_data_open
from obstaclechallenge import process_data_obstacle

from config import *
from utils import *

robot_data = robotdata.RobotData(CAMERA_WIDTH, CAMERA_HEIGHT)
gpio = pgpio.PGPIO()

def main():
    global robot_data

    try:
        # TODO: Check ultrasonic_info if it have any error

        gpio.setup_gpio(13, gpio.Direction.INPUT, gpio.Pull.UP)
        time.sleep(1)

        while(gpio.input_gpio(13) == 1):
            time.sleep(0.010)

        time.sleep(0.8)

        robot_data.re_init_serial()
        last_update_time = time.time()
        try:
            while True:
                current_time = time.time()
                delta_time = current_time - last_update_time
                last_update_time = current_time

                robot_data.process_data()

                full_image = robot_data.get_image()
                image = full_image[IMAGE_HEIGHT:, :]

                ultrasonic_info = robot_data.get_ultrasonic_data()
                gyro_info = robot_data.get_gyro_data()

                if not delta_time == 0:
                    #print(f'{1/delta_time} {ultrasonic_info}')
                    pass

                # result = process_data_open(ultrasonic_info, gyro_info, image, delta_time)
                result = process_data_obstacle(ultrasonic_info, gyro_info, image, delta_time)
                # result = (0, 0)

                speed_target = 0
                steering_percent = 0 
                if type(result) is bool:
                    if result == False:
                        robot_data.send_data(0.0, 0.0)
                        break
                else:
                    speed_target, steering_percent = result

                robot_data.send_data(speed_target, steering_percent)

        except Exception as e: print(e)
        finally:
            cv2.destroyAllWindows()
            robot_data.close()
    except Exception as e:
        print(e)
        cv2.destroyAllWindows()
        robot_data.close()

if __name__ == "__main__":
    main()
