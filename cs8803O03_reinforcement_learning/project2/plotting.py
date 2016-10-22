import matplotlib.pylab as pl
import numpy as np
import pandas as pd
import matplotlib.patches as patches
from sklearn.neural_network import MLPRegressor


def make_fig1():
    names = ["iterations", "rewards"]
    df20k = pd.read_csv("tmp/lunar_lander_01/lunar_lander_20k.txt", names=names)    

    fig1 = pl.figure(figsize=(5, 5))
    ax = pl.subplot(111)
    ax.plot(df20k["iterations"][:5000], df20k["rewards"][:5000], "-", 
            color="gray", lw=0.5)    
    ax.set_xlabel("episode")
    ax.set_ylabel("raw reward")
    ax.text(3500, -474, r"$\epsilon = 1.00$", fontsize=12)    
    ax.text(3500, -550, r"$\epsilon_\mathrm{decay} = 0.99$", fontsize=12)    
    ax.text(3500, -625,r"$\gamma = 0.99$", fontsize=12)
    ax.text(3500, -700,r"$\alpha = 10^{-4}$", fontsize=12)
    pl.savefig("fig1.pdf", bbox_inches="tight")
    pl.close("all")

def make_fig2():
    names = ["iterations", "rewards"]
    df20k = pd.read_csv("tmp/lunar_lander_01/lunar_lander_20k.txt", names=names)
    avg_rewards_20k = pd.rolling_mean(df20k["rewards"], 100)   

    fig1 = pl.figure(figsize=(5, 5))
    ax = pl.subplot(111)
    ax.plot(df20k["iterations"], avg_rewards_20k, "-", color="blue", lw=2)    
    ax.add_patch(patches.Rectangle((1000, 150), 13000, 100, 
                                   alpha=0.3, color="gray"))
    ax.set_xlabel("episode")
    ax.set_ylabel("average reward")
    ax.text(6000, 260, r"trained", fontsize=12)
    ax.text(14500, -75, r"overfitting", fontsize=12)
    pl.savefig("fig2.pdf", bbox_inches="tight") 
    pl.close("all")   

def make_fig3():
    names = ["iterations", "rewards"]
    # df20k = pd.read_csv("tmp/lunar_lander_01/lunar_lander_20k.txt", names=names)
    df20k = pd.read_csv("tmp/lunar_lander_03/lunar_lander_5k.txt", names=names)
    df5k = pd.read_csv("tmp/lunar_lander_02/lunar_lander_5k.txt", names=names)
    avg_rewards_20k = pd.rolling_mean(df20k["rewards"], 100)   
    avg_rewards_5k = pd.rolling_mean(df5k["rewards"], 100)   

    fig1 = pl.figure(figsize=(12, 5))
    ax = pl.subplot(121)
    ax.plot(df20k["iterations"][:5000], avg_rewards_20k[:5000], "-", 
            color="blue", lw=2)    
    ax.plot(df5k["iterations"], avg_rewards_5k, "-", color="green", lw=2)    
    ax.set_xlabel("episode", size=18)
    ax.set_ylabel("average reward", size=18)
    ax.text(2500, 220, "training epochs = 2", color="blue")
    ax.text(2500, 10, "training epochs = 10", color="green")    
    ax.tick_params(labelsize=16)

    np.random.seed(1)
    ssize = 256
    ones = np.ones((ssize, 1))
    x = np.linspace(-1, 1, ssize).reshape((ssize, 1))
    y = -0.5 + 2 * np.sin(2 * np.pi * x) + np.random.random((ssize, 1))
    X = np.hstack((ones, x))

    regressor = MLPRegressor(hidden_layer_sizes=(50, 50),
                             activation="relu",
                             batch_size=128,
                             max_iter=2,
                             solver="adam")

    alpha_list = np.logspace(-6, -1, 1000)
    loss = np.zeros(alpha_list.shape)
    
    for idx, alpha_t in enumerate(alpha_list):    
        regressor.learning_rate_init = alpha_t
        regressor.fit(X, y)
        loss[idx] = regressor.loss_
        print "learning rate = %0.6f, loss_ = %0.3f" % (alpha_t, loss[idx])   

    yp = regressor.predict(X)

    ax = fig1.add_subplot(122)
    ax.semilogx(alpha_list, loss)
    ax.set_xlabel("learning rate", size=18)
    ax.set_ylabel("fitting loss", size=18)
    ax.tick_params(labelsize=16)

    fig1.savefig("fig0.pdf", bbox_inches="tight")
    pl.close("all")

if __name__ == '__main__':
    make_fig1()
    make_fig2()
    make_fig3()

