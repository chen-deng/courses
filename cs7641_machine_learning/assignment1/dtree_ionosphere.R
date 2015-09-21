## Clear workspace
rm(list = ls())

## Set working directory
setwd("/Users/rkekatpure/work/code/coursework/fall2015_machine_learning/assignment1")
par(mar=c(1,1,1,1))

## Source common utilities
source("utils.R")
load_ionosphere()

## Load the decision-tree library
library(rpart)
library(caret)

cat(sprintf("Training set has %d rows and %d columns\n", nrow(rtrain), ncol(rtrain)))
cat(sprintf("Test set has %d rows and %d columns\n", nrow(rtest), ncol(rtest)))

## Normalize training and testing data
ntrain <- nrow(rtrain$x)
ntest <- nrow(rtest$x)

normFactor <- max(rtrain$x)
nxtrain <- rtrain$x/normFactor
meanFactor <- mean(nxtrain)
nxtrain <- nxtrain - meanFactor
nxtest <- rtest$x/normFactor - meanFactor

## Create data frames with normalized data
dtrain <- data.frame(y=rtrain$y, X=rtrain$x)
dtest <- data.frame(y=rtest$y, X=rtest$x)

## Create a formula
fm = as.formula(paste("y ~ ", paste0("X.", colnames(rtrain$x), collapse="+")))

## Run the model in the loop to determine the maximum depth
tree_depths <- c(1:29)
etrain <- numeric(length(tree_depths))
etest = numeric(length(tree_depths))

for (i in c(1 : length(tree_depths))) {
    fit <- rpart(formula=fm, data=dtrain, method="class", 
                 cp=0, minsplit=0, minbucket=0, maxdepth=tree_depths[i])
    
    ypred <- predict(fit, dtest[, -1], type="class")
    
    ## Model accuracy on training data
    res_train <- predict(fit, dtrain[, -1], type="class")
    etrain[i] <- 100 * sum(rtrain$y != res_train)/ntrain
    
    ## Model accuracy of test set
    res_test <- predict(fit, dtest[, -1], type="class")
    etest[i] <- 100 * sum(rtest$y != res_test)/ntest
    
    cat(sprintf("depth = %d, Training error = %f, test error = %f\n", 
                tree_depths[i], etrain[i], etest[i]))
}

# write.table(data.frame(depth=tree_depths, training_error=etrain, test_error=etest),
#           file="results/creditset/dtree/train_test_error_vs_depth.csv",
#           sep=",",
#           row.names=F)

## Plotting and saving
pdf("results/ionosphere/dtree/dtree_varimp.pdf", width=10, height=6)
vi <- varImp(fit)
idx <- order(vi$Overall)
vi_vals <- vi[idx, 1]
vi_names <- row.names(vi)[idx]
barplot(vi_vals, names.arg=vi_names, horiz=T)
dev.off()
