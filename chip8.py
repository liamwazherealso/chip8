import pygame, sys
from pygame.locals import *
from subprocess import Popen, PIPE
from threading import Thread
from Queue import Queue, Empty
from os import getcwd
import time
import random

WINDOW_H = 32
WINDOW_W = 64

FPS = 60
FPSCLOCK = pygame.time.Clock()
displaySurface = pygame.display.set_mode((WINDOW_W, WINDOW_H))
pixArray = pygame.PixelArray(displaySurface)

screenDraw = Queue(maxsize=20)  # setup for gathering points to turn on from main.c




class Game():
    def __init__(self):
        pygame.init()
        self.breaker = False

    def stream_watcher(self, identifier, stream):

        for line in stream: # take in string

            if line == 'break\n':  # if the new pixels are all added to the queue, close the thread
                self.breaker = True
            else:
                line = line.split()
                screenDraw.put([int(line[0]), int(line[1])], 1)  # each line has "# #" format"""


    def main(self):
        self.proc = Popen(getcwd() + '/main', stdout=PIPE)
        Thread(target=self.stream_watcher, name='stdout-watcher', args=('STDOUT', self.proc.stdout)).start()
        count = 0

        while True:

            for event in pygame.event.get():
                if event.type == QUIT:
                    self.proc.kill()
                    pygame.quit()
                    sys.exit()

            count = (count + 1) % 10
            if count == 0:
                while not screenDraw.empty():
                    coor = screenDraw.get(True, 1)
                    pixArray[coor[0]][coor[1]] = (random.randint(0,255), random.randint(0,255),random.randint(0,255))  # fun
                    self.breaker = False

            pygame.display.update()
            FPSCLOCK.tick(FPS)


if __name__ == '__main__':
    game = Game()
    game.main()
