import os
import random
import sys

random.seed()

TRACE = False

USE_SIZE = 'unsigned int'

SIZES_MTRX = {
    'long long': {
        'MIN': -9223372036854775808,
        'MAX': 9223372036854775807,
        'SIZE': 64,
        },
    'unsigned long long': {
        'MIN': 0,
        'MAX': 18446744073709551615,
        'SIZE': 64,
        },
    'int': {
        'MIN': -2147483648,
        'MAX': 2147483647,
        'SIZE': 32,
        },
    'unsigned int': {
        'MIN': 0,
        'MAX': 4294967295,
        'SIZE': 32,
        },
    'short': {
        'MIN': -32768,
        'MAX': 32767,
        'SIZE': 16,
        },
    'unsigned short': {
        'MIN': 0,
        'MAX': 65535,
        'SIZE': 16,
        },
    'char': {
        'MIN': -128,
        'MAX': 127,
        'SIZE': 8,
        },
    'unsigned char': {
        'MIN': 0,
        'MAX': 255,
        'SIZE': 8,
        },
}

SIZE      = SIZES_MTRX[USE_SIZE]['SIZE']
MIN_RANGE = SIZES_MTRX[USE_SIZE]['MIN']
MAX_RANGE = SIZES_MTRX[USE_SIZE]['MAX']
ERROR_MSG = f'{USE_SIZE} value overload'

class HwBin:
    value = []
    size = 64

    def __init__(self, v: int = 0, s: int = SIZE):
        self.size = s
        self.value = self.from_int(v)

    def __str__(self):
        return ''.join([str(x) for x in self.value])

    def from_int(self, num):

        if self.size <= SIZE and (num < MIN_RANGE or MAX_RANGE < num):
            raise Exception(ERROR_MSG)

        bits = [0 for _ in range(self.size)]
        # num_str = bin(num).removeprefix('-').removeprefix('0b')  # need python >=3.9
        num_str = bin(num).replace('-', '')[2:]
        res_bit = self.size - 1
        for i in num_str[::-1]:
            bits[res_bit] = int(i)
            res_bit -= 1

        if num < 0:
            bits = [abs(x - 1) for x in bits]
            tows_comp = int(''.join([str(x) for x in bits[:]]), 2) + 1
            bits = [int(x) for x in bin(tows_comp).replace('-', '')[2:]]

        return bits

    def xor(self, v):
        res = [x ^ y for x, y in zip(self.value, self.from_int(v))]
        self.value = res

    def to_int(self):
        if USE_SIZE.count('unsigned'):  # crutch
            return self.to_uint()

        is_negative = self.value[0]
        if is_negative:
            invert = [abs(x - 1) for x in self.value]
            res = int(''.join([str(x) for x in invert]), 2) + 1
            res = -res
        else:
            res = int(''.join([str(x) for x in self.value[:]]), 2)
        return res

    def to_uint(self):
        res = int(''.join([str(x) for x in self.value[:]]), 2)
        return res


def random_nl():
    spaces = random.randrange(1, 255)
    return spaces


def random_value():
    value = random.randint(MIN_RANGE, MAX_RANGE)
    return value


def main(fs=32, dt_filename='datafile.txt', exp_filename='datafile_expected.txt'):
    writen_bytes = 0
    first_value = random_value()
    res_sum = first_value
    res_xor = HwBin(first_value)
    attr = os.O_WRONLY | os.O_CREAT | os.O_TRUNC
    data_file = os.open(dt_filename, attr, mode=0o664)
    writen_bytes += os.write(data_file, str.encode(f'{first_value}'))
    print(f'first_value: {first_value}, res_sum:{res_sum}, res_xor {res_xor} "{res_xor.to_int()}"\n', file=sys.stderr)
    count_spaces = random_nl()
    while True:
        value = random_value()

        if count_spaces > 0:
            count_spaces -= 1
            str_value = f' {value}'
        else:
            count_spaces = random_nl()
            str_value = f'\n{value}'

        if writen_bytes + len(str_value) + 1 < fs:
            writen_bytes += os.write(data_file, str.encode(str_value))
            res_sum = res_sum + value
            first_int = res_xor.to_int()
            first_uint = res_xor.to_uint()
            first_bin = f'{res_xor}'
            second = HwBin(value)
            res_xor.xor(value)
            if TRACE:
                print(
                    f'xor 1 {first_bin} {first_int} ({first_uint})\n'
                    f'xor 2 {second}     {second.to_int()} ({second.to_uint()})\n'
                    f'xor = {res_xor}         {res_xor.to_int()} ({res_xor.to_uint()})\n',
                    file=sys.stderr
                )
        else:
            writen_bytes += os.write(data_file, str.encode('\n'))
            break

    os.close(data_file)
    res_sub = (2 * first_value) - res_sum
    res_xor_64 = HwBin(res_xor.to_int(), 64)
    result_string = f'Summary:     {res_sum}\n' \
                    f'Subtraction: {res_sub}\n' \
                    f'XOR:         {res_xor.to_int()}, \'{res_xor_64}\'\n'

    print(result_string)
    result_file = os.open(exp_filename, attr, mode=0o664)
    os.write(result_file, str.encode(result_string))
    os.close(result_file)


if __name__ == "__main__":

    file_size_arg = 32
    data_file_arg = 'datafile.txt'
    expect_file_arg = 'datafile_expected.txt'

    if len(sys.argv) == 4:
        file_size_arg = sys.argv[1]
        data_file_arg = sys.argv[2]
        expect_file_arg = sys.argv[3]
    elif len(sys.argv) == 3:
        file_size_arg = sys.argv[1]
        data_file_arg = sys.argv[2]
    elif len(sys.argv) == 2:
        file_size_arg = sys.argv[1]

    main(fs=int(file_size_arg), dt_filename=data_file_arg, exp_filename=expect_file_arg)

    exit()
