#It was taking too much CPU power for the Java program to continue adding files to
#a folder with several thousand other items, so periodically I ran this code to
#move all the files to a second folder.

import os

files = sorted([x for x in os.listdir("/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/imgs/") if x.endswith(".png")])

for x in files:
    os.rename("/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/imgs/" + x, "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/imgs2/" + x)
