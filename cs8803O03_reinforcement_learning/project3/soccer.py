import numpy as np
import random
import matplotlib.pylab as pl
import pdb 
import time 
from cvxopt import matrix, solvers

random.seed(0)
np.random.seed(0)
solvers.options["show_progress"] = False
solvers.options['glpk'] = {'msg_lev': 'GLP_MSG_OFF'}  # cvxopt 1.1.8

class Soccer(object):
    def __init__(self):
        self.timestep = 0
        self.gamma = 0.9
        self.alpha0 = self.alpha = 0.15
        self.alpha_min = 1.0e-3
        self.alpha_decay = 0.9999954
        self.max_steps = 1000
        self.actions = ["N", "W", "E", "S", "T"]
        self.action_idx = dict(zip(self.actions, range(len(self.actions))))
        self.action_pairs = ["%s%s" % (i, j) for i in self.actions 
                                             for j in self.actions]
        self.players = ["A", "B"]
        self.opponent = {"A": "B", "B": "A"}
        self.board = [(i, j) for i in range(1, 3) for j in range(1, 5)]
        self.goalx = {"A": 1, "B": 4}

        self.states =  [(t1, t2, bw) for t1 in self.board 
                                     for t2 in self.board 
                                     for bw in self.players 
                                     if t1 != t2] 

        d = dict.fromkeys(self.action_pairs, tuple())
        self.Q = {s: d.copy() for s in self.states}        
        
        for state, avdict in self.Q.items():
            for av in avdict.keys():
                # avdict[av] = (random.random(), random.random())
                avdict[av] = (1.0, 1.0)

        # Variables to monitor state `s` in the CEQ paper
        self.monitor_state = ((1, 3), (1, 2), "B")        
        self.monitor_obs_max = 1E6
        self.monitor_val = np.zeros((int(self.monitor_obs_max), ))
        self.monitor_timestep = np.zeros((int(self.monitor_obs_max), ))
        self.monitor_obs_cnt = 0


    def initialize(self):
        positions = random.sample([(1, 2), (1, 3), (2, 2), (2, 3)], 2)
        self.pos = dict(zip(self.players, positions))     
        self.ball_with = np.random.choice(self.players)        

    def display(self):
        grid = [["--"] * 4, ["--"] * 4]
        for player, pos in self.pos.items():
            i, j = pos
            grid[i - 1][j - 1] = "%s " % player

        i, j = self.pos[self.ball_with]     
        grid[i - 1][j - 1] = "%s*" % self.ball_with

        print "|" + "|".join(grid[0]) + "|\n|" +  "|".join(grid[1]) + "|"

    def allowed(self, next_pos, player, opponent):
        return (next_pos in self.board) and (self.pos[opponent] != next_pos)

    def get_next_pos(self, pos, action):
        if action == "N":
            return (pos[0] - 1, pos[1])

        if action == "W":
            return (pos[0], pos[1] - 1)

        if action == "E":
            return (pos[0], pos[1] + 1)

        if action == "S":
            return (pos[0] + 1, pos[1])

        if action == "T":
            return pos

    def move(self, player, action, opponent):        
        next_pos = self.get_next_pos(self.pos[player], action)            
        
        if self.allowed(next_pos, player, opponent):            
            self.pos[player] = next_pos

        if (next_pos == self.pos[opponent]) and (self.ball_with == player):
            self.ball_with = opponent

    def done(self):
        for player in self.players:
            if (self.ball_with == player) and (self.pos[player][1] in [1, 4]):
                return True, player, self.pos[player]

        return False, None, None

    def get_state(self):
        return tuple((self.pos["A"], self.pos["B"], self.ball_with))


    def get_matrix(self, state):        
        N = len(self.actions)        
        qma = np.zeros((N, N))
        qmb = np.zeros((N, N))
        # pdb.set_trace()
        for av, qv in self.Q[state].items():
            i, j = [self.action_idx.get(a) for a in list(av)]
            qma[i, j] = qv[0]
            qmb[i, j] = qv[1]
        return qma, qmb

    def foe(self, state, *args):
        N = len(self.actions)
        qma, _ = self.get_matrix(state)
        v1 = np.ones((N, 1))
        v0 = np.zeros((N, 1))
        In = np.eye(N)
        T = np.hstack((-qma.T, -v1))
        Z = np.hstack((-In, v0))

        # Numpy arrays
        G = np.vstack((T, Z))        
        h = np.zeros((G.shape[0], 1))
        A = np.vstack((v1, [[0]])).T
        b = np.array([[1.0]])

        # CVXOPT variables
        c = matrix([0.0, 0.0, 0.0, 0.0, 0.0, 1.0])
        G = matrix(G)
        h = matrix(h)
        A = matrix(A)
        b = matrix(b)

        sol = solvers.lp(c, G, h, A, b, solver="glpk")
        return -sol["x"][-1]

    def ceq(self, state, *args):
        QA, QB = self.get_matrix(state)
        m, n = QA.shape # m rows, n columns

        # Constraint matrix. Not negated yet.
        G = np.zeros((2 * m * (m - 1), m * m))

        # Process m * (m - 1) rationality constraints for A
        row_num = 0
        for r1 in range(m):
            start_col = start_row = m * r1
            end_col = m * (r1 + 1)
            for r2 in range(m):
                if r1 != r2:            
                    G[row_num, start_col : end_col] = QA[r1, :] - QA[r2, :]
                    row_num += 1

        # Process m * (m - 1) rationality constraints for B
        for c1 in range(n):
            for c2 in range(n):
                if c1 != c2:
                    Z = np.zeros((m, n))
                    Z[:, c1] = QB[:, c1] - QB[:, c2]
                    G[row_num, :] = Z.flatten()
                    row_num += 1
        
        # Positivity constraints
        G = np.vstack((G, np.eye(m * m)))   
        h = np.zeros((G.shape[0], 1))

        # Equality (normalization) constraint
        A = np.ones((1, m * m))
        b = np.array([[1.0]])

        # Objective function
        Qt = QA + QB
        c = Qt.flatten()

        # CVXOPT variables
        c = matrix(c)
        G = matrix(-G)
        h = matrix(h)
        A = matrix(A)
        b = matrix(b)

        sol = solvers.lp(c, G, h, A, b, solver=None)
        return -sol["primal objective"]


    def friend(self, state, idx):        
        qvals = self.Q[state].values()
        return max(qvals, key=lambda x: x[idx])[idx], None

         
    def Q_update(self, reward, av, prev_state, next_state):                
        # pdb.set_trace()
        qa, qb = self.Q[prev_state][av]        
        al = self.alpha
        g = self.gamma
        vf = self.ceq
        
        if vf == self.friend:
            va = vf(next_state, 0)
            vb = vf(next_state, 1)
        else:
            va = vb = vf(next_state)

        qa_update = (1.0 - al) * qa + al * ((1.0 - g) * reward["A"] + g * va)
        qb_update = (1.0 - al) * qb + al * ((1.0 - g) * reward["B"] + g * vb)

        self.Q[prev_state][av] = (qa_update, qb_update)
        self.alpha0 *= self.alpha_decay
        self.alpha = max(self.alpha_min, self.alpha0)

        # Record the monitor state
        if (prev_state == self.monitor_state) \
          and (av == "ST") \
          and (self.monitor_obs_cnt < self.monitor_obs_max):
            self.monitor_val[self.monitor_obs_cnt] = qa_update
            self.monitor_timestep[self.monitor_obs_cnt] = self.timestep
            self.monitor_obs_cnt += 1

    def episode(self):      
        self.initialize()        
        self.isdone = False   
        step = 0

        while (not self.isdone) and (step < self.max_steps):
            step += 1
            self.timestep += 1            
            v_action = np.random.choice(self.actions, 2, replace=True)
            players = np.random.permutation(self.players)            
            reward = dict.fromkeys(self.players, 0)
            # time.sleep(0.1)
            prev_state = self.get_state()
            for p, a in zip(players, v_action):
                oppn = self.opponent[p]
                # print "player = %s, action = %s, oppn = %s" % (p, a, oppn)
                self.move(p, a, oppn)
                # self.display()                
                isdone, goalee, position = self.done()                
                if isdone:                    
                    # print "Game end. Ball with: %s, position: %s" % (goalee, position)
                    if (position[1] == self.goalx[goalee]):
                        reward = {goalee: 100, self.opponent[goalee]: -100}
                    else:
                        reward = {goalee: -100, self.opponent[goalee]: 100}
                    # print "reward = %s" % reward
                    self.isdone = True
                    break

            next_state = self.get_state()
            ad = dict(zip(players, v_action))
            av = "%s%s" % (ad["A"], ad["B"])
            self.Q_update(reward, av, prev_state, next_state)


if __name__ == "__main__":
    soccer = Soccer()
    N = 200000
    for i in range(N):
        if i % 1000 == 0:
            print "episode = %d" % i
        soccer.episode()

    tvals = soccer.monitor_timestep[soccer.monitor_timestep>0.]
    mvals = soccer.monitor_val[soccer.monitor_timestep > 0.]
    dvals = np.abs(mvals[:-1] - mvals[1:])

    pl.close("all")
    pl.plot(tvals[1:] / 1.0E5, dvals, "-")
    pl.xlabel(r"Simulation iteration ($10^5$)")
    pl.ylabel("Q-value difference")
    pl.savefig("ceq.pdf", bbox_inches="tight")
    