# Clear workspace
rm(list = ls())

# Set working directory
setwd("/Users/rkekatpure/work/code/coursework/fall2015_machine_learning/assignment1")

# Source common utilities
source("utils.R")
load_ionosphere()

# Load neural network library
library(neuralnet)
library(nnet)

# calculate the length of the Wts vector
num_wts = function(p, M, K) {
    # returns the length of the Wts vector
    #
    # p = ncol(X) # number of predictor variables
    # M = size    # number of hidden units
    # K = 1       # number of classes
    return ((p + 1) * M + K * (M + 1))
}

# Create training and test sets from MNIST data
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

# number of predictor variables
p <- ncol(nxtrain) 

# x, y input the number of output classes
K <- 10 

best_M <- 100
best_maxit <- 250
best_decay <- 0.2

tstart <- proc.time()

nnmodel <- nnet(x=nxtrain, y = class.ind(rtrain$y),
                softmax=TRUE,
                size=best_M, decay=best_decay, maxit=best_maxit,
                # bag=TRUE, 
                MaxNWts=num_wts(p, best_M, K),
                # Wts=runif(num_wts(p, best_M, K), -0.5, 0.5),
                trace=TRUE)

tend <- proc.time()

elapsed <- tend[3] - tstart[3]
ytrain.predicted <- as.factor(predict(nnmodel, newdata=nxtrain, type="class"))
train.error <- 100 * sum(ytrain.predicted != rtrain$y)/ntrain 

ypred <- as.factor(predict(nnmodel, newdata=nxtest, type="class"))
test.error <- 100 * sum(ypred != rtest$y)/ntest

cat(sprintf("training error %f, test error = %f, run time = %f\n", 
            train.error, test.error, elapsed))

# nnmodel <- neuralnet(formula = nnFormula, 
#                      data <- dat,
#                      hidden = nHidden,
#                      algorithm = "rprop+",
#                      learningrate.limit=list(min=c(1e-10), max=c(0.01)),
#                      learningrate.factor=list(minus=c(0.5), plus=c(1.2)),
#                      err.fct="sse",
#                      act.fct="tanh",
#                      threshold=0.1,
#                      lifesign="full", 
#                      lifesign.step=500,
#                      stepmax=1e4)

# nnmodel <- neuralnet(formula=nnFormula, 
#                      data=dat,
#                      hidden=nHidden,
#                      algorithm="rprop+",
#                      learningrate=5.0,
#                      rep=1)

