# Clear workspace
rm(list = ls())

# Load libraries
library("e1071")
library("caret")

run_model <- function(xtrain, ytrain, xtest, ytest, 
                      kernel="radial", degree=1, 
                      gamma=gamma, cost=10) {
    # Build a data frame from training data
    dat <- data.frame(x=xtrain, y=ytrain)
    svmfit <- svm(y~., data=dat, kernel=kernel, cost=cost, 
                  degree=degree, gamma=gamma)    
    predicted <- predict(svmfit, xtest)
    training_error <- 100.0 * sum(predict(svmfit, xtrain) != ytrain)/length(ytrain)    
    test_error <- 100.0 * sum(predict(svmfit, xtest) != ytest)/length(ytest)    
    
    # @return
    c(training_error, test_error)
}

# Main

# Set working directory
setwd("/Users/rkekatpure/work/code/coursework/fall2015_machine_learning/assignment1")

# load_mnist.R contains logic for loading the mnist dataset and displaying an 
# individual digit by reshaping a given row of the training data X. To use the
# functions in the file, we have to 'source' the file. Its similar to the 
# 'import' in Python
source("utils.R")
load_ionosphere()


# Take random sample out of training and test data
ntest <- nrow(rtrain$x)
ntrain <- nrow(rtest$x)
cat(sprintf("Training set has %d rows and %d columns\n", 
            nrow(rtrain$x), ncol(rtrain$x)))
cat(sprintf("Test set has %d rows and %d columns\n", 
            nrow(rtest$x), ncol(rtest$x)))

# Remove zero variance columns
threshold <- 0.2
alldata <- rbind(rtrain$x, rtest$x)
nzv <- nearZeroVar(alldata, saveMetrics=TRUE)
dim(nzv[nzv$percentUnique > threshold,])
above_threshold <- nzv[nzv$percentUnique > threshold, ]
nzvcols <- as.numeric(rownames(above_threshold))

xtrain_nzv <- rtrain$x[, nzvcols]
xtest_nzv <- rtest$x[, nzvcols]

Xtrain <- as.matrix(rtrain$x[,-2])
colnames(Xtrain) <- NULL

Xtest <- as.matrix(rtest$x[, -2])
colnames(Xtest) <- NULL

## Variation with cost
costs <- 10^seq(-4, 5, by=0.01)
etrain <- numeric(length(costs))
etest <- numeric(length(costs))

for (i in c(1:length(costs))){
    cost <- costs[i]
    errs <- run_model(Xtrain, rtrain$y, 
                      Xtest, rtest$y, 
                      kernel="radial", 
                      degree=3,
                      gamma=1e-3,
                      cost=cost)
    etrain[i] <- errs[1]
    etest[i] <- errs[2]
    cat(sprintf("cost = %f, training error = %0.2f, test error = %0.2f\n", cost, errs[1], errs[2]))    
}

df = data.frame(cost=costs, etrain=etrain, etest=etest)
write.csv(df, file="results/ionosphere/svm/error_vs_cost.csv", row.names=F)
# plot(costs, etest, log='x')

## Variation with gamma
gammas <- 10^seq(-6, 2, by=0.01)
etrain <- numeric(length(gammas))
etest <- numeric(length(gammas))

for (j in c(1:length(gammas))){
    gamma <- gammas[j]
    errs <- run_model(Xtrain, rtrain$y, 
                      Xtest, rtest$y, 
                      kernel="radial", 
                      degree=3,
                      gamma=gamma,
                      cost=71)
    etrain[j] <- errs[1]
    etest[j] <- errs[2]
    cat(sprintf("gamma = %f, training error = %0.2f, test error = %0.2f\n", gamma, errs[1], errs[2]))    
}

df = data.frame(gamma=gammas, etrain=etrain, etest=etest)
write.csv(df, file="results/ionosphere/svm/error_vs_gamma.csv", row.names=F)
plot(gammas, etest, log='x')
