import threading    # for writing to socket
from signal import signal, SIGINT   # for handle signal SIGINT
from sys import exit
from pynput import keyboard     # for representing keys as chars
from pynput.keyboard import Listener, Key   # for keyboard listener
from time import sleep
import socket       # for TCP
import struct       # for TCP encoding
import ctypes


################################# GLOBAL VARS ##################################
# TCP Setup
HOST = '127.0.0.1'
PORT = 5006
ADDRESS = (HOST, PORT)

map = {
    "BUTTON_A" : 0,
    "BUTTON_B" : 1,
    "BUTTON_X" : 2,
    "BUTTON_Y" : 3,
    "L_BUMPER" : 4,
    "R_BUMPER" : 5,
    "L_TRIGGER" : 6,
    "R_TRIGGER" : 7,
    "BUTTON_BACK" : 8,
    "BUTTON_START" : 9,
    "L_STICK" : 10,
    "R_STICK" : 11,
    "DPAD_UP" : 12,
    "DPAD_DOWN" : 13,
    "DPAD_LEFT" : 14,
    "DPAD_RIGHT" : 15,
    "BUTTON_XBOX" : 16,
    "JOYSTICK_LEFT_X_RIGHT" : 17,
    "JOYSTICK_LEFT_X_LEFT" : 18,
    "JOYSTICK_LEFT_Y_DOWN" : 19,
    "JOYSTICK_LEFT_Y_UP" : 20,
    "JOYSTICK_RIGHT_X_LEFT" : 21,
    "JOYSTICK_RIGHT_X_RIGHT" : 22,
    "JOYSTICK_RIGHT_Y_DOWN" : 23,
    "JOYSTICK_RIGHT_Y_UP" : 24
}
controls = {
        keyboard.KeyCode(char=k) : map["BUTTON_A"],
        keyboard.KeyCode(char=l) : map["BUTTON_B"],
        keyboard.KeyCode(char=j) : map["BUTTON_X"],
        keyboard.KeyCode(char=i) : map["BUTTON_Y"],
        keyboard.KeyCode(char=u) : map["L_BUMPER"],
        keyboard.KeyCode(char=o) : map["R_BUMPER"],
        keyboard.KeyCode(char=y) : map["L_TRIGGER"],
        keyboard.KeyCode(char=p) : map["R_TRIGGER"],
        keyboard.KeyCode(char=n) : map["BUTTON_BACK"],
        keyboard.KeyCode(char=m) : map["BUTTON_START"],
        keyboard.KeyCode(char=q) : map["L_STICK"],
        keyboard.KeyCode(char=e) : map["R_STICK"],
        keyboard.KeyCode(char=t) : map["DPAD_UP"],
        keyboard.KeyCode(char=g) : map["DPAD_DOWN"],
        keyboard.KeyCode(char=f) : map["DPAD_LEFT"],
        keyboard.KeyCode(char=h) : map["DPAD_RIGHT"],
        keyboard.KeyCode(char=z) : map["BUTTON_XBOX"],
        keyboard.KeyCode(char=d) : map["JOYSTICK_LEFT_X_RIGHT"],
        keyboard.KeyCode(char=a) : map["JOYSTICK_LEFT_X_LEFT"],
        keyboard.KeyCode(char=s) : map["JOYSTICK_LEFT_Y_DOWN"],
        keyboard.KeyCode(char=w) : map["JOYSTICK_LEFT_Y_UP"],
        keyboard.Key.left : map["JOYSTICK_RIGHT_X_LEFT"],
        keyboard.Key.right : map["JOYSTICK_RIGHT_X_RIGHT"],
        keyboard.Key.down : map["JOYSTICK_RIGHT_Y_DOWN"],
        keyboard.Key.up : map["JOYSTICK_RIGHT_Y_UP"]
    }

keyboardKeys = [keyboard.KeyCode(char="a"),
                keyboard.KeyCode(char="b"),
                keyboard.KeyCode(char="c"),
                keyboard.KeyCode(char="d"),
                keyboard.KeyCode(char="e"),
                keyboard.KeyCode(char="f"),
                keyboard.KeyCode(char="g"),
                keyboard.KeyCode(char="h"),
                keyboard.KeyCode(char="i"),
                keyboard.KeyCode(char="j"),
                keyboard.KeyCode(char="k"),
                keyboard.KeyCode(char="l"),
                keyboard.KeyCode(char="m"),
                keyboard.KeyCode(char="n"),
                keyboard.KeyCode(char="o"),
                keyboard.KeyCode(char="p"),
                keyboard.KeyCode(char="q"),
                keyboard.KeyCode(char="r"),
                keyboard.KeyCode(char="s"),
                keyboard.KeyCode(char="t"),
                keyboard.KeyCode(char="u"),
                keyboard.KeyCode(char="v"),
                keyboard.KeyCode(char="w"),
                keyboard.KeyCode(char="x"),
                keyboard.KeyCode(char="y"),
                keyboard.KeyCode(char="z"),
                keyboard.KeyCode(char="1"),
                keyboard.KeyCode(char="2"),
                keyboard.KeyCode(char="3"),
                keyboard.KeyCode(char="4"),
                keyboard.KeyCode(char="5"),
                keyboard.KeyCode(char="6"),
                keyboard.KeyCode(char="7"),
                keyboard.KeyCode(char="8"),
                keyboard.KeyCode(char="9"),
                keyboard.KeyCode(char="0"),
                keyboard.KeyCode(char=","),
                keyboard.KeyCode(char="."),
                keyboard.KeyCode(char="/"),
                keyboard.KeyCode(char=";"),
                keyboard.KeyCode(char="'"),
                keyboard.KeyCode(char="["),
                keyboard.KeyCode(char="]"),
                keyboard.Key.left,
                keyboard.Key.right,
                keyboard.Key.up,
                keyboard.Key.down]
#################################### Set Up & Clean up ####################################

def connect_tcp():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print('waiting for server')
    sock.connect(ADDRESS) # connect to server
    print('connected to server')
    return sock # return socket object

def handler(signal_received, frame):
    print("Exiting gracefully")
    exit(0)

#################################### Listening & Writing ####################################

def activate_gamepad_bit(on):
    gamepad_bits | (1 << on)

def deactivate_gamepad_bit(off):
    gamepad_bits & ~(1 << off)

def activate_keyboard_bit(on):
    keyboard_bits | (1 << on)

def deactivate_keyboard_bit(off):
    keyboard_bits & ~(1 << off)

def on_press(key):
    mutex.acquire()
    if key in controls:
        activate_gamepad_bit(controls[key])
    
    if key in keyboardKeys:
        activate_keyboard_bit(keyboardKeys.index(key))

    mutex.release()

def on_release(key):
    mutex.acquire()
    if key in controls:
        deactivate_gamepad_bit(controls[key])

    if key in keyboardKeys:
        deactivate_keyboard_bit(keyboardKeys.index(key))

    elif key == keyboard.Key.esc:
        mutex.release()
        return False # close keyboard listener
    mutex.release()

def keyboard_control():
    writer = threading.Thread(target = write_to_socket)
    writer.start()
    with Listener(on_press = on_press,on_release = on_release) as listener: # assign keyboard actions to functions
        listener.join()

def write_to_socket():
    sock = connect_tcp()
    while(True):
        mutex.acquire()
        sock.send(gamepad_bits) # send the 'bitstring' over tcp using socket object
        sock.send(keyboard_bits)
        mutex.release()
        sleep(0.05) # allows for the listener to modify the bitstring

#################################### Main ####################################

def main():
    signal(SIGINT, handler) 
    global gamepad_bits
    global keyboard_bits
    gamepad_bits = c_ulonglong(0) # 'bitstring' to be modified and sent 
    keyboard_bits = c_ulonglong(0)
    global mutex
    mutex = threading.Lock() # used to avoid race conditions when reading and sending data
    keyboard_control()

if __name__=="__main__":
    main()
