"""Original synthetic CUR/ANI fixtures; never copy game assets to CI or releases."""
from pathlib import Path
import argparse
import struct

def cursor(frame=0,variant=0):
    size = 32
    pixels = bytearray()
    mask = bytearray()
    for y in reversed(range(size)):
        row = bytearray(4)
        for x in range(size):
            visible = (2 <= x <= 10 and 2 <= y <= 22 and x <= 2 + y//2)
            visible |= (frame == 1 and 16 <= x <= 25 and 8 <= y <= 15)
            pixels += bytes((220 if frame else 90, 190, 30+variant*20, 255)) if visible else bytes(4)
            if not visible:
                row[x//8] |= 1 << (7-x%8)
        mask += row
    dib = struct.pack('<IiiHHIIiiII', 40, size, size*2, 1, 32, 0, len(pixels), 0, 0, 0, 0) + pixels + mask
    return struct.pack('<HHH', 0, 2, 1) + struct.pack('<BBBBHHII', size, size, 0, 0, 3, 4, len(dib), 22) + dib

def chunk(kind, data):
    return kind + struct.pack('<I', len(data)) + data + (b'\0' if len(data) % 2 else b'')

def animated(variant=0):
    header = struct.pack('<9I', 36, 2, 2, 32, 32, 32, 1, 6, 1)
    data = b'ACON' + chunk(b'anih', header) + chunk(b'LIST', b'fram' + chunk(b'icon', cursor(1,variant)) + chunk(b'icon', cursor(2,variant)))
    return b'RIFF' + struct.pack('<I', len(data)) + data

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('directory',type=Path)
    args=parser.parse_args()
    target=args.directory.resolve()
    files=target/'gfx'/'cursors'
    files.mkdir(parents=True,exist_ok=True)
    for variant,name in enumerate(('normal','selected','dragselect','grab','grabbing')):
        (files/f'{name}.cur').write_bytes(cursor(variant=variant))
    for variant,name in enumerate(('busy','no_move','friendly_move','attack_move'),5):
        (files/f'{name}.ani').write_bytes(animated(variant))
    # Layout discovery fixture; not run and never used as an injection target.
    (target/'stellaris.exe').write_bytes(b'CursorBridge layout fixture; not executable')
    print('Generated original synthetic fixtures.')

if __name__=='__main__':
    main()
