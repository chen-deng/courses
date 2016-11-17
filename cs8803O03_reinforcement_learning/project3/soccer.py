import numpy as np
import random
import matplotlib.pylab as pl
import pdb 
import time 
from copy import deepcopy

random.seed(0)
np.random.seed(0)

class Soccer(object):
    def __init__(self):
        self.gamma = 0.7
        self.alpha0 = self.alpha = 0.05
        self.alpha_min = 1.0e-3
        self.alpha_decay = 1.0 - 1.0e-5
        self.max_steps = 1000
        self.actions = ["N", "W", "E", "S", "T"]
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

        # self.initialize()
        # self.display()

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


    # def friend(self, state, idx):        
    #     qvals = self.Q[state].values()
    #     return max(qvals, key=lambda x: x[idx])[idx]

    def friend(self, state, idx):        
        qvals = self.Q[state].values()
        return max(qvals, key=lambda x: x[idx])[idx]

         
    def Q_update(self, reward, av, prev_state, next_state):                
        # pdb.set_trace()
        qa = self.Q[prev_state][av][0]
        qb = self.Q[prev_state][av][1]

        al = self.alpha
        g = self.gamma
        vf = self.friend
        
        qa_update = (1.0 - al) * qa + al * ((1.0 - g) * reward["A"] + g * vf(next_state, 0))
        qb_update = (1.0 - al) * qb + al * ((1.0 - g) * reward["B"] + g * vf(next_state, 1))
        self.Q[prev_state][av] = (qa_update, qb_update)
        # print "updated_values = %0.2f, %0.2f" % (qa_update, qb_update)
        self.alpha0 *= self.alpha_decay
        self.alpha = max(self.alpha_min, self.alpha0)


    def episode(self):      

        self.initialize()        
        self.isdone = False   
        step = 0

        while (not self.isdone) and (step < self.max_steps):
            step += 1
            # print "step = %d" % step
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
            # print "prev_state: " + str(prev_state)
            # print "next_state: " + str(next_state)
            ad = dict(zip(players, v_action))
            av = "%s%s" % (ad["A"], ad["B"])
            self.Q_update(reward, av, prev_state, next_state)



if __name__ == "__main__":
    soccer = Soccer()
    N = 200000
    vals = np.zeros((N,))
    state = ((1, 3), (1, 2), "B")
    for i in range(N):
        if i % 10000 == 0:
            print "episode = %d" % i
        soccer.episode()
        vals[i] = soccer.Q[state]["ST"][0]

    dvals = np.abs(vals[:-1] - vals[1:])
    pl.close("all")
    pl.plot(dvals, "-")
    pl.xlim([0, 1e6])
    pl.ylim([0, 0.5])
    pl.show()




