## Clear workspace
# rm(list = ls())

## Set working directory
setwd("/Users/rkekatpure/work/code/coursework/fall2015_machine_learning/assignment1")

## Get a sample of training data
source("utils.R")
load_mnist()


## Function to run model 
run_model <- function(xtrain, xtest, ytrain, ytest, klist){
    # initialize percentage error
    perr = numeric(length=length(klist))
    
    N = length(klist)    
    for (i in 1:N) {
        
        # Perform the classification
        knn_model <- knn(xtrain, xtest, ytrain, k=klist[i])
        
        # Convert factor results to numeric (http://stackoverflow.com/a/3418192/1165786)
        predicted <- as.numeric(levels(knn_model))[knn_model]
        
        # Get error rate
        perr[i] <- 100 * sum(predicted != ytest)/length(ytest)
        row = paste(klist[i], perr[i], sep=",")
        print(row)
    }
    perr
}

# KNN is contained in the 'class' library
library(class)

# Take random sample out of training and test data
N_test <- 200
N_train <- 6 * N_test
smp_train <- get_sample(N_train, rtrain)
smp_test <- get_sample(N_test, rtest)

# Create list of neighbors
maxk <- 20
klist <- c(1 : maxk)

## Train the model
training_error <- run_model(smp_train$x, smp_train$x, 
                            smp_train$y, smp_train$y, klist)

test_error <- run_model(smp_train$x, smp_test$x, 
                        smp_train$y, smp_test$y, klist)

write.csv(data.frame(neighbors=klist, training_error=training_error, test_error=test_error),
          file="results/mnist/knn/error_vs_neighbors.csv",
          row.names=F)

