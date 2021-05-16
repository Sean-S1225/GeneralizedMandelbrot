#The files were too big for Eclipse to handle, so this program splits them in half.

import json
import os

files = sorted([x for x in os.listdir("/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/") if x.endswith(".json")])

for index, value in enumerate(files):
    name = value[:value.index(".json")]
    print(name)
    fileOrig = open('/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/' + name + '.json', 'r')
    data = json.loads(fileOrig.read())

    file1 = open('/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/' + name + '_0.json', 'w')
    file2 = open('/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/' + name + '_1.json', 'w')

    if len(data['nums']) == 100:
        x1 = {
            "width": 900,
            "height": 900,
            "iterations": 50,
            "nums": data['nums'][0:50]
        }
        x2 = {
            "width": 900,
            "height": 900,
            "iterations": 50,
            "nums": data['nums'][50:]
        }
        file1.write(json.dumps(x1))
        file2.write(json.dumps(x2))
        os.remove('/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/' + name + '.json')
    elif len(data['nums']) == 101:
        x1 = {
            "width": 900,
            "height": 900,
            "iterations": 51,
            "nums": data['nums'][0:51]
        }
        x2 = {
            "width": 900,
            "height": 900,
            "iterations": 50,
            "nums": data['nums'][51:]
        }
        file1.write(json.dumps(x1))
        file2.write(json.dumps(x2))
        os.remove('/Users/Sean/Documents/Coding/Eclipse/Visualize Mandelbrot from C/files/' + name + '.json')
    else:
        print("-----" + str(index) + "-----")
