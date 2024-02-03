import copy
import os
import random
import sys
import time

random.seed()


# -127 - 127
MIN_RANGE = -127
MAX_RANGE = 128


class Char:
    value = [0, 0, 0, 0, 0, 0, 0, 0]

    def __init__(self, v):
        self.value = self.from_int(v)

    def __str__(self):
        return ''.join([str(x) for x in self.value])

    @staticmethod
    def from_int(num):
        if num < -127 or num > 255:
            raise Exception('char value overflow')
        bits = [0, 0, 0, 0, 0, 0, 0, 0]
        # num_str = bin(num).removeprefix('-').removeprefix('0b')  # need python >=3.9
        num_str = bin(num).replace('-', '')[2:]
        res_bit = 7
        for i in num_str[::-1]:
            bits[res_bit] = int(i)
            res_bit -= 1

        if num < 0:
            bits = [abs(x-1) for x in bits]
            tows_comp = int(''.join([str(x) for x in bits[:]]), 2) + 1
            bits = [int(x) for x in bin(tows_comp).replace('-', '')[2:]]
        
        return bits

    def xor(self, v):
        val = self.from_int(v)
        for i in range(8):
            res = self.value[i] + val[i]
            if res > 1:
                self.value[i] = 0
            else:
                self.value[i] = res

    def to_int(self):
        is_negative = self.value[0]

        if is_negative:
            reverse = [abs(x - 1) for x in self.value]
            res = int(''.join([str(x) for x in reverse]), 2) + 1
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


def main(fs=16, dt_filename='datafile.txt', exp_filename='datafile_expected.txt'):
    print(f'{fs}, {dt_filename}, {exp_filename}', file=sys.stderr)
    writen_bytes = 0
    first_value = random_value()
    res_sum = first_value
    res_xor = Char(first_value)
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
            second = Char(value)
            res_xor.xor(value)
            print(
                    f'xor 1 {first_bin} {first_int} ({first_uint})\n' \
                    f'xor 2 {second}     {second.to_int()} ({second.to_uint()})\n' \
                    f'xor = {res_xor}         {res_xor.to_int()} ({res_xor.to_uint()})\n'
                    , file=sys.stderr
                  )
        else:
            writen_bytes += os.write(data_file, str.encode('\n'))
            break

    os.close(data_file)

    res_sub = (2 * first_value) - res_sum
    result_string = f'Summary:     {res_sum}\n' \
                    f'Subtraction: {res_sub}\n' \
                    f'XOR:         {res_xor}, \'{res_xor.to_int()}\'\n'
    
    print(result_string)
    result_file = os.open(exp_filename, attr, mode=0o664)
    os.write(result_file, str.encode(result_string))
    os.close(result_file)


if __name__ == "__main__":
    start_time = time.time()

    file_size = 32
    # file_size = 32
    # file_size = 1024
    # file_size = 1048576
    data_file = 'datafile.txt'
    expect_file = 'datafile_expected.txt'

    if len(sys.argv) == 4:
        file_size = sys.argv[1]
        data_file = sys.argv[2]
        expect_file = sys.argv[3]
    elif len(sys.argv) == 3:
        file_size = sys.argv[1]
        data_file = sys.argv[2]
    elif len(sys.argv) == 2:
        file_size = sys.argv[1]

    print(f'{sys.argv}\n{file_size}, {data_file}, {expect_file}', file=sys.stderr)

    main(fs=int(file_size), dt_filename=data_file, exp_filename=expect_file)

    # ch = Char(25)  # 25 -> '11001'
    # print(f'25 -> {ch}')
    # ch.xor(5)
    # print(f'25^5 = {ch} ', '[', ch.to_int(), ']')  # 25 xor 5 = 28 -> '11001' xor '101' = '11100'

    # print("==== execution time: {:.1f} seconds ====".format(time.time() - start_time), file=sys.stderr)

    exit()
