"""Generate CursorBridge's original, MIT-licensed semantic CUR/ANI themes.

No external images, fonts, game assets or image libraries are used.
"""
from pathlib import Path
import argparse
import math
import struct

NAMES = ['normal.cur', 'selected.cur', 'dragselect.cur', 'grab.cur', 'grabbing.cur',
         'busy.ani', 'no_move.ani', 'friendly_move.ani', 'attack_move.ani']
PALETTES = [(245, 248, 255), (70, 215, 255), (255, 205, 65), (245, 100, 225)]
SIZE = 48

def polygon(x, y, points):
    inside = False
    previous = points[-1]
    for current in points:
        ax, ay = previous; bx, by = current
        if (ay > y) != (by > y) and x < (bx-ax)*(y-ay)/(by-ay)+ax:
            inside = not inside
        previous = current
    return inside

def cur(shape, palette, resource, frame=0):
    ink = PALETTES[palette]
    if resource == 7: ink = (80, 255, 145)
    if resource == 8: ink = (255, 100, 105)
    if resource == 6: ink = (255, 175, 70)
    if resource >= 6 and frame % 2: ink = tuple(max(70, int(c * .75)) for c in ink)
    pixels = {}
    for y in range(SIZE):
        for x in range(SIZE):
            dx, dy = x-23.5, y-23.5; distance = math.hypot(dx, dy)
            if resource == 5:
                angle = (math.atan2(dy, dx) + math.pi*2 - frame*math.pi/4) % (math.pi*2)
                visible = 12 <= distance <= 18 and angle < math.pi*1.55
            elif shape == 0:
                visible = polygon(x+.5, y+.5, [(2,2),(2,35),(11,27),(19,43),(25,40),(17,24),(30,24)])
            elif shape == 1:
                visible = (abs(dx)<=2 and 4<=abs(dy)<=19) or (abs(dy)<=2 and 4<=abs(dx)<=19)
            else:
                visible = 14<=distance<=17 or (abs(dx)<1.5 and abs(dy)<1.5)
            if visible: pixels[x,y] = ink
    # State badges remain distinguishable in every shape and color.
    if resource not in (0,5):
        for y in range(32,44):
            for x in range(33,45):
                dx,dy=x-39,y-38
                visible = False
                if resource in (1,7): visible = abs(dx)<=1 or abs(dy)<=1  # selected / friendly
                elif resource in (6,8): visible = abs(dx-dy)<=1 or abs(dx+dy)<=1 # blocked / attack
                elif resource == 2: visible = abs(dx)>=4 or abs(dy)>=4 # selection box
                elif resource == 3: visible = abs(dx)<=4 and abs(dy)<=4 and (dy>=0 or x%3==0)
                elif resource == 4: visible = abs(dx)<=4 and abs(dy)<=3 # closed grab
                if visible: pixels[x,y]=ink
    outline = set()
    for x,y in pixels:
        for oy in (-1,0,1):
            for ox in (-1,0,1):
                if 0<=x+ox<SIZE and 0<=y+oy<SIZE and (x+ox,y+oy) not in pixels:
                    outline.add((x+ox,y+oy))
    dibpixels=bytearray(); mask=bytearray()
    for y in reversed(range(SIZE)):
        row=bytearray(((SIZE+31)//32)*4)
        for x in range(SIZE):
            rgb = pixels.get((x,y), (8,12,20) if (x,y) in outline else None)
            dibpixels += bytes((rgb[2],rgb[1],rgb[0],255)) if rgb else bytes(4)
            if rgb is None: row[x//8] |= 1<<(7-x%8)
        mask += row
    # 23/48 remains centered while Win32's 1px rounding stays inside the image.
    hotspot=(2,2) if shape==0 and resource!=5 else (23,23)
    dib=struct.pack('<IiiHHIIiiII',40,SIZE,SIZE*2,1,32,0,len(dibpixels),0,0,0,0)+dibpixels+mask
    return struct.pack('<HHH',0,2,1)+struct.pack('<BBBBHHII',SIZE,SIZE,0,0,*hotspot,len(dib),22)+dib

def chunk(kind,data):
    return kind+struct.pack('<I',len(data))+data+(b'\0' if len(data)%2 else b'')

def ani(shape,palette,resource):
    frames=8 if resource==5 else 2
    header=struct.pack('<9I',36,frames,frames,SIZE,SIZE,32,1,8,1)
    data=b'ACON'+chunk(b'anih',header)+chunk(b'LIST',b'fram'+b''.join(chunk(b'icon',cur(shape,palette,resource,f)) for f in range(frames)))
    return b'RIFF'+struct.pack('<I',len(data))+data

def generate(output):
    for shape in range(3):
        for palette in range(4):
            directory=output/str(1+shape*4+palette);directory.mkdir(parents=True,exist_ok=True)
            for resource,name in enumerate(NAMES):
                (directory/name).write_bytes(cur(shape,palette,resource) if resource<5 else ani(shape,palette,resource))

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output',type=Path)
    generate(parser.parse_args().output)
