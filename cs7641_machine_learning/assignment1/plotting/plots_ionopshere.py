import matplotlib.pylab as pl
import glob
import numpy as np
import os

figopts = {
    "markeredgecolor": "w",
    "markeredgewidth": 0,
    "linewidth": 2
}


def knn_plots():
    ## Plot of training and test error vs number of neighbors
    datafile = "../results/ionosphere/knn/error_vs_neighbors.csv" 
    fig1 = pl.figure()
    data = np.loadtxt(datafile, skiprows=1, delimiter=",")
    nbrs, etrain, etest = data[:, 0], data[:, 1], data[:, 2]

    pl.plot(nbrs, etrain, "bo", **figopts)
    pl.plot(nbrs, etest, "ro", **figopts)
    pl.xlim([0, 11])
    pl.ylim([0, 40])        
    pl.legend(["train", "test"], frameon=False, loc=4)
    pl.xlabel("neighbors")
    pl.ylabel("% error")    
    pl.savefig("../results/ionosphere/knn/error_vs_nbrs.pdf")

def dtree_plots():
    fig1 = pl.figure()
    fname = "../results/ionosphere/dtree/train_test_error_vs_depth.csv"
    data = np.loadtxt(fname, skiprows=1, delimiter=",")
    depths, etrain, etest = data[:, 0], data[:, 1], data[:, 2]      
        
    pl.plot(depths, etrain, "bo:", **figopts)
    pl.plot(depths, etest, "ro:", **figopts)  
    pl.xlim([0, 10])
    pl.ylim([-9, 40])
    pl.xlabel("tree depth")
    pl.ylabel("% error")
    pl.legend(["train", "test"], frameon=False)
    pl.savefig("../results/ionosphere/dtree/train_test_error_vs_depth.pdf")

def boost_plots():
    fname = "../results/ionosphere/boosting/error_vs_boosting_iterations.csv"
    data = np.loadtxt(fname, skiprows=1, delimiter=",")
    
    err_base_learner = 10.1
    err_large_tree = 23.38

    iters, etest = data[:, 0], 100 * data[:, 2]
    fig1 = pl.figure()
    pl.plot(iters, etest, "g-", **figopts)
    pl.plot(iters, [err_base_learner] * len(iters), "b--", **figopts)
    pl.plot(iters, [err_large_tree] * len(iters), "m--", **figopts)
    pl.xlabel("Boosting iterations")
    pl.ylabel("% error")
    pl.text(100, 1.05 * err_base_learner, "base learner", color="b", fontsize=10)
    pl.text(10, 1.05 * err_large_tree, "single tree with 25 nodes", color="m", fontsize=10)
    pl.text(600, 7, "boosted tree ensemble", color="g", fontsize=10)
    pl.savefig("../results/ionosphere/boosting/error_vs_boosting_iterations.pdf")

def svm_plots():
    root = "../results/ionosphere/svm"
    fig1 = pl.figure(figsize=(10,5))        
    data = np.loadtxt(os.path.join(root, "error_vs_cost.csv"), skiprows=1, delimiter=",")
    cost, etrain, etest = data[:, 0], data[:, 1], data[:, 2]
    
    ax = fig1.add_subplot(121)
    ax.semilogx(cost, etrain, "b-", **figopts)
    ax.semilogx(cost, etest, "r-", **figopts)
    pl.xlabel("Cost parameter")
    pl.ylabel("% error")
    pl.xlim([0.1, 1e5])
    pl.legend(["train", "test"], frameon=False, loc=1)

    data = np.loadtxt(os.path.join(root, "error_vs_gamma.csv"), skiprows=1, delimiter=",")
    gamma, etrain, etest = data[:, 0], data[:, 1], data[:, 2]
    ax2 = fig1.add_subplot(122)
    ax2.semilogx(gamma, etrain, "b-", **figopts)
    ax2.semilogx(gamma, etest, "r-", **figopts)
    pl.xlabel(r"gamma parameter of RBF")
    pl.ylabel("% error")
    pl.xlim([1e-5, 1e1])
    pl.legend(["train", "test"], frameon=False, loc=2)
    pl.savefig(os.path.join(root, "error_vs_cost_gamma.pdf"))        

def nnet_plots():
    root = "../results/ionosphere/nnet"
    data = np.loadtxt(os.path.join(root, "error_vs_hidden_units.csv"),
        skiprows=1, delimiter=",")
    hdn_units, etrain, etest = data[:, 0], data[:, 1], data[:, 2]

    fig1 = pl.figure()
    pl.plot(hdn_units, etrain, "bo", **figopts)
    pl.plot(hdn_units, etest, "ro", **figopts)
    pl.xlabel("hidden units")
    pl.ylabel("% error")
    pl.xlim([0, 120])
    pl.ylim([-3, 22])
    pl.legend(["train", "test"], frameon=False, loc=1)
    pl.savefig(os.path.join(root, "error_vs_hidden_units.pdf"))
    
if __name__ == '__main__':
    knn_plots()    
    dtree_plots()
    boost_plots()
    svm_plots()    
    nnet_plots()


















































