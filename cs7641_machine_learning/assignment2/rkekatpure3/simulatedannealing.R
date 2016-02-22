# Clear workspace
rm(list = ls())

# Set working directory
setwd("/Users/rkekatpure/work/code/coursework/fall2015_machine_learning/assignment2")

# Source common utilities
source("utils.R")
load_ionosphere()

# Load neural network library
library(nnet)
library(GenSA)

# calculate the length of the Wts vector
num_wts <- function(p, M, K) {
    # returns the length of the Wts vector
    #
    # p = ncol(X) # number of predictor variables
    # M = size    # number of hidden units
    # K = 1       # number of classes
    return ((p + 1) * M + K * (M + 1))
}

# Calculate evalfun
# evalfun <- function 

# Get a function 
# Create training and test sets
ntest <- nrow(rtrain$x)
ntrain <- nrow(rtest$x)

# Normalize training and testing data
normFactor <- max(rtrain$x)
nxtrain <- rtrain$x/normFactor
meanFactor <- mean(nxtrain)
nxtrain <- nxtrain - meanFactor
nxtest <- rtest$x/normFactor - meanFactor

cat(sprintf("Training set has %d rows and %d columns\n", nrow(nxtrain), ncol(nxtrain)))
cat(sprintf("Test set has %d rows and %d columns\n", nrow(nxtest), ncol(nxtest)))

# p = number of predictors, K = number of output classes, M = number of hidden units
p <- ncol(nxtrain) 
K <- 2
M <- 10
nwts <- num_wts(p, M, K)
best_maxit <- 1
best_decay <- 0.2

niter <- 20000
fvals <- numeric(niter)
trainerr <- numeric(niter)
testerr <- numeric(niter)
idx <- 1
evalfun <- function(wts) {
    nnmodel <- nnet(x=nxtrain, y = class.ind(rtrain$y),
                    entropy=T,
                    size=M, 
                    decay=best_decay, 
                    maxit=best_maxit,
                    MaxNWts=nwts,
                    Wts=wts,
                    trace=T)
    nnmodel$lev <- c('b', 'g')
    
    ytrain.predicted <- factor(predict(nnmodel, newdata=nxtrain, type="class"), c('b', 'g'))
    train.error <- 100 * sum(ytrain.predicted != rtrain$y)/ntrain 
    
    ypred <- factor(predict(nnmodel, newdata=nxtest, type="class"), c('b', 'g'))
    test.error <- 100 * sum(ypred != rtest$y)/ntest
    
#     cat(sprintf("training error %f, test error = %f\n", 
#                 train.error, test.error))
    trainerr[idx] <<- train.error
    testerr[idx] <<- test.error
    fvals[idx] <<- nnmodel$value
    idx <<- idx + 1
    cat(sprintf("idx = %d, fval = %f\n", idx, nnmodel$value))
    nnmodel$value
    # nnmodel
}

iwts <- runif(nwts, -0.5, 0.5)
# start <- proc.time()
# nnmodel <- evalfun(iwts)
# end <- proc.time()
# elapsed <- end[3] - start[3]
# cat(sprintf("elapsed = %f", elapsed))
# iwts <- nnmodel$wts


lwr <- rep(-5, nwts)
upr <- rep(5, nwts)
set.seed(1234)

start <- proc.time()
out <- GenSA(par=iwts, fn=evalfun, lower=lwr, upper=upr,
             control=list(max.call=niter, temperature=10000))
end <- proc.time()
elapsed <- end[3] - start[3]
cat(sprintf("elapsed = %f", elapsed))

dfresult <- data.frame(fvals=fvals, trainerr=trainerr, testerr=testerr)
write.csv(dfresult, file="simulatedannealing1.csv")


