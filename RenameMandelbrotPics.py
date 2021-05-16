#ffmpeg requires (I think) images to be in the format 0001.png, 0002.png, 0003.png, and so on.
#The first half of this code was supposed to number each file, but for some reason they did not save as .png files,
#so the second half of the code was written to re-save each image as a .png .

import os

# files = sorted([x for x in os.listdir("/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/imgs/") if x.endswith(".png")],
#     key = lambda x: 1000 * int(x.split("_")[0]) + 100 * int(x.split("_")[1]) + int(x.split("_")[2][:len(x.split("_")[2]) - 4]))
#
# for index, value in enumerate(files):
#     os.rename("/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/imgs/" + value, "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/imgs/" + format(index, '06') + ".png")


files = sorted([x for x in os.listdir("/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/imgs/") if not x.endswith(".DS_Store")])

for x in files:
    os.rename("/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/imgs/" + x, "/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/imgs/" + x + ".png")
