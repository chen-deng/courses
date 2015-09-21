## Clear workspace
# rm(list = ls())

## Set working directory
setwd("/Users/rkekatpure/work/code/coursework/fall2015_machine_learning/assignment1")

## Source common utilities
source("utils.R")
# load_mnist()

## Load neural network library
library(rpart)

## Create training and test sets from MNIST data
ntest <- 5000
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

## Create data frames with normalized data
dtrain <- data.frame(y=ytrain, X=nxtrain)
dtest <- data.frame(y=ytest, X=nxtest)

## Train the model
fm <- as.formula(paste0("y ~", paste0("X.", 1 : ncol(nxtrain), collapse = "+")))

## Run the model in the loop to determine the maximum depth
tree_depths <- c(1:29)
etrain <- numeric(length(tree_depths))
etest = numeric(length(tree_depths))

for (i in c(1 : length(tree_depths))) {
    fit <- rpart(formula=fm, data=dtrain, method="class", 
                 cp=0, minsplit=0, minbucket=0, maxdepth=tree_depths[i])
    
    ypred <- predict(fit, dtest[, -1], type="class")
    
    ## Model accuracy on training data
    resTrain <- predict(fit, dtrain[, -1], type="class")
    etrain[i] <- 100 * sum(ytrain != resTrain)/ntrain
    
    
    ## Model accuracy of test set
    resTest <- predict(fit, dtest[, -1], type="class")
    etest[i] <- 100 * sum(ytest != resTest)/ntest
    
    cat(sprintf("depth = %d, Training error = %f, test error = %f\n", 
                tree_depths[i], etrain[i], etest[i]))
}

write.table(data.frame(depth=tree_depths, training_error=etrain, test_error=etest),
          file="results/dtree/train_test_error_vs_depth.csv",
          sep=",",
          row.names=F)

## Plotting and saving
# pdf("dtree_train_test_err.pdf", width=10, height=6)
plot(tree_depths, etrain, type='l', col='blue', 
     xlab="", ylab="",
     xlim=c(0, 32), ylim=c(0,100),
     lwd=2.5)

lines(tree_depths, etest, type='l', col='red',
      xlab="", ylab="",
      lwd=2.5)

title(xlab="Decision tree depth", ylab="% error", cex.lab=1.5)
legend(20, 100, 
       c("training error", "test error"),
       lwd=c(2.5,2.5),
       col=c("blue","red"))
