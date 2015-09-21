## Clear workspace
# rm(list = ls())

## Set working directory
setwd("/Users/rkekatpure/work/code/coursework/fall2015_machine_learning/assignment1")
par(mar=c(1,1,1,1))

## Source common utilities
source("utils.R")
load_ionosphere()

## Load neural network library
library(rpart)
library(adabag)

## Normalize training and testing data
ntrain <- nrow(rtrain$x)
ntest <- nrow(rtest$x)

normFactor <- max(rtrain$x)
nxtrain <- rtrain$x/normFactor
meanFactor <- mean(nxtrain)
nxtrain <- nxtrain - meanFactor
nxtest <- rtest$x/normFactor - meanFactor

## Create data frames with normalized data
dtrain <- data.frame(y=as.factor(rtrain$y), X=rtrain$x)
dtest <- data.frame(y=as.factor(rtest$y), X=rtest$x)

## Create a formula
fm = as.formula(paste("y ~ ", paste0("X.", colnames(rtrain$x), collapse="+")))

## First fit an unboosted decision tree using rpart 
rfit <- rpart(formula=fm, data=dtrain, method="class", 
             minsplit=0, minbucket=0,
             maxdepth=30)

## Model accuracy on training data
res_train <- predict(rfit, dtrain[, -1], type="class")
etrain <- 100 * sum(rtrain$y != res_train)/ntrain

## Model accuracy of test set
res_test <- predict(rfit, dtest[, -1], type="class")
etest <- 100 * sum(rtest$y != res_test)/ntest

# Get number of nodes
rpart_num_nodes <- nrow(rfit$frame)
cat(sprintf("======================RPART RESULTS======================\n"))
cat(sprintf("nodes = %d, Training error = %f, test error = %f\n", 
            rpart_num_nodes, etrain, etest))

## Ada boost
ada_complexity_param <- 0.01
ada_maxdepth <- 3

ada_fit <- boosting(fm, data=dtrain, boos=T, mfinal=1000, 
                   coeflearn="Breiman", 
                   control=rpart.control(maxdepth=ada_maxdepth))

res_train <- predict.boosting(ada_fit, newdata=dtrain)
res_test <- predict.boosting(ada_fit, newdata=dtest)

errorevol(ada_fit, newdata=dtest) -> evol.test
errorevol(ada_fit, newdata=dtrain) -> evol.train

plot.errorevol(evol.test, evol.train, lty=c(1,1))

cat(sprintf("======================ADABOOST RESULTS======================\n"))
cat(sprintf("Training error = %f, test error = %f\n", res_train$error, res_test$error))

errdf <- data.frame(itrn=c(1 : length(evol.test$error)),
                    etrain=evol.train$error,
                    etest=evol.test$error)
errdf
# write.csv(errdf, "results/ionosphere/boosting/error_vs_boosting_iterations.csv", 
#           row.names=F)
