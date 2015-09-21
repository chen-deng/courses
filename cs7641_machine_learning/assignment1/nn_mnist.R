# Clear workspace
# rm(list = ls())

# Set working directory
setwd("/Users/rkekatpure/work/code/coursework/fall2015_machine_learning/assignment1")

# Source common utilities
source("utils.R")
# load_mnist()

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
ntest <- 10000
ntrain <- 60000
strain <- get_sample(ntrain, rtrain)
xtrain <- strain$x
ytrain <- as.factor(strain$y)

stest <- get_sample(ntest, rtest)
xtest <- stest$x
ytest <- as.factor(stest$y)

cat(sprintf("Training set has %d rows and %d columns\n", nrow(xtrain), ncol(xtrain)))
cat(sprintf("Test set has %d rows and %d columns\n", nrow(xtest), ncol(xtest)))

# Normalize training and testing data
normFactor <- max(xtrain)
nxtrain <- xtrain/normFactor
meanFactor <- mean(nxtrain)
nxtrain <- nxtrain - meanFactor
nxtest <- xtest/normFactor - meanFactor

# number of predictor variables
p <- ncol(nxtrain) 

# x, y input the number of output classes
K <- 10 

best_M <- 50
best_maxit <- 250
best_decay <- 0.5

tstart <- proc.time()

nnmodel <- nnet(x=nxtrain, y = class.ind(ytrain),
                softmax=TRUE,
                size=best_M, decay=best_decay, maxit=best_maxit,
                # bag=TRUE, 
                MaxNWts=num_wts(p, best_M, K),
                # Wts=runif(num_wts(p, best_M, K), -0.5, 0.5),
                trace=TRUE)

tend <- proc.time()

elapsed <- tend[3] - tstart[3]
ytrain.predicted <- as.numeric(predict(nnmodel, newdata=nxtrain, type="class"))
train.error <- 100 * sum(ytrain.predicted != ytrain)/ntrain 

ypred <- as.numeric(predict(nnmodel, newdata=nxtest, type="class"))
test.error <- 100 * sum(ypred != ytest)/ntest

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

