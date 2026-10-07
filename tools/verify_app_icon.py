"""Check the actual EXE icon group, its artwork bytes and Explorer extraction."""
import ctypes
from pathlib import Path
import struct


def verify_app_icon(exe: Path):
    kernel = ctypes.WinDLL('kernel32', use_last_error=True)
    user = ctypes.WinDLL('user32', use_last_error=True)
    shell = ctypes.WinDLL('shell32', use_last_error=True)
    kernel.LoadLibraryExW.argtypes = [ctypes.c_wchar_p, ctypes.c_void_p, ctypes.c_uint32]
    kernel.LoadLibraryExW.restype = ctypes.c_void_p
    kernel.FindResourceW.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p]
    kernel.FindResourceW.restype = ctypes.c_void_p
    kernel.SizeofResource.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    kernel.SizeofResource.restype = ctypes.c_uint32
    kernel.LoadResource.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    kernel.LoadResource.restype = ctypes.c_void_p
    kernel.LockResource.argtypes = [ctypes.c_void_p]
    kernel.LockResource.restype = ctypes.c_void_p
    kernel.FreeLibrary.argtypes = [ctypes.c_void_p]
    user.DestroyIcon.argtypes = [ctypes.c_void_p]
    shell.ExtractIconExW.argtypes = [ctypes.c_wchar_p, ctypes.c_int,
                                   ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.c_void_p), ctypes.c_uint]
    shell.ExtractIconExW.restype = ctypes.c_uint
    # Map the PE as data, never execute code from the EXE being inspected.
    module = kernel.LoadLibraryExW(str(exe), None, 0x2 | 0x20)
    if not module:
        raise ctypes.WinError(ctypes.get_last_error())

    def resource(identifier, kind):
        found = kernel.FindResourceW(module, identifier, kind)
        if not found:
            raise ctypes.WinError(ctypes.get_last_error())
        length = kernel.SizeofResource(module, found)
        pointer = kernel.LockResource(kernel.LoadResource(module, found))
        assert length and pointer, 'Empty icon resource'
        return ctypes.string_at(pointer, length)

    try:
        source = (Path(__file__).resolve().parents[1] / 'assets/branding/cursorbridge.ico').read_bytes()
        assert struct.unpack_from('<HHH', source) == (0, 1, 9)
        expected = {}
        for index in range(9):
            width, height, _, _, _, _, length, offset = struct.unpack_from('<BBBBHHII', source, 6 + 16 * index)
            expected[(width or 256, height or 256)] = source[offset:offset + length]
        group = resource(101, 14)  # RT_GROUP_ICON
        assert struct.unpack_from('<HHH', group) == (0, 1, 9)
        observed = set()
        for index in range(9):
            width, height, _, _, _, _, length, identifier = struct.unpack_from('<BBBBHHIH', group, 6 + 14 * index)
            size = (width or 256, height or 256)
            data = resource(identifier, 3)  # RT_ICON
            assert len(data) == length and data == expected[size], f'Artwork mismatch at {size}'
            observed.add(size)
        assert observed == set(expected)
    finally:
        kernel.FreeLibrary(module)
    large, little = ctypes.c_void_p(), ctypes.c_void_p()
    try:
        assert shell.ExtractIconExW(str(exe), -1, None, None, 0) == 1, 'Unexpected default icon group'
        result = shell.ExtractIconExW(str(exe), 0, ctypes.byref(large), ctypes.byref(little), 1)
        assert result not in (0, 0xffffffff), 'Explorer icon extraction failed'
        assert large.value and little.value, 'Explorer could not extract both icon sizes'
    finally:
        if large.value:
            user.DestroyIcon(large)
        if little.value:
            user.DestroyIcon(little)
    print('EXE icon verified: nine exact artwork frames; Explorer large/small extraction passed.')
