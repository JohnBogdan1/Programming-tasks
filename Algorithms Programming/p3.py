import operator


def minimum_length(cablaj):
    one_list = []
    zero_list = []
    minimum_length = 0
    pins = []
    h = 0
    # creez un tuplu
    h_max = [0, 0, 1000]
    for i in xrange(len(cablaj)):

        if cablaj[i] == '1':
            if not zero_list:
                h = 0
                one_list.append(i)
            elif zero_list:
                x = zero_list.pop()

                if x < h_max[1] and i > h_max[2]:
                    h = h_max[0] + 1
                else:
                    h += 1
                if h >= h_max[0]:
                    h_max[0] = h
                    h_max[1] = x
                    h_max[2] = i

                minimum_length += abs(i - x) + 2 * h
                pins.append([x + 1, i + 1])

        elif cablaj[i] == '0':
            if not one_list:
                h = 0
                zero_list.append(i)
            elif one_list:
                x = one_list.pop()

                if x < h_max[1] and i > h_max[2]:
                    h = h_max[0] + 1
                else:
                    h += 1
                if h >= h_max[0]:
                    h_max[0] = h
                    h_max[1] = x
                    h_max[2] = i

                minimum_length += abs(i - x) + 2 * h
                pins.append([x + 1, i + 1])

    return minimum_length, pins


if __name__ == '__main__':
    file_input = open('cablaj.in', 'r')
    file_output = open('cablaj.out', 'w')

    strings = file_input.read().split("\n")

    nr_pins = int(strings[0])

    cablaj = strings[1]

    result = minimum_length(cablaj)

    minimum_units = result[0]

    pins = result[1]
    pins = sorted(pins, key=operator.itemgetter(0))

    file_output.write(str(minimum_units) + "\n")
    for i in xrange(nr_pins):
        file_output.write(str(pins[i][0]) + " " + str(pins[i][1]) + "\n")

    file_input.close()
    file_output.close()
