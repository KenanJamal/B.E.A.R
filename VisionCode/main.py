import os
import cv2
import json
nameofthefolder = "japanese"
folder = f"braill-images/{nameofthefolder}"
w = 300
h = 200
letters = []

for file_name in sorted(os.listdir(folder)):

    ##color conversion to greyScale so what matter the data is (we now handle img)
    path = os.path.join(folder, file_name)
    img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
    if img is None:
        continue
    ##thresholding (which is making every thing either black or white if its under 127 then it's black
    ##if not then it's 255 which is white)
    _, bw = cv2.threshold(img, 127, 255, cv2.THRESH_BINARY)
    if bw.shape != (300, 200):
        img = cv2.resize(img, (200, 300))

    h, w = bw.shape
    values = []
    for row in range(3):
        row_values = []
        for col in range(2):
            y1 = row * (h //3)
            y2 = y1 + h // 3
            x1 = col * (w //2)
            x2 = x1 + w // 2
            section = bw[y1: y2,x1:x2]
            if(section.mean() < 128):
                row_values.append(1)
            else:
                row_values.append(0)
        values.append(row_values)
    nameOfTheFile, ex = os.path.splitext(file_name)
    entry = {"name": nameOfTheFile, "value": values}
    letters.append(entry)


    print(values , nameOfTheFile)
with open(f"output/{nameofthefolder}.json", "w", encoding="utf-8") as f:
    json.dump(letters, f, ensure_ascii=False, indent=2)