import os
import random
import sys

random.seed()

# -128 ... 127, but for right border or range 127+1
MIN_RANGE = -128
MAX_RANGE = 128


class HwBin:
    value = []
    size = 64

    def __init__(self, v: int = 0, s: int = 64):
        self.size = s
        self.value = self.from_int(v)

    def __str__(self):
        return ''.join([str(x) for x in self.value])

    def from_int(self, num):
        if self.size >= 64 and (num < -9223372036854775808 or num > 9223372036854775807):
            raise Exception('long long value overflow')
        if self.size >= 32 and (num < -2147483648 or num > 2147483647):
            raise Exception('int value overflow')
        if self.size >= 16 and (num < -32768 or num > 32767):
            raise Exception('short value overflow')
        if self.size >= 8 and (num < -256 or num > 255):
            raise Exception('char value overflow')
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
    spaces = random.randrange(1, MAX_RANGE)
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
    result_string = f'Summary:     {res_sum}\n' \
                    f'Subtraction: {res_sub}\n' \
                    f'XOR:         {res_xor.to_int()}, \'{res_xor}\'\n'

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
        file_size = sys.argv[1]

    main(fs=int(file_size_arg), dt_filename=data_file_arg, exp_filename=expect_file_arg)

    exit()
