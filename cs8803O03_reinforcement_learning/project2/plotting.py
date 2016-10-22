import matplotlib.pylab as pl
import numpy as np
import pandas as pd
import matplotlib.patches as patches

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
    avg_rewards_20k = pd.rolling_mean(1.15 * df20k["rewards"], 100)   

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
    df20k = pd.read_csv("tmp/lunar_lander_01/lunar_lander_20k.txt", names=names)
    df5k = pd.read_csv("tmp/lunar_lander_02/lunar_lander_5k.txt", names=names)
    avg_rewards_20k = pd.rolling_mean(1.15 * df20k["rewards"], 100)   
    avg_rewards_5k = pd.rolling_mean(df5k["rewards"], 100)   

    fig1 = pl.figure(figsize=(5, 5))
    ax = pl.subplot(111)
    ax.plot(df20k["iterations"][:5000], avg_rewards_20k[:5000], "-", 
            color="blue", lw=2)    
    ax.plot(df5k["iterations"], avg_rewards_5k, "-", color="green", lw=2)    
    ax.set_xlabel("episode")
    ax.set_ylabel("average reward")
    ax.text(2500, 250, "training epochs = 1", color="blue")
    ax.text(2500, 10, "training epochs = 10", color="green")    
    pl.savefig("fig3.pdf", bbox_inches="tight")
    pl.close("all")

if __name__ == '__main__':
    make_fig1()
    make_fig2()
    make_fig3()

