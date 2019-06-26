def min_number_of_switches(sir):
    nr_switches = 0
    nr_distinct_vars = 0

    while len(sir) != 0:
        x = sir.rfind(sir[0])
        if x != 0:
            nr_switches += len(sir) - 1 - x
            sir = sir[:0] + sir[1:]
            sir = sir[:x - 1] + sir[x:]
        elif x == 0:
            nr_distinct_vars += 1
            nr_switches += (len(sir)) / 2
            sir = sir[:0] + sir[1:]
            if nr_distinct_vars > 1:
                return str(-1) + "\n"

    return str(nr_switches) + "\n"


if __name__ == '__main__':
    file_input = open('joc.in', 'r')
    file_output = open('joc.out', 'w')

    strings = file_input.read().split("\n")

    nr = int(strings[0])

    for i in xrange(1, nr + 1):
        file_output.write(min_number_of_switches(strings[i]))

    file_input.close()
    file_output.close()
