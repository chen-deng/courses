## Clear workspace
# rm(list = ls())

## Set working directory
setwd("/Users/rkekatpure/work/code/coursework/fall2015_machine_learning/assignment1")
par(mar=c(1,1,1,1))

## Source common utilities
source("utils.R")
# load_mnist()

## Load neural network library
library(rpart)
library(adabag)

## Create training and test sets from MNIST data
ntest <- 1000
ntrain <- 6 * ntest
strain <- get_sample(ntrain, rtrain)
xtrain <- strain$x
ytrain <- as.factor(strain$y)

stest <- get_sample(ntest, rtest)
xtest <- stest$x
ytest <- as.factor(stest$y)

cat(sprintf("Training set has %d rows and %d columns\n", nrow(xtrain), ncol(xtrain)))
cat(sprintf("Test set has %d rows and %d columns\n", nrow(xtest), ncol(xtest)))

## Normalize training and testing data
normFactor <- max(xtrain)
nxtrain <- xtrain/normFactor
meanFactor <- mean(nxtrain)
nxtrain <- nxtrain - meanFactor
nxtest <- xtest/normFactor - meanFactor

## Create data frames for training ad testing from normalized matrices
dtrain <- data.frame(y=ytrain, X=nxtrain)
dtest <- data.frame(y=ytest, X=nxtest)

## Create the formula
fm <- as.formula(paste0("y ~", paste0("X.", 1 : ncol(nxtrain), collapse = "+")))

# Set the pruning parameters
rpart_complexity_param <- 5e-4
rpart_maxdepth <- 30

## First fit an unboosted decision tree using rpart 
fit <- rpart(formula=fm, data=dtrain, method="class", 
             minsplit=0, minbucket=0,
             cp=rpart_complexity_param, 
             maxdepth=rpart_maxdepth)

## Print its training and test error, and number of nodes
rpart_res_train <- predict(fit, dtrain[, -1], type="class")
rpart_etrain <- 100 * sum(ytrain != rpart_res_train)/ntrain

# Model accuracy of test set
rpart_res_test <- predict(fit, dtest[, -1], type="class")
rpart_etest <- 100 * sum(ytest != rpart_res_test)/ntest

# Get number of nodes
rpart_num_nodes <- nrow(fit$frame)
cat(sprintf("======================RPART RESULTS======================\n"))
cat(sprintf("nodes = %d, Training error = %f, test error = %f\n", 
            rpart_num_nodes, rpart_etrain, rpart_etest))

## Ada boost
ada_complexity_param <- 0.01
ada_maxdepth <- 4

ada_fit <- boosting(fm, data=dtrain, boos=T, mfinal=200, 
                   coeflearn="Breiman", 
                   control=rpart.control(maxdepth=ada_maxdepth))

res_train <- predict.boosting(ada_fit, newdata=dtrain)
res_test <- predict.boosting(ada_fit, newdata=dtest)

errorevol(ada_fit, newdata=dtest) -> evol.test
errorevol(ada_fit, newdata=dtrain) -> evol.train

plot.errorevol(evol.train, evol.test,
               lty=c(1,1))

cat(sprintf("======================ADABOOST RESULTS======================\n"))
cat(sprintf("Training error = %f, test error = %f\n", res_train$error, res_test$error))
