# Clear workspace
# rm(list = ls())

# Set working directory
setwd("/Users/rkekatpure/work/code/coursework/fall2015_machine_learning/assignment1")

# Source common utilities
source("utils.R")
# load_mnist()

# Load neural network library
library(rpart)

# Create training and test sets from MNIST data
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

# Normalize training and testing data
normFactor <- max(xtrain)
nxtrain <- xtrain/normFactor
meanFactor <- mean(nxtrain)
nxtrain <- nxtrain - meanFactor
nxtest <- xtest/normFactor - meanFactor

dtrain <- data.frame(y=ytrain, X=xtrain)
dtest <- data.frame(y=ytest, X=xtest)
# Train the model
fm <- as.formula(paste0("y ~", paste0("X.", 1 : ncol(nxtrain), collapse = "+")))

# Train the model with a low complexity parameter on all data
fit <- rpart(formula=fm, data=dtrain, method="class", 
             cp=0, minsplit=0, minbucket=0, maxdepth=15)

# Now prune the tree using various complexity_parameter values
complexity_param <- 10^seq(0, -5, by=-0.05)
num_nodes <- integer(length(complexity_param))
etrain <- numeric(length(complexity_param))
etest = numeric(length(complexity_param))


for (i in c(1 : length(complexity_param))) {

    cp <- complexity_param[i]
    
    fitp <- prune(fit, cp=cp)
    ypred <- predict(fitp, dtest[, -1], type="class")
    
    # Model accuracy on training data
    resTrain <- predict(fitp, dtrain[, -1], type="class")
    etrain[i] <- 100 * sum(ytrain != resTrain)/ntrain
    
    # Model accuracy of test set
    resTest <- predict(fitp, dtest[, -1], type="class")
    etest[i] <- 100 * sum(ytest != resTest)/ntest
    
    # Get number of nodes
    num_nodes[i] <- nrow(fitp$frame)
    
    cat(sprintf("cp = %f, nodes = %d, Training error = %f, test error = %f\n", 
                cp, num_nodes[i], etrain[i], etest[i]))
}

write.table(data.frame(cp=complexity_param, 
                       nodes=num_nodes, 
                       training_error=etrain, 
                       test_error=etest),
           file="results/dtree/train_test_error_vs_cp.csv",
           sep=",",
           row.names=F)

# Plotting and saving
# pdf("dtree_train_test_err.pdf", width=10, height=6)
matplot(num_nodes, cbind(etrain, etest), 
        type=c("l", "l"), 
        pch=c(22, 22),
        col=c('blue', "red"), 
        log="xy",
        xlab="", ylab="",
        #xlim=c(1e-6, 1), # ylim=c(0,100),
        lwd=2.5,
        lty=c(1, 1))

title(xlab="Number of nodes", ylab="% error", cex.lab=1.5)

legend(1, 2, 
       c("training error", "test error"),
       lwd=c(2.5,2.5),
       col=c("blue","red"))



