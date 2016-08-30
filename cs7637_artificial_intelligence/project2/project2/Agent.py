# Your Agent for solving Raven's Progressive Matrices. You MUST modify this file.
#
# You may also create and submit new files in addition to modifying this file.
#
# Make sure your file retains methods with the signatures:
# def __init__(self)
# def Solve(self,problem)
#
# These methods will be necessary for the project's main method to run.

# Install Pillow and uncomment this line to access image processing.
#from PIL import Image

from sets import Set
from collections import defaultdict

class Agent:
    # The default constructor for your Agent. Make sure to execute any
    # processing necessary before your Agent starts solving problems here.
    #
    # Do not add any variables to this signature; they will not be used by
    # main().
    def __init__(self):
        pass

    def flatten(self, fig):        
        descr = Set()
        includeAttrs = ["shape", "size", "height", "angle", "fill"]         
        for _, obj_val in fig.objects.items():            
            for attr in includeAttrs:
                descr.add("%s:%s" % (attr, obj_val.attributes.get(attr)))

        return descr    

    def semantic_diff(self, d1, d2):
        return d1.symmetric_difference(d2)               

    def similarity(self, set1, set2):
        """
        Jaccard similarity
        """
        len_union = len(set1.union(set2))
        if len_union == 0:
            return 1
        else:
            len_intersc = len(set1.intersection(set2))
            return float(len_intersc) / float(len_union)

    def normalize(self, lst):
        sum_ = sum(lst)
        return [float(e)/max(0.01, float(sum_)) for e in lst]

    def round_floats(self, lst):
        return [float("%0.3f" % x) for x in lst]

    # The primary method for solving incoming Raven's Progressive Matrices.
    # For each problem, your Agent's Solve() method will be called. At the
    # conclusion of Solve(), your Agent should return a list representing its
    # confidence on each of the answers to the question: for example 
    # [.1,.1,.1,.1,.5,.1] for 6 answer problems or [.3,.2,.1,.1,0,0,.2,.1] for 8 answer problems.
    #
    # In addition to returning your answer at the end of the method, your Agent
    # may also call problem.checkAnswer(int givenAnswer). The parameter
    # passed to checkAnswer should be your Agent's current guess for the
    # problem; checkAnswer will return the correct answer to the problem. This
    # allows your Agent to check its answer. Note, however, that after your
    # agent has called checkAnswer, it will *not* be able to change its answer.
    # checkAnswer is used to allow your Agent to learn from its incorrect
    # answers; however, your Agent cannot change the answer to a question it
    # has already answered.
    #
    # If your Agent calls checkAnswer during execution of Solve, the answer it
    # returns will be ignored; otherwise, the answer returned at the end of
    # Solve will be taken as your Agent's answer to this problem.
    #
    # Make sure to return your answer *as an integer* at the end of Solve().
    # Returning your answer as a string may cause your program to crash.
    def Solve(self,problem):       

        if problem.problemType == "3x3":
            figList = "ACG"
            choiceList = "12345678"
        elif problem.problemType == "2x2":
            figList = "ABC"
            choiceList = "123456"

        dA, dB, dC = map(self.flatten, map(problem.figures.get, list(figList)))
        
        # Calculate semantic difference between figures A and B
        diffAB = self.semantic_diff(dA, dB)
        diffAC = self.semantic_diff(dA, dC)       

        # Calculate differences between figure C and each of the choices
        answer_choices = map(problem.figures.get, list(choiceList))
        answer_descrs = map(self.flatten, answer_choices)

        # horizontal direction
        hsims = []
        vsims = []
        for idx, dx in enumerate(answer_descrs):
            diffCx = self.semantic_diff(dC, dx)
            diffBx = self.semantic_diff(dB, dx)         
            hsim = self.similarity(diffAB, diffCx)   
            vsim = self.similarity(diffAC, diffBx)
            hsims.append(hsim)
            vsims.append(vsim)
            print "\n%s\nChoice %d\n" % (">" * 60, (idx + 1))
            print "Hdiff: %s\nHdiff: %s\nhsim=%f\n\nVdiff: %s\nVdiff: %s\nvsim=%f\n" \
            % (diffAB, diffCx, hsim, diffAC, diffBx, vsim)            

        simil_overall = [h + v for h, v in zip(hsims, vsims)]
        proba = self.normalize(simil_overall) 
        maxindex = proba.index(max(proba))
        proba2 = [0.0] * len(proba)
        proba2[maxindex] = 1.0
        print "%s: %s" % (problem.name, self.round_floats(proba))
        print "%s: %s" % (problem.name, proba2)
        print "".join(["=="] * 40)
        return proba2
