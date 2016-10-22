import random

class FixedLengthList(object):
    def __init__(self, maxsize):
        self.maxsize = maxsize
        self.lst = list()

    def put(self, item):
        if len(self.lst) >= self.maxsize:
            self.lst.pop(0)
        self.lst.append(item)

    def putlist(self, lst):
        for item in lst:
            self.put(item)

    def sample(self, samplesize):        
        return random.sample(self.lst, min(samplesize, len(self.lst)))


