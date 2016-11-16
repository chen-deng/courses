import numpy as np
import random
import matplotlib.pylab as pl
import pdb 
import time 

class Soccer(object):
    def __init__(self):
        self.gamma = 0.99
        self.alpha = 1.0e-3
        self.alpha_decay = 0.999
        self.max_steps = 1e6
        self.actions = ["N", "W", "E", "S", "T"]
        self.players = ["A", "B"]
        self.board = [(i, j) for i in range(1, 3) for j in range(1, 5)]

        indices = random.sample([(1, 2), (1, 3), (2, 2), (2, 3)], 2)
        self.pos = dict(zip(self.players, indices))

        self.ball_with = np.random.choice(self.players)

        self.display()

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
        

    def step(self):
        for i in range(100):
            print "step = %d\n" % i                    
            v_action = np.random.choice(self.actions, 2, replace=True)
            players = np.random.permutation(self.players)
            opponents = reversed(players)
            time.sleep(0.1)
            for p, a, o in zip(players, v_action, opponents):
                print "player = %s, action = %s, oppn = %s" % (p, a, o)
                self.move(p, a, o)
                self.display()
                print "\n"
                # if self.game_end():
                #     self.record_reward()

if __name__ == "__main__":
    soccer = Soccer()
    soccer.step()





