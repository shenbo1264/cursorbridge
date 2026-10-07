"""Encode the selected, unchanged square master as Windows icon frames.

Pillow is needed only to regenerate the checked-in ICO, not to build the app.
"""
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SIZES = (16, 20, 24, 32, 40, 48, 64, 128, 256)


def main():
    directory = ROOT / 'assets/branding'
    with Image.open(directory / 'cursorbridge-a1.png') as master:
        if master.width != master.height:
            raise ValueError('The selected master must be square')
        master.convert('RGBA').save(directory / 'cursorbridge.ico', format='ICO',
                                    sizes=[(size, size) for size in SIZES])
    with Image.open(directory / 'cursorbridge.ico') as icon:
        assert icon.ico.sizes() == {(size, size) for size in SIZES}
    print('Encoded nine Windows icon sizes; original master unchanged.')


if __name__ == '__main__':
    main()
