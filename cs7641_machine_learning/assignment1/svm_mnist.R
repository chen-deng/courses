# Clear workspace
# rm(list = ls())

# Load libraries
library("e1071")
library("caret")

run_model <- function(xtrain, ytrain, xtest, ytest, 
                      kernel="radial", degree=1, gamma=gamma) {
    # Build a data frame from training data
    dat <- data.frame(x=xtrain, y=ytrain)
    svmfit <- svm(y~., data=dat, kernel=kernel, cost=10, 
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
# load_mnist()


# Take random sample out of training and test data
ntest <- 10000
ntrain <- 6 * ntest
strain <- get_sample(ntrain, rtrain)
xtrain <- strain$x
ytrain <- as.factor(strain$y)

stest <- get_sample(ntest, rtest)
xtest <- stest$x
ytest <- as.factor(stest$y)

cat(sprintf("Training set has %d rows and %d columns\n", nrow(xtrain), ncol(xtrain)))
cat(sprintf("Test set has %d rows and %d columns\n", nrow(xtest), ncol(xtest)))

# Remove zero variance columns
threshold <- 0.2
alldata <- rbind(strain$x, stest$x)
nzv <- nearZeroVar(alldata, saveMetrics=TRUE)
dim(nzv[nzv$percentUnique > threshold,])
above_threshold <- nzv[nzv$percentUnique > threshold, ]
nzvcols <- as.numeric(rownames(above_threshold))

xtrain_nzv <- xtrain[, nzvcols]
xtest_nzv <- xtest[, nzvcols]

# Tuning the parameters using the 'tune' function
# dat <- data.frame(x=xtrain_nzv, label=ytrain)
# tune.out <- tune(svm, label ~ ., data=dat, kernel="radial", 
#                  ranges=list(cost <- c(0.01, 0.1, 1, 10), 
#                              gamma <- c(0.001, 0.01, 0.1, 1)))

for (N in seq(60000, 60000, by=500)) {
    errs <- run_model(xtrain_nzv[1 : N, ], ytrain[1 : N], 
                      xtest_nzv, ytest, 
                      kernel="polynomial", 
                      degree=3,
                      gamma=1e-3)
    cat(sprintf("training error = %0.2f, test error = %0.2f\n", errs[1], errs[2]))    
}
