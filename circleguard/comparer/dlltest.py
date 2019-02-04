import ctypes
from json import loads
from base64 import b64decode


def main():
    dll = ctypes.CDLL('./Debug/comparer.dll')

    dll_b64decode = dll.b64decode
    dll_b64decode.restype = ctypes.c_size_t

    with open('test.json', 'r') as f:
        enc_bytes = bytes(loads(f.read())['content'], 'utf-8')

    int_dec = [int(x) for x in b64decode(enc_bytes)]

    enc_buf = ctypes.create_string_buffer(enc_bytes, len(enc_bytes))
    buf_size = ctypes.c_size_t(len(enc_buf))

    dec_size = dll_b64decode(ctypes.byref(enc_buf), buf_size)

    if dec_size != len(int_dec):
        print('DECODE SIZE MISMATCH')

    for x in range(len(int_dec)):
        if int_dec[x] != ord(enc_buf[x]):
            print('DECODE MISMATCH AT BYTE {}'.formate(x))

    dll_lzmaDecode = dll.lzmaDecode
    dll_lzmaDecode.restype = ctypes.c_size_t

    dec_p = ctypes.POINTER(ctypes.c_uint8)()

    ld_size = dll_lzmaDecode(ctypes.byref(enc_buf), dec_size, ctypes.byref(dec_p))

    print(ld_size)
    dec_arr = ctypes.cast(dec_p, ctypes.c_char_p)
    dec_str = dec_arr.value[:ld_size].decode('utf-8')

    print(dec_str)
    parsed = [x.split('|') for x in dec_str.split(',')[:-1]]

    for i, j in enumerate(parsed):
        if int(j[0]) < 0:
            print(i)


if __name__ == '__main__':
    main()
