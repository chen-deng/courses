import numpy as np

QA = np.arange(9).reshape((3,3))
QB = 0.1 * QA

n, m = QA.shape
G = np.zeros((2 * n * (n-1), n * n))


# Process QA
row_num = 0
for i in range(n):
    start_col = start_row = n * i
    end_col = n * (i + 1)
    for j in range(n):
        if i != j:            
            G[row_num, start_col : end_col] = QA[i, :] - QA[j, :]
            row_num += 1

# Process QB
for c1 in range(m):
    for c2 in range(m):
        if c1 != c2:
            Z = np.zeros((n, n))
            Z[:, c1] = QB[:, c1] - QB[:, c2]
            G[row_num, :] = Z.flatten()
            row_num += 1

