# Clear workspace
rm(list = ls())

# Set working directory
setwd("/Users/rkekatpure/work/code/coursework/fall2015_machine_learning/assignment2")

# Load libraries
library(GenSA)
library(GA)
library(ggplot2)

# define the objective function
fn <- function(x) {
    val <- sin(x) + 0.5 * sin(10 * x)
    val
}

# Simulated annealing minimizes the function, so we need to negate
fn.sa <- function(x) {
    -fn(x)
}

# Genetic algorithm maximizes the function, so we need the function as is
fn.ga <- function(x) {
    fn(x)
}

lwr <- 0
upr <- 1
set.seed(1234)

out.sa <- GenSA(par=0.25, 
             fn=fn.sa, 
             lower=lwr, 
             upper=upr, 
             control=list(verbose=F, 
                          temperature=100)
            )

# out.ga <- ga(type="real-valued", 
#                fn.ga, 
#                min=lwr, 
#                max=upr,
#                maxiter=1000,
#                seed=1234,
#                pmutation=0.1)

xp <- seq(lwr , upr, by=0.001)

# # plot the curve
# plot(xp, fn(xp), type='l')
# 
# # plot the Simulated annealing solution
# points(out.sa$par, -out.sa$value, pch=19, col='red', bg='red')
# 
# # plot the Genetic algorithm solution
# points(slot(out.ga, "solution"), slot(out.ga, "fitnessValue"),
#        pch=4, col='blue', lwd=4)




