#import ez_profile
import pygame
import random
import calculations

pygame.init()

dis_width = 1200
dis_height = 800
stop_overlapping = False
dis = pygame.display.set_mode((dis_width, dis_height), pygame.RESIZABLE)
pygame.display.set_caption('dots&lines')


def game_loop():
    game_over = False
    clear = True

    rgb = (random.random(), random.random(), random.random())
    
    calculations.set_values(dis_width, dis_height, stop_overlapping)

    while not game_over:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                game_over = True
            if event.type == pygame.VIDEORESIZE:
                w, h = event.size
                calculations.resize(w, h)

        if clear:
            dis.fill((0,0,0))

        #calculates dots position
        dots = calculations.calculate_dots()
                
        #creates list of lines
        lines = calculations.calculate_lines()
            

        #display lines
        for pos1, pos2, shade in lines:
            pygame.draw.line(dis, (shade*rgb[0], shade*rgb[1], shade*rgb[2]), pos1, pos2)


        #displays dots
        for x, y in dots:
            dis.set_at((int(x), int(y)), (255*(1-rgb[0]), 255*(1-rgb[1]), 255*(1-rgb[2])))

        #update
        pygame.display.update()

        #framerate calculation
        calculations.display_fps()
    pygame.quit()
    quit()
game_loop()
