import time, timeit

start_time = time.time()


def minimum_adjustments(mesaj, versions, m, n, nr_var):
    dp = [[0 for i in xrange(n + 1)] for i in xrange(m + 1)]

    for i in xrange(m + 1):
        for j in xrange(n + 1):
            if i == 0:
                dp[i][j] = j
            elif j == 0:
                dp[i][j] = i
            else:
                found_similar = False
                for k in xrange(nr_var):
                    if mesaj[i - 1] == versions[k][j - 1]:
                        found_similar = True
                        break
                if found_similar:
                    dp[i][j] = dp[i - 1][j - 1]
                else:
                    dp[i][j] = 1 + min(dp[i][j - 1], dp[i - 1][j], dp[i - 1][j - 1])
    return dp[m][n]


if __name__ == '__main__':
    file_input = open('evaluare.in', 'r')
    file_output = open('evaluare.out', 'w')

    strings = file_input.read().split("\n")

    [NR_VAR, N] = map(int, strings[0].split())

    versions = []
    for i in xrange(1, NR_VAR + 1):
        versions.append(strings[i].split())

    M = int(strings[NR_VAR + 1])
    mesaj = strings[NR_VAR + 2].split()

    file_output.write(str(minimum_adjustments(mesaj, versions, M, N, NR_VAR)))

    file_input.close()
    file_output.close()
print("--- %s seconds ---" % (time.time() - start_time))
