# Clear workspace
rm(list = ls())

# Set working directory
setwd("/Users/rkekatpure/work/code/coursework/fall2015_machine_learning/assignment2")

# Source common utilities
source("utils.R")
load_ionosphere()

# Load neural network library
library(nnet)
library(GA)

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

niter <- 200000
fvals <- rep(1e-3, niter)
trainerr <- rep(1e-3, niter)
testerr <- rep(1e-3, niter)
idx <- 1
evalfun <- function(wts) {
    nnmodel <- nnet(x=nxtrain, y = class.ind(rtrain$y),
                    entropy=T,
                    size=M, 
                    decay=best_decay, 
                    maxit=best_maxit,
                    MaxNWts=nwts,
                    Wts=wts,
                    trace=F)
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
    if (idx %% 10000 == 0){
        cat(sprintf("idx = %d, fval = %f\n", idx, nnmodel$value))
    }
    -nnmodel$value
}

fnum <- 5
lwr <- rep(-fnum, nwts)
upr <- rep(fnum, nwts)

gaControl("real-valued" = list(crossover="gareal_spCrossover"))
start <- proc.time()
GA <- ga(type="real-valued", 
         evalfun, 
         min=lwr, 
         max=upr,
         maxiter=5000,
         seed=1234,
         pmutation=0.99,
         pcrossover=0.99)

end <- proc.time()
elapsed <- end[3] - start[3]
cat(sprintf("elapsed = %f", elapsed))

dfresult <- data.frame(fvals=fvals[fvals > 1e-3], 
                       trainerr=trainerr[trainerr > 1e-3], 
                       testerr=testerr[testerr > 1e-3])
write.csv(dfresult, file="geneticalgorithm_sp.csv")
