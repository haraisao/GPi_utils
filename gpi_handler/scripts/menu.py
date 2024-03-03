#
import pygame
import pygame_menu
import subprocess
from netifaces import interfaces, ifaddresses, AF_INET

def get_ip_addr(name='wlan0'):
  addresses = [
    i["addr"]
    for i in ifaddresses(name).setdefault(AF_INET, [{"addr": "No IP addr"}])
  ]
  return addresses[0]

#
#
pygame.init()
pygame.mouse.set_visible(False)
width = pygame.display.Info().current_w
height = pygame.display.Info().current_h
surface = pygame.display.set_mode((width, height))

def set_difficulty(value, difficulty):
  pass

def start_the_game():
  pass


####
menu = pygame_menu.Menu("Menu", width, height,
        theme=pygame_menu.themes.THEME_DARK)

menu.add.text_input("Addr: ", default=get_ip_addr())
menu.add.button("Quit", pygame_menu.events.EXIT)

menu.mainloop(surface)

