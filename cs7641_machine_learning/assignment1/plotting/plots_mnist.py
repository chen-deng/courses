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
    datafiles = glob.glob("../results/mnist/knn/output_Ntrain*")
    datafiles.sort(key=os.path.getmtime)

    fig1 = pl.figure()

    subplots = [221, 222, 223, 224]
    subfig_labels = ["a", "b", "c", "d"]

    for df, sp, sf in zip(datafiles, subplots,subfig_labels):
        ax = fig1.add_subplot(sp)
        data = np.loadtxt(df, skiprows=1, delimiter=",")
        nbrs, etrain, etest = data[:, 0], data[:, 1], data[:, 2]
        ax.plot(nbrs, etrain, "bo", **figopts)
        ax.plot(nbrs, etest, "ro", **figopts)

        pl.xlim([0, 11])
        pl.ylim([0, 20])        
        if sp == 221:
            pl.legend(["train", "test"], frameon=False, loc=4)
        else:    
            pl.legend(["train", "test"], frameon=False)
        pl.text(1, 17, "(%s)" % sf)
        if sp in [223, 224]:
            pl.xlabel("neighbors")

        if sp in [221, 223]:
            pl.ylabel("% error")
    
    pl.savefig("../results/mnist/knn/error_vs_nbrs.pdf")

    ## Plot of minimum error vs dataset size    
    training_size = [600, 1200, 3000, 6000]
    train_errors = []
    test_errors = []

    for df in datafiles:
        data = np.loadtxt(df, skiprows=1, delimiter=",")
        train_errors.append(data[-1, 1])
        test_errors.append(data[-1, 2])
    
    fig2 = pl.figure()
    pl.plot(training_size, train_errors, "bs", **figopts)
    pl.plot(training_size, test_errors, "rs", **figopts)
    pl.xlabel("training size")
    pl.ylabel("% error")
    pl.xlim([0, 7000])
    pl.ylim([0, 20])
    pl.savefig("../results/mnist/knn/error_vs_training_size.pdf")

def dtree_plots():
    subplots = [221, 222, 223, 224]
    subfig_labels = ["a", "b", "c", "d"]
    fig1 = pl.figure()
    root = "../results/mnist/dtree"

    ## Error evolution with tree depth
    datafiles = [
        "train_test_error_vs_depth_Ntrain6000_unnorm.csv",
        "train_test_error_vs_depth_Ntrain12000_unnorm.csv",
        "train_test_error_vs_depth_Ntrain30000_unnorm.csv",
        "train_test_error_vs_depth_Ntrain60000_unnorm.csv"
    ]

    for df, sp, sf in zip(datafiles, subplots, subfig_labels):
        fname = os.path.join(root, df)
        data = np.loadtxt(fname, skiprows=1, delimiter=",")
        depths, etrain, etest = data[:, 0], data[:, 1], data[:, 2]      
        
        ax = fig1.add_subplot(sp)
        ax.plot(depths, etrain, "bo", **figopts)
        ax.plot(depths, etest, "ro", **figopts)
        
        pl.xlim([0, 32])
        pl.ylim([-9, 100])
        pl.text(2, 90, "(%s)" % sf)
        if sp in [223, 224]:
            pl.xlabel("tree depth")

        if sp in [221, 223]:
            pl.ylabel("% error")

        pl.legend(["train", "test"], frameon=False)
        pl.savefig(os.path.join(root, "error_vs_depth.pdf"))

    ## Error evolution with number of nodes
    datafiles = [
        "train_test_error_vs_cp_Ntrain6000_unnorm.csv",
        "train_test_error_vs_cp_Ntrain60000_unnorm.csv"
    ]
    
    pl.close()
    fig2 = pl.figure(figsize=(10,5))
    subplots = [121, 122]
    subfig_labels = ["a", "b", "c", "d"]

    for df, sp, sf in zip(datafiles, subplots, subfig_labels):
        fname = os.path.join(root, df)
        data = np.loadtxt(fname, skiprows=1, delimiter=",")
        cp, nodes, etrain, etest = data[:, 0], data[:, 1], data[:, 2], data[:, 3]
        
        ax = fig2.add_subplot(sp)
        ax.plot(nodes, etrain, 'bo', **figopts)
        ax.plot(nodes, etest, 'ro', **figopts)
        pl.text(100, 90, "(%s)" % sf)
        pl.xlim([-50, 2000])
        pl.ylim([0, 100])
        pl.xlabel("nodes")
        pl.ylabel("% error")
        pl.legend(["train", "test"], frameon=False)

    pl.savefig(os.path.join(root, "error_vs_nodes.pdf"))

def svm_plots():
    root = "../results/mnist/svm"
    dfiles = [
        "error_vs_training_size_cost0.1.txt",
        "error_vs_training_size_cost1.txt",
        "error_vs_training_size_cost10.txt",
        "error_vs_training_size_cost100000.txt",
    ]

    subplots = [221, 222, 223, 224]
    subfig_labels = ["a", "b", "c", "d"]    
        
    fig1 = pl.figure()        
    for df, sp, sf in zip(dfiles, subplots, subfig_labels):
        fname = os.path.join(root, df)
        data = np.loadtxt(fname, skiprows=1, delimiter=",")
        size, etrain, etest = data[:, 0], data[:, 1], data[:, 2]
        ax = fig1.add_subplot(sp)
        ax.plot(size/1000, etrain, "bo", **figopts)
        ax.plot(size/1000, etest, "ro", **figopts)

        pl.xlim([0, 7])
        if sp == 221:
            pl.ylim([-1, 20])            
            yt = 18
            loc = 4
        else:
            pl.ylim([-1, 20])            
            yt = 18
            loc = 1

        pl.text(0.2, yt, "(%s)" % sf)

        if sp in [223, 224]:
            pl.xlabel("Training set size (1000)")

        if sp in [221, 223]:
            pl.ylabel("% error")

    pl.legend(["train", "test"], frameon=False, loc=loc)
    pl.savefig(os.path.join(root, "error_vs_training_size.pdf"))        

    ## Polynomial kernel: Error vs Kernel degree
    fig2 = pl.figure(figsize=(10,5))
    data = np.loadtxt(os.path.join(root, "error_vs_kernel_degree.txt"), skiprows=1, delimiter=",")
    deg, etrain, etest = data[:, 0], data[:, 1], data[:, 2]

    ax = fig2.add_subplot(121)
    ax.plot(deg, etrain, "bo", **figopts)
    ax.plot(deg, etest, "ro", **figopts)
    pl.xlim(0, 9)
    pl.ylim(-2, 50)
    pl.xlabel("polynomial kernel degree")
    pl.ylabel("% error")
    pl.text(0.5, 45, "(a)")
    # pl.legend(["train", "test"], frameon=False, loc=1)

    data = np.loadtxt(os.path.join(root, "error_vs_kernel_gamma.txt"), skiprows=1, delimiter=",")
    gamma, etrain, etest = data[:, 0], data[:, 1], data[:, 2]

    ax = fig2.add_subplot(122)
    ax.semilogx(gamma, etrain, "bo", **figopts)
    ax.semilogx(gamma, etest, "ro", **figopts)

    pl.xlim([1e-6, 1])
    pl.ylim([-2, 50])
    pl.xlabel("RBF Kernel gamma")
    pl.ylabel("% error")
    pl.text(2e-6, 45, "(b)")
    pl.legend(["train", "test"], frameon=False, loc=1)

    pl.savefig(os.path.join(root, "error_vs_complexity.pdf"))                

def boost_plots():
    root = "../results/mnist/boosting"
    data = np.loadtxt(os.path.join(root, "error_vs_iterations_Ntrain=6000_Ntrees=200.txt"),
        skiprows=1, delimiter=",")
    
    err_weak_learner = 32.4
    err_large_tree = 20.7

    iters, etest = data[:, 0], 100 * data[:, 2]
    fig1 = pl.figure()
    pl.plot(iters, etest, "g-", **figopts)
    pl.plot(iters, [err_weak_learner] * len(iters), "b--", **figopts)
    pl.plot(iters, [err_large_tree] * len(iters), "m--", **figopts)
    pl.xlabel("Boosting iterations")
    pl.ylabel("% error")
    pl.text(100, 32.5, "base learner (50 nodes)", color="b", fontsize=10)
    pl.text(0, 21, "tree with 409 nodes", color="m", fontsize=10)
    pl.text(15, 25, "boosted tree ensemble", color="g", fontsize=10)
    pl.savefig(os.path.join(root, "error_vs_boosting_iters.pdf"))

def nnet_plots():
    root = "../results/mnist/nnet"
    data = np.loadtxt(os.path.join(root, "error_vs_hidden_units_Ntrain600.txt"),
        skiprows=1, delimiter=",")
    hdn_units, etrain, etest = data[:, 0], data[:, 1], data[:, 2]

    fig1 = pl.figure(figsize=(10,5))
    ax1 = fig1.add_subplot(121)
    ax1.plot(hdn_units, etrain, "bo", **figopts)
    ax1.plot(hdn_units, etest, "ro", **figopts)
    pl.xlabel("hidden units")
    pl.ylabel("% error")
    pl.xlim([0, 40])
    pl.ylim([-3, 22])
    pl.legend(["train", "test"], frameon=False, loc=1)
    data = np.loadtxt(os.path.join(root, "error_vs_training_size_hiddenunits20.txt"),
        skiprows=1, delimiter=",")
    ntrain, etrain, etest = data[:, 0], data[:, 1], data[:, 2]

    ax2 = fig1.add_subplot(122)
    ax2.semilogx(ntrain, etrain, "bo", **figopts)
    ax2.semilogx(ntrain, etest, "ro", **figopts)
    pl.xlabel("training set size")
    pl.ylabel("% error")
    pl.xlim([200, 100000])
    pl.ylim([-3, 22])
    pl.legend(["train", "test"], frameon=False, loc=1)
    pl.savefig(os.path.join(root, "error_vs_hidden_units_and_training_size.pdf"))

    
if __name__ == '__main__':
    # knn_plots()    
    # dtree_plots()
    # svm_plots()
    # boost_plots()
    nnet_plots()


















































