from cvxopt import matrix, solvers

solvers.options["show_progress"] = True

c = matrix([0., 0., 0., 1.])
G = matrix([[0., 1., -1., -1., 0, 0],
            [-1., 0., 1. ,0., -1., 0],
            [1., -1., 0., 0., 0., -1.],
            [-1., -1., -1., 0., 0., 0.]])

h = matrix([0., 0., 0., 0., 0., 0.])

A = matrix([[1.], [1.], [1.], [0.] ])
b = matrix([[1.]])

sol = solvers.lp(c, G, h, A, b, solver="glpk")
print sol["x"]